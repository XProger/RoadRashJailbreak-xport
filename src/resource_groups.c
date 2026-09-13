/* Resource-group selection and mask, actual F-overlay MIPS. */
#include "resource_groups.h"
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
uint32_t sub_F_80064254(RRJMemory *m,uint32_t mode)
{
    uint32_t mask=mode<6?1u<<mode:1;
    *(uint8_t *)rrj_at(m,0x8009C5E4,1)=(uint8_t)mask;
    return mask; /* v1=800A0000 is a scratch register, not a 64-bit result. */
}
uint32_t sub_F_80068448(RRJMemory *m)
{
    uint32_t mode=rrj_read32(m,0x800D80D8),first,second,last,index;
    switch(mode) {
    case 32:
        first=rrj_read32(m,0x8009C4BC);index=b(m,0x800D81E1);
        rrj_write32(m,0x8009C668,first);
        if(index>1){rrj_write32(m,0x8009C66C,0);rrj_write32(m,0x8009C670,0);return 0x8009C5D0;}
        last=rrj_read32(m,0x8009C4C0+4*index);
        rrj_write32(m,0x8009C670,0);rrj_write32(m,0x8009C66C,last);return last;
    case 4:case 8:
        first=rrj_read32(m,mode==4?0x8009C4D0:0x8009C508);
        last=rrj_read32(m,mode==4?0x8009C4CC:0x8009C50C);
        rrj_write32(m,0x8009C670,0);rrj_write32(m,0x8009C668,first);rrj_write32(m,0x8009C66C,last);return last;
    case 16:case 24:
        first=rrj_read32(m,0x8009C4EC);second=rrj_read32(m,mode==16?0x8009C4F0:0x8009C510);
        last=rrj_read32(m,mode==16?0x8009C4F4:0x8009C514);
        rrj_write32(m,0x8009C668,first);rrj_write32(m,0x8009C66C,second);rrj_write32(m,0x8009C670,last);return last;
    case 17:
        first=rrj_read32(m,0x8009C4F8);index=b(m,0x800D81DF);second=rrj_read32(m,0x8009C4FC);
        rrj_write32(m,0x8009C668,first);rrj_write32(m,0x8009C66C,second);
        last=rrj_read32(m,(index<18 || index>=128)?0x8009C500:0x8009C504);
        rrj_write32(m,0x8009C670,last);return last;
    default:rrj_write32(m,0x8009C668,0);rrj_write32(m,0x8009C66C,0);rrj_write32(m,0x8009C670,0);return 0x8009C5D0;
    }
}
