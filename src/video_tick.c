/* F8006DE5C: original video frame/end bookkeeping, codec/display boundaries. */
#include "video_tick.h"
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
uint32_t sub_F_8006DE5C(RRJMemory *m,uint32_t menu,RRJVideoPhaseCall cb)
{
    uint32_t flags=h(m,0x8009C5DA),item,desc,channel,args[9],result=1;
    if(!(flags&1))return 1;
    item=rrj_read32(m,menu+16);if(!item)return 1;desc=item+16;
    if(flags&4){
        channel=sh(h(m,0x8009C688));half(m,0x8009C5DA,flags&0xfff8);
        (void)call(m,cb,0x8005F7E0,channel,0,0,0);
        (void)call(m,cb,0x8001460C,rrj_read32(m,0x8009C684),0,0,0);
        rrj_write32(m,0x8009C684,0xffffffff);screen(m,cb,0);return 0;
    }
    args[4]=rrj_read32(m,desc+16);args[0]=sh(h(m,desc+12));args[1]=sh(h(m,desc+14));
    args[2]=sh(h(m,0x8009C688));args[3]=rrj_read32(m,desc+4);
    args[5]=args[6]=args[7]=args[8]=0;if(!cb)abort();
    if(!cb(m,0x8005F484,args)){close_video(m,cb);byte(m,0x8009C5E2,1);result=0;}
    rrj_write32(m,desc+16,rrj_read32(m,desc+16)+1);return result;
}
