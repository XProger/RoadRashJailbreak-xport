/* Original background colour setup and clear-before-display-switch. */
#include "screen.h"
#include "menu.h"
#include <stdlib.h>
uint32_t sub_8001BF1C(RRJMemory *m,uint32_t enabled,uint32_t red,uint32_t green,uint32_t blue,RRJScreenCall call)
{
    uint32_t context,i;
    if(!call)abort();(void)call(m,0x800487C0,0);(void)call(m,0x80047724,0);
    context=rrj_read32(m,0x8005B470);
    if(context)for(i=0;i<2;++i){
        uint8_t *p=rrj_at(m,context+40+112*i,4);
        p[0]=(uint8_t)enabled;p[1]=(uint8_t)red;p[2]=(uint8_t)green;p[3]=(uint8_t)blue;
    }
    return 0;
}
uint32_t sub_F_80080D08(RRJMemory *m,RRJScreenCall call)
{
    if(!call)abort();(void)call(m,0x80048944,0x80088C7C);
    (void)sub_8001C3F4(m);return call(m,0x8001C408,0);
}

uint32_t sub_8001BE08(RRJMemory *m,uint32_t x,uint32_t y,uint32_t width,uint32_t height,RRJScreenCall call,RRJScreenEnv env)
{
    uint32_t i,draw=0x800D6CC8,attr=0x800D6CE3,context,h,w;
    (void)x;if(!call||!env)abort();
    (void)call(m,0x800487C0,0);(void)call(m,0x80047724,0);
    rrj_write32(m,0x8005B470,0x800D6CB8);rrj_write32(m,0x800D6CC0,width);rrj_write32(m,0x800D6CC4,height);
    for(i=0;i<2;++i,draw+=112,attr+=112){
        context=rrj_read32(m,0x8005B470);h=rrj_read32(m,context+12);w=rrj_read32(m,context+8);
        env(m,0x8004CC44,draw,0,256*i,w,h);
        context=rrj_read32(m,0x8005B470);h=rrj_read32(m,context+12);w=rrj_read32(m,context+8);
        env(m,0x8004CD04,draw+92,0,256*i,w,h);
        *(uint8_t *)rrj_at(m,attr-5,1)=1;*(uint8_t *)rrj_at(m,attr-4,1)=0;
        rrj_put16(rrj_at(m,attr+79,2),(uint16_t)height);rrj_put16(rrj_at(m,attr+75,2),(uint16_t)y);
        *(uint8_t *)rrj_at(m,attr-3,1)=0;*(uint8_t *)rrj_at(m,attr-2,1)=0;
        *(uint8_t *)rrj_at(m,attr-1,1)=0;*(uint8_t *)rrj_at(m,attr,1)=0;
    }
    return 0;
}
