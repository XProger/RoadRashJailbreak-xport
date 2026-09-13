/* Main update hierarchy, actual zero-game-argument F800667E4 entry. */
#include "menu_update.h"
#include "menu.h"
#include "resource_select.h"
#include "resource_refresh.h"
#include "resource_apply.h"
#include "menu_video.h"
#include <stdlib.h>
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
static uint32_t sh(uint32_t v){return v<32768?v:v|0xffff0000;}
static void put(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static void half(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
static void call(RRJMemory *m,RRJSDKCall effect,uint32_t fn,uint32_t arg){if(!effect) abort();effect(m,fn,arg,0);}
uint32_t sub_F_800667E4(RRJMemory *m,RRJMenuUpdateCall update,RRJSDKCall effect)
{
    uint32_t id,menu,list,root,fn,done=0,mode,enabled,request=1;
    put(m,0x8009C5E3,0);
    if(rrj_read32(m,0x800D712C) || rrj_read32(m,0x800D71EC)) rrj_write32(m,0x8005ACAC,0);
    id=h(m,0x8009C5D0);half(m,0x8009C5D8,id);
    menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*sh(id));rrj_write32(m,0x8009C5C8,menu);
    switch(id){
    case 0:case 1:case 2:case 6:case 11:case 12:case 15:case 16:case 19:case 20:case 22:case 58:
        if(b(m,0x8009C5E0)){call(m,effect,0x8007F158,1);put(m,0x8009C5E0,0);}request=0;break;
    case 3:request=0;break;
    case 4:case 5:case 29:put(m,0x800D80F3,0);break;
    case 40:put(m,0x800D80EB,b(m,0x8009C5F3));break;
    default:break;
    }
    if(request && b(m,0x8009C5DF) && !b(m,0x8009C5E0) && !(h(m,0x8009C5DA)&1)){
        call(m,effect,0x8007EDE0,b(m,0x8009C5DE));put(m,0x8009C5E0,1);
    }
    if(h(m,0x8009C5DA)&4) (void)rrj_video_dummy(m,0x8006FED4,0);
    menu=rrj_read32(m,0x8009C5C8);list=rrj_read32(m,menu+16);
    rrj_write32(m,0x8009C654,list?list+120*sh(h(m,menu+4)):0);
    (void)sub_F_800680E8(m,rrj_read32(m,0x8009C5C8),rrj_read32(m,0x8009C654));
    root=rrj_read32(m,0x8009C664);
    if(root){fn=rrj_read32(m,0x8009CC40+4*sb(b(m,root+2)));if(fn){if(!update) abort();done=update(m,fn,rrj_read32(m,0x8009C5C8));}}
    (void)sub_F_80064B30(m,rrj_read32(m,0x8009C5C8),rrj_read32(m,0x8009C654));
    while(h(m,0x8009C5D8)!=65535 && !done){
        menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*sh(h(m,0x8009C5D8)));
        fn=rrj_read32(m,0x8009C8C0+4*sh(h(m,menu+6)));rrj_write32(m,0x8009C5C8,menu);
        if(fn){if(!update) abort();done=update(m,fn,menu);}
        half(m,0x8009C5D8,h(m,rrj_read32(m,0x8009C5C8)+10));
    }
    mode=rrj_read32(m,0x800D80D8);enabled=0;
    if(mode==32) enabled=rrj_read32(m,0x8005AE7C)&1;
    if(mode==1) enabled=rrj_read32(m,0x8005AE7C)&2;
    if(mode==8) enabled=rrj_read32(m,0x8005AE7C)&4;
    if(enabled && (b(m,0x800D7182) || b(m,0x800D7192))){
        uint32_t left=b(m,0x800D7182);
        put(m,0x800D80E9,0);put(m,0x800D80DD,b(m,0x800D80DD)|24);call(m,effect,0x8002D250,0);
        mode=b(m,0x800D80DC);put(m,0x800D80DC,left?(mode==2?0:mode==4?2:4):(mode==2?4:mode==4?0:2));
    }
    sub_F_8006738C(m,0x800D81D8,effect);
    sub_F_800685BC(m,rrj_read32(m,0x8009C654));sub_F_8006310C(m,rrj_read32(m,0x8009C664),effect);
    if(rrj_s32(rrj_read32(m,0x8005ACAC))<1801) return 4;
    if(h(m,0x8009C5D0)==4) half(m,0x8009C5D2,58);
    return 58;
}
