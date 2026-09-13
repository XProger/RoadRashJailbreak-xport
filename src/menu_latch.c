#include "wip.h"
/* Menu input snapshot and event consumption, audited MIPS branch. */
#include "menu_latch.h"
#include <stdio.h>
#include <stdlib.h>
uint32_t sub_8001E0B4(RRJMemory *m,uint32_t dst,uint32_t src,uint32_t bytes)
{
    uint32_t result=dst;
    while(bytes){uint32_t value=rrj_read32(m,src);src+=4;bytes-=4;rrj_write32(m,dst,value);dst+=4;}
    return result;
}
void sub_8001CB3C_menu(RRJMemory *m,RRJSDKCall critical)
{
    uint32_t count,player,button,record;
    if(!critical) abort();
    critical(m,0x80043DA4,0,0);
    (void)sub_8001E0B4(m,0x800D7128,0x800D6DE0,768);
    count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+52);
    for(player=0;rrj_s32(player)<rrj_s32(count);++player){
        record=0x800D6DF4+192*player;
        for(button=0;button<19;++button,record+=8){
            uint32_t held=rrj_read32(m,record);
            *(uint8_t *)rrj_at(m,record+6,1)=0;
            if(rrj_s32(held)<0) held=0;
            rrj_write32(m,record,held);
        }
    }
    if(*(uint8_t *)rrj_at(m,rrj_read32(m,0x8005B2F8),1)!=2){
        RRJ_WIP(m,0x8001CB3C,"function","input_latch","skip_non_menu_tail_continue_critical_exit",NULL,0);
    }
    critical(m,0x80043DB4,0,0);
}
