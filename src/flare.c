/* Hand translation of seven IDA drafts, checked against RRJ MIPS.
 * See status/decompilation for contracts, table bytes and differential probes.
 * Helpers preserve store order, link address bits, and untouched packet bytes.
 */
#include "flare.h"
#include "packet.h"

static uint32_t packet(RRJMemory *memory, uint32_t size)
{
    uint32_t context = rrj_read32(memory, 0x8005B470);
    uint32_t address = rrj_read32(memory, context + 0x10C);
    if (address + size >= rrj_read32(memory, 0x8005B4D0)) {
        address = sub_80021C98(memory, address, size);
        context = rrj_read32(memory, 0x8005B470);
        rrj_write32(memory, context + 0x10C, address);
    }
    context = rrj_read32(memory, 0x8005B470);
    address = rrj_read32(memory, context + 0x10C);
    rrj_write32(memory, context + 0x10C, address + size);
    return address;
}

uint32_t sub_8002AF9C(RRJMemory *memory, const void *quad, const void *color, void *link)
{
    const uint8_t *pt = (const uint8_t *)quad;
    uint32_t address = packet(memory, 24), value;
    uint8_t *prim = (uint8_t *)rrj_at(memory, address, 24);
    rrj_put32(prim, rrj_u32(link)); rrj_put32(link, address);
    value = rrj_u32(color);
    prim[3] = 5; rrj_put32(prim + 4, value); prim[7] = 0x2A;
    rrj_put32(prim + 8, rrj_u32(pt));
    rrj_put32(prim + 12, rrj_u32(pt + 4));
    rrj_put32(prim + 16, rrj_u32(pt + 8));
    value = rrj_u32(pt + 12); rrj_put32(prim + 20, value);
    return value;
}

uint32_t sub_8002B164(RRJMemory *memory, const void *quad, const void *color, void *link)
{
    const uint8_t *pt = (const uint8_t *)quad;
    uint32_t address = packet(memory, 36), value;
    uint8_t *prim = (uint8_t *)rrj_at(memory, address, 36);
    rrj_put32(prim, rrj_u32(link)); rrj_put32(link, address);
    value = rrj_u32(color);
    rrj_put32(prim + 12, 0); rrj_put32(prim + 4, value);
    value = rrj_u32(color); /* second read: retain even if color aliases packet */
    prim[3] = 8; rrj_put32(prim + 28, 0); prim[7] = 0x3A;
    rrj_put32(prim + 20, value);
    rrj_put32(prim + 8, rrj_u32(pt));
    rrj_put32(prim + 16, rrj_u32(pt + 4));
    rrj_put32(prim + 24, rrj_u32(pt + 8));
    value = rrj_u32(pt + 12); rrj_put32(prim + 32, value);
    return value;
}

uint32_t sub_8002B258(RRJMemory *memory, const void *quad, const void *color, uint32_t type, void *link)
{
    const uint8_t *pt = (const uint8_t *)quad;
    const uint8_t *texture = (const uint8_t *)rrj_at(memory, 0x800D4270 + type * 28, 24);
    uint32_t address = packet(memory, 40), value;
    uint8_t *prim = (uint8_t *)rrj_at(memory, address, 40);
    rrj_put32(prim, rrj_u32(link)); rrj_put32(link, address);
    value = rrj_u32(color);
    prim[3] = 9; rrj_put32(prim + 4, value); prim[7] = 0x2E;
    rrj_put32(prim + 8, rrj_u32(pt)); rrj_put32(prim + 16, rrj_u32(pt + 4));
    rrj_put32(prim + 24, rrj_u32(pt + 8)); rrj_put32(prim + 32, rrj_u32(pt + 12));
    prim[12] = texture[0]; prim[13] = (uint8_t)(texture[12] - 1 + texture[4]);
    prim[20] = texture[0]; prim[21] = texture[4];
    prim[28] = (uint8_t)(texture[8] - 1 + texture[0]);
    prim[29] = (uint8_t)(texture[12] - 1 + texture[4]);
    prim[36] = (uint8_t)(texture[8] - 1 + texture[0]); prim[37] = texture[4];
    rrj_put16(prim + 22, (rrj_u16(texture + 16) & 0xFF9F) | 0x20);
    value = rrj_u16(texture + 20); rrj_put16(prim + 14, value);
    return value;
}

static void xy_offset(uint8_t *point, const void *xy, uint32_t x, uint32_t y)
{
    rrj_put16(point, rrj_u16(xy) + x);
    rrj_put16(point + 2, rrj_u16((const uint8_t *)xy + 2) + y);
}

uint32_t sub_8002B3FC(RRJMemory *memory, const void *xy, const void *color, uint32_t width, uint32_t height, uint8_t type, void *link)
{
    uint8_t pt[16];
    xy_offset(pt, xy, 0u - width, height); xy_offset(pt + 4, xy, width, height);
    xy_offset(pt + 8, xy, 0u - width, 0u - height); xy_offset(pt + 12, xy, width, 0u - height);
    return sub_8002B258(memory, pt, color, type, link);
}

uint32_t sub_8002B4A0(RRJMemory *memory, const void *xy, const void *color, uint32_t width, uint32_t height, void *link)
{
    uint8_t pt[24];
    uint32_t quarter = (uint32_t)(rrj_s32(width) / 4), half = (uint32_t)(rrj_s32(width) / 2);
    uint32_t h = (uint32_t)(rrj_s32(height) / 2);
    xy_offset(pt, xy, 0u - quarter, h); xy_offset(pt + 4, xy, quarter, h);
    xy_offset(pt + 8, xy, 0u - half, 0); xy_offset(pt + 12, xy, half, 0);
    xy_offset(pt + 16, xy, 0u - quarter, 0u - h); xy_offset(pt + 20, xy, quarter, 0u - h);
    sub_8002AF9C(memory, pt, color, link);
    return sub_8002AF9C(memory, pt + 8, color, link);
}

static uint32_t scaled(uint32_t size, const void *table_halfword)
{
    const uint16_t bits = rrj_u16(table_halfword);
    const int32_t factor = bits < 0x8000 ? bits : (int32_t)bits - 65536;
    /* MULT/MFLO truncates BEFORE signed division, not after a 64-bit divide. */
    return (uint32_t)(rrj_s32(size * (uint32_t)factor) / 256);
}

uint32_t sub_8002B5B4(RRJMemory *memory, const void *xy, const void *color, uint32_t width, uint32_t height, void *link)
{
    uint8_t pt[32]; unsigned i;
    const uint8_t *table = (const uint8_t *)rrj_at(memory, 0x80053A28, 64);
    for (i = 0; i < 8; ++i)
        xy_offset(pt + i * 4, xy, scaled(width, table + i * 8), scaled(height, table + i * 8 + 2));
    sub_8002AF9C(memory, pt, color, link); sub_8002AF9C(memory, pt + 8, color, link);
    return sub_8002AF9C(memory, pt + 16, color, link);
}

uint32_t sub_8002B68C(RRJMemory *memory, const void *xy, const void *color, uint32_t width, uint32_t height, void *link)
{
    uint8_t pt[72]; unsigned i; uint32_t result = 0;
    const uint8_t *table = (const uint8_t *)rrj_at(memory, 0x800539E8, 64);
    for (i = 0; i < 9; ++i) {
        const uint8_t *record = table + (i % 8) * 8;
        xy_offset(pt + i * 8, xy, scaled(width, record), scaled(height, record + 2));
        xy_offset(pt + i * 8 + 4, xy, scaled(width - 5, record), scaled(height - 5, record + 2));
    }
    for (i = 0; i < 8; ++i) result = sub_8002B164(memory, pt + i * 8, color, link);
    return result;
}
