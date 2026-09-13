/* Resident MIPS font lookup, width and textured packet generation. WIP until
 * live menu integration. The actual 8002CDC8 ABI has SIX arguments. */
#include "font.h"
#include "packet.h"
static uint32_t byte(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t half(RRJMemory *m,uint32_t a) {return rrj_u16(rrj_at(m,a,2));}
static uint32_t sbits(uint32_t b) {return b<128?b:b|0xffffff00;}
static uint32_t hbits(uint32_t h) {h&=65535;return h<32768?h:h|0xffff0000;}
static uint32_t asr1(uint32_t n) {return (n>>1)|(n&0x80000000);}
/* PsyQ strlen boundary, operating on original RAM data rather than casting an
 * unbounded original pointer to a host string. No SDK function is registered. */
static uint32_t text_length(RRJMemory *m,uint32_t s)
{uint32_t n=0;while(byte(m,s+n)) ++n;return n;}
uint32_t sub_8002D1F8(RRJMemory *m,uint32_t ch,uint32_t table,uint32_t count,uint32_t stride)
{
    ch&=255;
    while(count) {
        uint32_t item=table+asr1(count)*stride,c=byte(m,item);
        if(ch==c) return item;
        if(ch>c) {table=item+stride;--count;}
        count=asr1(count);
    }
    return 0;
}
uint32_t sub_8002D1A0(RRJMemory *m,uint32_t ch,uint32_t header,uint32_t glyphs)
{
    uint32_t item;
    ch&=255;item=glyphs+(ch-32)*11;
    if(byte(m,item)==ch) return item;
    return sub_8002D1F8(m,ch,glyphs,half(m,header+10),11);
}
uint32_t sub_8002D0D8(RRJMemory *m,uint32_t font,uint32_t text)
{
    uint32_t total=0,record,i,n,glyph;
    if(font==0xffffffff) return 0;
    record=0x800D8078+24*font;n=text_length(m,text);
    for(i=0;rrj_s32(i)<rrj_s32(n);++i) {
        glyph=sub_8002D1A0(m,byte(m,text+i),rrj_read32(m,record+4),rrj_read32(m,record+8));
        if(glyph) total+=sbits(byte(m,glyph+8));
    }
    return hbits(total);
}
uint32_t sub_8002CDC8(RRJMemory *m,uint32_t font,uint32_t text,uint32_t x,uint32_t y,uint32_t link,uint32_t color)
{
    uint32_t record,clut,page,n,space=8,glyph,context,packet,cursor,i,emitted=0;
    if(font==0xffffffff) return 0xffffffff;
    y&=65535;record=0x800D8078+24*font;
    clut=half(m,0x8005B538+2*sbits(byte(m,record+3)));page=half(m,record+16);
    n=text_length(m,text);
    glyph=sub_8002D1A0(m,32,rrj_read32(m,record+4),rrj_read32(m,record+8));
    if(glyph) space=sbits(byte(m,glyph+8));
    context=rrj_read32(m,0x8005B470);cursor=rrj_read32(m,context+268);
    if(cursor+40*n>=rrj_read32(m,0x8005B4D0)) {
        cursor=sub_80021C98(m,cursor,40*n);
        rrj_write32(m,rrj_read32(m,0x8005B470)+268,cursor);
    }
    packet=rrj_read32(m,rrj_read32(m,0x8005B470)+268);color|=0x2e000000;
    for(i=0;rrj_s32(i)<rrj_s32(n);++i) {
        uint32_t ch=byte(m,text+i),left,right,top,bottom,u0,u1,v0,v1,w,h,dx,dy;
        if(ch==32) {x+=space;continue;}
        glyph=sub_8002D1A0(m,ch,rrj_read32(m,record+4),rrj_read32(m,record+8));
        if(!glyph) continue;
        ++emitted;
        dx=sbits(byte(m,glyph+9));dy=sbits(byte(m,glyph+10));
        u0=byte(m,record+18)+byte(m,glyph+4);w=byte(m,glyph+2);
        v0=byte(m,record+19);h=byte(m,glyph+3);
        left=hbits(x+dx);right=hbits(x+dx+w);u1=(u0+w)&255;u0&=255;
        v0+=byte(m,glyph+6);v1=((v0+h)&255)<<8;v0=(v0&255)<<8;
        /* MIPS ORs SIGN-extended x into packed XY. Do not mask it to 16 bits:
         * negative x intentionally reproduces the original upper-half effect. */
        rrj_write32(m,packet,rrj_read32(m,link)|0x09000000);
        rrj_write32(m,packet+4,color);top=(y+dy)<<16;bottom=(y+dy+h)<<16;
        rrj_write32(m,packet+8,left|top);
        rrj_write32(m,packet+16,right|top);
        rrj_write32(m,packet+12,u0|v0|(clut<<16));
        rrj_write32(m,packet+24,left|bottom);
        rrj_write32(m,packet+28,u0|v1);
        rrj_write32(m,packet+32,right|bottom);
        rrj_write32(m,packet+36,u1|v1);
        rrj_write32(m,packet+20,u1|v0|(page<<16));
        rrj_write32(m,link,packet);
        x+=sbits(byte(m,glyph+8));packet+=40;
    }
    context=rrj_read32(m,0x8005B470);cursor=rrj_read32(m,context+268);
    rrj_write32(m,context+268,cursor+40*emitted);
    return 40*emitted;
}
