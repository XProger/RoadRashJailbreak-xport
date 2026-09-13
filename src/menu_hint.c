/* MIPS rectangle alignment and button-label/image renderer. */
#include "menu_hint.h"
#include "font.h"
#include <stdlib.h>
static uint32_t h(RRJMemory *m,uint32_t a) {return rrj_u16(rrj_at(m,a,2));}
static uint32_t b(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t sxh(uint32_t v) {v&=65535;return v<32768?v:v|0xffff0000;}
static uint32_t sxb(uint32_t v) {return v<128?v:v|0xffffff00;}
static uint32_t asr1(uint32_t v) {return (v>>1)|(v&0x80000000);}
uint32_t sub_8002CB08(RRJMemory *m,uint32_t font,uint32_t id,uint32_t rect,uint32_t link,uint32_t color,uint32_t mode,RRJBlinkText blink)
{
    uint32_t text,x,y,width,box;
    if(mode>3) return 3;
    if(mode==3) {
        if(!(rrj_read32(m,0x8005ACA8)&16)) return 0;
        if(!blink) abort();
        return blink(m,font,id,rect,link,color);
    }
    if(mode==0) {
        x=sxh(h(m,rect));y=sxh(h(m,rect+2));
        text=rrj_read32(m,rrj_read32(m,0x8005B544)+4*id);
    } else {
        text=rrj_read32(m,rrj_read32(m,0x8005B544)+4*id);
        width=sub_8002D0D8(m,font,text);
        x=h(m,rect);box=h(m,rect+4);y=sxh(h(m,rect+2));
        if(mode==2) {width=asr1(width);box=asr1(sxh(box));}
        x=sxh(x+box-width);
        text=rrj_read32(m,rrj_read32(m,0x8005B544)+4*id);
    }
    return sub_8002CDC8(m,font,text,x,y,link,color);
}
uint32_t sub_F_8006E7A4(RRJMemory *m,uint32_t menu,uint32_t entry,RRJMenuResource resource,RRJBlinkText blink)
{
    uint32_t font,link,color,id,mode;
    if((h(m,entry+10)&32) && !(h(m,menu)&2)) return 1;
    font=rrj_read32(m,0x8009C5C0);
    if(b(m,0x800D8078+24*font)) {
        color=rrj_read32(m,entry+40);id=h(m,entry+28);mode=h(m,entry+30);
        link=rrj_read32(m,0x8009CFC8)+4*sxb(b(m,0x8009C5E1))+4;
        (void)sub_8002CB08(m,font,id,entry+32,link,color,mode,blink);
    }
    link=rrj_read32(m,0x8009CFC8)+4*sxb(b(m,0x8009C5E1))+20;
    return sub_F_800700F0(m,menu,entry+16,link,resource);
}
