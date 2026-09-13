/* F80080488: deferred resource release, preserving callback-visible reloads. */
#include "menu_cleanup.h"
#include <stdlib.h>
uint32_t sub_F_80080488(RRJMemory *m,RRJCleanupCall call)
{
    uint32_t index=0,entry=0x800A0970,descriptor,allocation,count,flags;
    if(!call) abort();
    (void)call(m,0x800487C0,0);
    (void)call(m,0x80043DA4,0);
    count=rrj_read32(m,0x8009D4DC);
    if(rrj_s32(count)>0) do {
        descriptor=rrj_read32(m,entry);allocation=rrj_read32(m,descriptor+4);
        if(allocation){
            (void)call(m,0x800144B8,allocation);
            descriptor=rrj_read32(m,entry);rrj_write32(m,descriptor+4,0);
            descriptor=rrj_read32(m,entry);
        }
        entry+=4;++index;
        count=rrj_read32(m,0x8009D4DC);
        flags=rrj_u16(rrj_at(m,descriptor,2));
        rrj_put16(rrj_at(m,descriptor,2),(uint16_t)(flags&0xfffb));
    } while(rrj_s32(index)<rrj_s32(count));
    rrj_write32(m,0x8009D4DC,0);
    return call(m,0x80043DB4,0);
}
