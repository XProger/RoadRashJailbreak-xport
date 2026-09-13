/* Menu hierarchy transition. Original flags, parent order and reloads retained. */
#include "menu_transition.h"
#include <stdlib.h>
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sh(uint32_t v){return v<32768?v:v|0xffff0000;}
static void half(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
static void byte(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static void call(RRJMemory *m,RRJSDKCall effect,uint32_t fn,uint32_t arg)
{
    if(!effect) abort();
    effect(m,fn,arg,0);
}
uint32_t sub_F_80080A70(RRJMemory *m,uint32_t menu)
{
    uint32_t list=rrj_read32(m,menu+16),count,index;
    if(!list) return 0;
    count=h(m,menu+8);if(!count || count>=32768) return 0;
    for(index=0;index<count;++index,list+=120) if(h(m,list+10)&0x1000) return index;
    return 0;
}
uint32_t sub_F_80066EF8(RRJMemory *m,RRJSDKCall effect)
{
    uint32_t current=sh(h(m,0x8009C5D0)),next=sh(h(m,0x8009C5D2)),shared=0,table,menu,ancestor,value;
    if(current==next) return next;
    rrj_write32(m,0x8005ACAC,0);rrj_write32(m,0x80088C50,0);
    if(current!=0xffffffff){
        table=rrj_read32(m,0x8009C68C);
        do {
            next=sh(h(m,0x8009C5D2));menu=rrj_read32(m,table+4*current);
            while(next!=0xffffffff){
                ancestor=rrj_read32(m,table+4*next);
                if(current==next){shared=1;break;}
                next=sh(h(m,ancestor+10));
            }
            if(!shared){
                value=h(m,menu+2);half(m,menu,h(m,menu)&0xfffc);
                if(!value) half(m,menu+2,0x7ffe);
            }
            current=sh(h(m,menu+10));
        } while(current!=0xffffffff);
    }
    next=sh(h(m,0x8009C5D2));menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*next);
    if(!(h(m,menu)&3) && !h(m,menu+2)) half(m,menu+2,0x7fff);
    current=sh(h(m,0x8009C5D0));
    if(current==4){byte(m,0x800D80E8,current);current=sh(h(m,0x8009C5D0));}
    if(rrj_s32(current)>=5 && (rrj_s32(current)<8 || current==29))
        call(m,effect,0x8006883C,sh(h(m,0x8009C5D2)));
    if(h(m,0x8009C5D0)==4){
        menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*sh(h(m,0x8009C5D2)));
        value=sub_F_80080A70(m,menu);
        menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*sh(h(m,0x8009C5D2)));
        half(m,menu+4,value);
    }
    next=h(m,0x8009C5D2);table=rrj_read32(m,0x8009C68C);half(m,0x8009C5D0,next);
    menu=rrj_read32(m,table+4*sh(next));value=h(m,menu)|3;
    rrj_write32(m,0x8009C5C8,menu);half(m,menu,value);return value;
}
uint32_t sub_F_8006711C(RRJMemory *m,RRJSDKCall effect)
{
    uint32_t current;
    rrj_write32(m,0x8009C658,rrj_read32(m,0x8009C654));current=h(m,0x8009C5D0);
    if(current==5 || current==29){
        byte(m,0x800D80F0,0);byte(m,0x800D80EA,1);byte(m,0x800D80DC,0);
        call(m,effect,0x8002D250,0);
    }
    return sub_F_80066EF8(m,effect);
}
