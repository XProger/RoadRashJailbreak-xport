#include "gpu.h"
#include "raster.h"
#include <stdio.h>
#include <string.h>

static int valid_upload(const PSX_RECT *r)
{
    return r->x>=0 && r->y>=0 && r->w>0 && r->h>0 &&
           (int)r->x+r->w<=1024 && (int)r->y+r->h<=512;
}
static sint32 host_load_image(void *user, PSX_RECT *r, uint32 *pixels)
{
    int y;
    (void)user;
    if (!valid_upload(r)) {
        fprintf(stderr,"WIP LoadImage: unsupported rectangle %d,%d %dx%d\n",r->x,r->y,r->w,r->h);
        return -1;
    }
    for (y=0;y<r->h;++y)
        memcpy(rrj_raster_vram()+(r->y+y)*1024+r->x,
               (const uint8_t *)pixels+(size_t)y*r->w*2,(size_t)r->w*2);
    return 0;
}
void rrj_gpu_bind_upload(PSX_CONFIG *config)
{
    config->host.load_image=host_load_image;
}
uint32_t rrj_gpu_upload(RRJMemory *memory, const uint8_t rect[8], uint32_t pixels)
{
    PSX_RECT r;
    uint32 *data;
    /* Native Windows is little-endian; memcpy also avoids alignment and
     * effective-type assumptions about the game's packed RECT bytes. */
    memcpy(&r,rect,sizeof r);
    if (!valid_upload(&r)) return (uint32_t)host_load_image(NULL,&r,NULL);
    data=(uint32 *)rrj_at(memory,pixels,(size_t)r.w*r.h*2);
    return (uint32_t)LoadImage(&r,data);
}

void rrj_gpu_clear_ot(RRJMemory *memory, uint32_t address, uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; ++i)
        rrj_write32(memory, address + i * 4, i ? (address + (i - 1) * 4) & 0xFFFFFF : 0xFFFFFF);
    /* RRJ PsyQ ClearOTagR adds its resident GPU barrier after DMA clear.
     * 80048D38/80048D40: retain this RAM-visible tail, not just terminator. */
    rrj_write32(memory,0x8005602C,0x04056018);
    rrj_write32(memory,address,0x0005602C);
}

/* Decode the DMA chain separately from the reused AA raster core. Padding
 * the temporary packet also makes AA's optional second command read safe.
 * Unsupported commands fail visibly while GPU integration is WIP. */
int rrj_gpu_draw_ot(RRJMemory *memory, uint32_t address, int origin_x, int origin_y)
{
    unsigned packets = 0;
    while ((address & 0xFFFFFF) != 0xFFFFFF) {
        uint32_t tag, count, offset = 0;
        const uint8_t *data;
        address &= 0xFFFFFF;
        if ((address & 3) || address > 0x1FFFFC || ++packets > 0x80000) return 0;
        tag = rrj_read32(memory, address); count = tag >> 24;
        if (count > (0x200000 - address - 4) / 4) return 0;
        data = rrj_at(memory, address + 4, count * 4);
        while (offset < count) {
            uint32_t word = rrj_u32(data + offset * 4);
            unsigned command = word >> 24, words = 0;
            uint32_t packet[16] = {0};
            if (command >= 0xE1 && command <= 0xE5) {
                rrj_raster_environment(word, origin_x, origin_y); ++offset; continue;
            }
            if (command == 0 || command == 1 || (command == 0xE6 && !(word & 3))) {
                ++offset; continue;
            }
            if (command == 0x80 && count - offset >= 4) {
                uint32_t source = rrj_u32(data + (offset + 1) * 4);
                uint32_t target = rrj_u32(data + (offset + 2) * 4);
                /* Menu's GPU barrier copies (0,0) to itself. Other VRAM
                 * moves need the host transfer adapter, still WIP. */
                if (source == target) { offset += 4; continue; }
            }
            switch (command & 0xFC) {
            case 0x20: words = 4; break;
            case 0x24: words = 7; break;
            case 0x28: words = 5; break;
            case 0x2C: words = 9; break;
            case 0x30: words = 6; break;
            case 0x34: words = 9; break;
            case 0x38: words = 8; break;
            case 0x3C: words = 12; break;
            case 0x60: words = 3; break;
            case 0x64: words = 4; break;
            case 0x68: case 0x70: case 0x78: words = 2; break;
            case 0x74: case 0x7C: words = 3; break;
            }
            if (!words || words > count - offset) {
                fprintf(stderr, "WIP GPU packet %02X at %08X word %u/%u\n", command, address, offset, count);
                return 0;
            }
            packet[0] = words << 24;
            memcpy(packet + 1, data + offset * 4, words * 4);
            rrj_raster_packet(packet); offset += words;
        }
        address = tag & 0xFFFFFF;
    }
    return 1;
}
