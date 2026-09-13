/* F preview lifecycle, actual nested stop/start; CD/codec are host boundaries. */
#include "video_preview.h"
#include "video_stop.h"
#include <stdlib.h>
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sh(uint32_t v){return v&0x8000?v|0xffff0000:v;}
static void half(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
uint32_t sub_F_8006FEF4(RRJMemory *m,uint32_t desc,RRJVideoPhaseCall cb,RRJVideoPhaseOpen open,RRJSDKCall stop)
{
    uint32_t args[9]={0},file,flags,channel;
    if(!cb)abort();if(cb(m,0x80022A78,args))return 0;
    if(h(m,0x8009C5DA)&1)return 0;
    if(!open)abort();file=open(m,0x8005BF84,rrj_read32(m,0x8008973C+4*rrj_read32(m,desc)));
    rrj_write32(m,0x8009C684,file);if(rrj_s32(file)<0)return 1;
    rrj_write32(m,desc+16,0);flags=h(m,0x8009C5DA);args[0]=rrj_read32(m,0x8009C684);
    half(m,0x8009C5DA,flags|1);channel=h(m,desc+8);half(m,0x8009C688,channel);
    args[3]=sh(h(m,desc+12));args[4]=sh(h(m,desc+14));args[5]=2048;args[6]=64;args[7]=1024;
    args[8]=h(m,desc+10);args[1]=rrj_read32(m,desc+4);args[2]=sh(channel);
    if(!cb(m,0x8005F36C,args)){(void)sub_F_8006FE6C(m,stop);return 0;}
    rrj_write32(m,desc+16,rrj_read32(m,desc+16)+1);return 1;
}
uint32_t sub_F_80070018(RRJMemory *m,uint32_t desc,RRJVideoPhaseCall cb,RRJVideoPhaseOpen open,RRJSDKCall stop)
{
    uint32_t flags=h(m,0x8009C5DA),args[9]={0},result=1;
    if(flags&4)return sub_F_8006FE6C(m,stop);
    if(!(flags&1)){
        if(flags&2)return sub_F_8006FEF4(m,desc,cb,open,stop);
        half(m,0x8009C5DA,flags|2);return 0;
    }
    args[4]=rrj_read32(m,desc+16);args[0]=sh(h(m,desc+12));args[1]=sh(h(m,desc+14));
    args[2]=sh(h(m,0x8009C688));args[3]=rrj_read32(m,desc+4);if(!cb)abort();
    if(!cb(m,0x8005F484,args)){half(m,0x8009C5DA,(h(m,0x8009C5DA)&0xfffe)|2);result=0;}
    rrj_write32(m,desc+16,rrj_read32(m,desc+16)+1);return result;
}
