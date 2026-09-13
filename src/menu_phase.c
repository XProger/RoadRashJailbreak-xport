/* F8006D5B0: transition-phase draw, preserving phase changes around renderer. */
#include "menu_phase.h"
uint32_t sub_F_8006D5B0(RRJMemory *m,uint32_t menu,RRJMenuDraw draw)
{
    if(rrj_u16(rrj_at(m,menu,2))&2){
        *(uint8_t *)rrj_at(m,0x8009C5E1,1)=*(uint8_t *)rrj_at(m,menu+12,1);
        (void)sub_F_8006D3E0(m,menu,draw);rrj_put16(rrj_at(m,menu+2,2),0);
    }else if(rrj_u16(rrj_at(m,menu+2,2))==32766){
        uint8_t value=*(uint8_t *)rrj_at(m,menu+13,1);
        rrj_put16(rrj_at(m,menu+2,2),1);*(uint8_t *)rrj_at(m,0x8009C5E1,1)=value;
        (void)sub_F_8006D3E0(m,menu,draw);
    }else rrj_put16(rrj_at(m,menu+2,2),0);
    return 0;
}
