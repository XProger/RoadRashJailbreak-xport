/* PsyQ default environments, verified against this executable's MIPS. */
#include "screen_env.h"
#include <stdlib.h>
static void half(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
static void byte(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
uint32_t sub_8004CC44(RRJMemory *m,uint32_t p,uint32_t x,uint32_t y,uint32_t w,uint32_t h,RRJScreenCall call)
{
    uint32_t mode;if(!call)abort();mode=call(m,0x80048428,0);
    half(m,p,x);half(m,p+2,y);half(m,p+4,w);
    half(m,p+12,0);half(m,p+14,0);half(m,p+16,0);half(m,p+18,0);
    byte(m,p+25,0);byte(m,p+26,0);byte(m,p+27,0);byte(m,p+22,1);half(m,p+6,h);
    byte(m,p+23,rrj_s32(h)<(mode?289:257));
    half(m,p+8,x);half(m,p+10,y);half(m,p+20,10);byte(m,p+24,0);return p;
}
uint32_t sub_8004CD04(RRJMemory *m,uint32_t p,uint32_t x,uint32_t y,uint32_t w,uint32_t h)
{
    half(m,p,x);half(m,p+2,y);half(m,p+4,w);
    half(m,p+8,0);half(m,p+10,0);half(m,p+12,0);half(m,p+14,0);
    byte(m,p+17,0);byte(m,p+16,0);byte(m,p+19,0);byte(m,p+18,0);half(m,p+6,h);return p;
}
