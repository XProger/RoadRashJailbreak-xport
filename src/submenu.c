/* F8006AE6C generic submenu input wrapper, actual MIPS ordering. */
#include "submenu.h"
#include <stdlib.h>
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t sh(uint32_t v){return v&0x8000?v|0xffff0000:v;}
static uint32_t sb(uint32_t v){return v&128?v|0xffffff00:v;}
uint32_t sub_F_8006AE6C(RRJMemory *m,uint32_t menu,RRJSubmenuCall cb)
{
    uint32_t item=rrj_read32(m,menu+16),selected,index,end,done=0;
    if(item){
        selected=item+120*sh(h(m,menu+4));
        if(rrj_s32(rrj_read32(m,0x800D717C))>0 && rrj_s32(rrj_read32(m,0x800D718C))>0 &&
           rrj_s32(rrj_read32(m,0x800D7194))>0 && rrj_s32(rrj_read32(m,0x800D713C))>0 &&
           rrj_s32(sb(b(m,0x800D717A)))>0 && h(m,selected+8)==12 && h(m,selected+18)==71){
            rrj_put16(rrj_at(m,0x8009C5D2,2),56);done=1;
        }
        index=sb(b(m,menu+14));end=index+sb(b(m,menu+15));
        while(rrj_s32(index)<rrj_s32(end) && !done){
            if(rrj_s32(sb(b(m,0x800D7128+192*index+74)))>0 &&
               rrj_read32(m,0x8009C7B8+4*sh(h(m,0x8009C5D0)))!=0xffffffff){
                if(!cb)abort();(void)cb(m,0x8007EAC0,3,0,0);done=1;
                rrj_put16(rrj_at(m,0x8009C5D2,2),(uint16_t)sb(b(m,0x800D80E8)));
            }
            end=sb(b(m,menu+14))+sb(b(m,menu+15));++index;
        }
    }
    if(done)return done;
    index=sb(b(m,menu+14));end=index+sb(b(m,menu+15));
    if(!cb)abort();return cb(m,0x8006B03C,menu,index,end);
}
