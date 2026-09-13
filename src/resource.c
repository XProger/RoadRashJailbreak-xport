/* WIP: IDA draft checked against F 80065768/8007A400 MIPS.
 * These functions enqueue work; queue execution is a separate dependency. */
#include "resource.h"
#include "resource_lookup.h"
#include <stdlib.h>
static uint32_t half(RRJMemory *m, uint32_t a)
{ return rrj_u16(rrj_at(m, a, 2)); }
static int32_t signed_half(RRJMemory *m, uint32_t a)
{ uint32_t v=half(m,a); return v<32768 ? (int32_t)v : (int32_t)v-65536; }
static void store_half(RRJMemory *m, uint32_t a, uint32_t v)
{ rrj_put16(rrj_at(m,a,2),v); }

uint32_t sub_F_80065768(RRJMemory *m, uint32_t descriptor, RRJImageUpload upload)
{
    uint8_t rect[8];
    uint32_t pixels, i;
    for (i=0;i<4;++i) rrj_put16(rect+i*2,half(m,descriptor+16+i*2));
    pixels=rrj_read32(m,descriptor+4)+16;
    if (!upload) abort();
    return upload(m,rect,pixels);
}

/* The count comparison is signed, but address arithmetic wraps at 32 bits.
 * Preserve load/store order even when supplied records alias queue storage. */
static void enqueue(RRJMemory *m, uint32_t descriptor, uint32_t rect,
                    uint32_t count_address, uint32_t base, uint32_t kind)
{
    uint32_t count=rrj_read32(m,count_address),q,data,y;
    if (rrj_s32(count)>=6) return;
    q=base+count*36;
    data=rrj_read32(m,descriptor+4);
    rrj_write32(m,q+8,kind);
    rrj_write32(m,q+12,1);
    rrj_write32(m,q+16,0);
    rrj_write32(m,q,data);
    rrj_write32(m,q+28,(uint32_t)signed_half(m,rect));
    y=(uint32_t)signed_half(m,rect+2);
    rrj_write32(m,count_address,count+1);
    rrj_write32(m,q+32,y);
}

uint32_t sub_F_8007A400(RRJMemory *m, uint32_t descriptor, uint32_t id, RRJImageUpload upload)
{
    uint32_t flags=rrj_read32(m,descriptor),rect,x,y;
    if (!(flags&0x10000000)) return 0;
    if (flags&0x40000000) {
        if (flags&0x80000000) {
            if (flags&0x20000000) return 1;
            rect=sub_F_8007A6C0(id,0);
            enqueue(m,descriptor,rect,0x8009C2F0,0x8009C2F8,4);
            rrj_write32(m,descriptor,rrj_read32(m,descriptor)|0x20000002);
            y=half(m,rect+2); x=half(m,rect);
            store_half(m,descriptor+28,((y&0x100)>>4)|((x&0x3ff)>>6)|0x100|4*(y&0x200));
            store_half(m,descriptor+24,(uint32_t)(signed_half(m,rect)%64));
            store_half(m,descriptor+26,(uint32_t)(signed_half(m,rect+2)%256));
            store_half(m,descriptor+30,0);
            return 1;
        }
        rect=sub_F_8007A6C0(id,0);
        enqueue(m,descriptor,rect,0x8009C2F4,0x8009C3D0,3);
        return 0;
    }
    if (!(flags&0x20000000)) (void)sub_F_80065768(m,descriptor,upload);
    return 1;
}
