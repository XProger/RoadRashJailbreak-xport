/* First eligible menu entry. MIPS caches list, count and resource mode. */
#include "menu_first.h"
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
uint32_t sub_F_80080ACC(RRJMemory *m,uint32_t menu)
{
    uint32_t list=rrj_read32(m,menu+16),count,mode,index;
    if(!list) return 0;
    count=rrj_u16(rrj_at(m,menu+8,2));if(!count || count>=32768) return 0;
    mode=rrj_read32(m,0x800D80D8);
    for(index=0;index<count;++index,list+=120){
        uint32_t type=rrj_u16(rrj_at(m,list+8,2)),flags;
        if(type!=12 && type!=13 && type!=17) continue;
        flags=rrj_read32(m,list);
        if((flags&0x40000000) && (b(m,0x800D70E1)>>4)!=8) continue;
        if((flags&0x02000000) && rrj_read32(m,0x800D742C+24*sb(b(m,0x8009C5E6)))!=1) continue;
        if((flags&0x10000000) && !b(m,0x800D80F3)) continue;
        if((flags&0x08000000) && mode!=4) continue;
        if((flags&0x04000000) && mode==4) continue;
        return index;
    }
    return 0;
}
