/* F8006DB5C, full phase branches. CD/codec/display effects use typed boundaries. */
#include "video_phase.h"
#include <stdlib.h>
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sh(uint32_t v){return v&0x8000?v|0xffff0000:v;}
static void half(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
static void byte(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static uint32_t call(RRJMemory *m,RRJVideoPhaseCall cb,uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t d)
{
    uint32_t args[9]={a,b,c,d,0,0,0,0,0};if(!cb)abort();return cb(m,fn,args);
}
static void screen(RRJMemory *m,RRJVideoPhaseCall cb,uint32_t video)
{
    (void)call(m,cb,0x80080D08,0,0,0,0);
    (void)call(m,cb,0x8001BE08,0,video?8:0,video?320:512,video?224:240);
    (void)call(m,cb,0x8001BF1C,1,0,0,0);
    (void)call(m,cb,0x8001C3F4,0,0,0,0);(void)call(m,cb,0x8001C408,0,0,0,0);
    byte(m,rrj_read32(m,0x8005B470)+125,video);
    byte(m,rrj_read32(m,0x8005B470)+237,video);
}
static void close_video(RRJMemory *m,RRJVideoPhaseCall cb)
{
    uint32_t flags;
    (void)call(m,cb,0x8005F7E0,sh(h(m,0x8009C688)),0,0,0);
    (void)call(m,cb,0x8001460C,rrj_read32(m,0x8009C684),0,0,0);
    flags=h(m,0x8009C5DA);rrj_write32(m,0x8009C684,0xffffffff);
    half(m,0x8009C5DA,flags&0xfffe);screen(m,cb,0);
}
uint32_t sub_F_8006DB5C(RRJMemory *m,uint32_t menu,RRJVideoPhaseCall cb,RRJVideoPhaseOpen open)
{
    uint32_t item=rrj_read32(m,menu+16),desc,phase,file,channel,args[9];
    if(!item)return 0;desc=item+16;
    if(!(h(m,menu)&2)){
        if(h(m,0x8009C5DA)&1)close_video(m,cb);
        half(m,menu+2,0);byte(m,0x8009C5E2,1);return 0;
    }
    phase=h(m,menu+2);
    if(phase!=32767){half(m,menu+2,phase-1);(void)call(m,cb,0x8006DE5C,menu,0,0,0);return 0;}
    if(call(m,cb,0x80022A78,0,0,0,0))return 0;
    if(h(m,0x8009C5DA)&1)return 0;
    if(!open)abort();file=open(m,0x8005BF84,rrj_read32(m,0x8008973C+4*rrj_read32(m,desc)));
    rrj_write32(m,0x8009C684,file);
    if(rrj_s32(file)>=0){
        screen(m,cb,1);
        channel=h(m,desc+8);rrj_write32(m,desc+16,0);
        args[0]=rrj_read32(m,0x8009C684);half(m,0x8009C688,channel);
        args[2]=sh(channel);args[3]=sh(h(m,desc+12));args[4]=sh(h(m,desc+14));
        args[5]=2048;args[6]=64;args[7]=1024;args[8]=h(m,desc+10);args[1]=rrj_read32(m,desc+4);
        if(!cb)abort();
        if(cb(m,0x8005F36C,args)){
            rrj_write32(m,desc+16,rrj_read32(m,desc+16)+1);half(m,0x8009C5DA,h(m,0x8009C5DA)|1);
        }else close_video(m,cb);
    }
    byte(m,0x8009C5E2,2);half(m,menu+2,3);return 0;
}
