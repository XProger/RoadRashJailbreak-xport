/* Original F video-stop bookkeeping, outside the WIP CD/MDEC boundary. */
#include "video_stop.h"
#include <stdlib.h>
uint32_t sub_F_8006FE6C(RRJMemory *m,RRJSDKCall stop)
{
    uint32_t channel,flags;
    if(rrj_u16(rrj_at(m,0x8009C5DA,2))&1){
        channel=rrj_u16(rrj_at(m,0x8009C688,2));if(channel&0x8000)channel|=0xffff0000;
        if(!stop)abort();stop(m,0x8005F7E0,channel,0);
        stop(m,0x8001460C,rrj_read32(m,0x8009C684),0);
        flags=rrj_u16(rrj_at(m,0x8009C5DA,2));
        rrj_write32(m,0x8009C684,0xffffffff);
        rrj_put16(rrj_at(m,0x8009C5DA,2),(uint16_t)(flags&0xfff8));
    }
    return 1;
}
uint32_t sub_F_8006FED4(RRJMemory *m,RRJSDKCall stop){return sub_F_8006FE6C(m,stop);}
