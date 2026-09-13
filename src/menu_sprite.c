/* Original F MIPS: resource selection and native sprite-packet producer. */
#include "menu_sprite.h"
#include "packet.h"
#include <stdlib.h>
static uint32_t b(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static void sb(RRJMemory *m,uint32_t a,uint32_t v) {*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static uint32_t h(RRJMemory *m,uint32_t a) {return rrj_u16(rrj_at(m,a,2));}
static void sh(RRJMemory *m,uint32_t a,uint32_t v) {rrj_put16(rrj_at(m,a,2),v);}
static uint32_t signed_byte(uint32_t v) {return v<128?v:v|0xffffff00;}
static uint32_t load(RRJMemory *m,RRJMenuResource resource,uint32_t function,uint32_t descriptor,uint32_t id)
{ if (!resource) abort();return resource(m,function,descriptor,id); }
/* A local C descriptor and an original RAM descriptor share the producer.
 * RAM reads remain lazy and ordered, so packet/data aliases retain MIPS effects. */
typedef struct {uint32_t address;const uint8_t *local;} ImageSource;
static uint32_t image_word(RRJMemory *m,ImageSource image,uint32_t offset)
{return image.local?rrj_u32(image.local+offset):rrj_read32(m,image.address+offset);}
static uint32_t image_half(RRJMemory *m,ImageSource image,uint32_t offset)
{return image.local?rrj_u16(image.local+offset):h(m,image.address+offset);}
static uint32_t draw_image(RRJMemory *m,uint32_t menu,ImageSource image,uint32_t link,RRJMenuResource resource)
{
 uint32_t id=image_word(m,image,0),index,descriptor,flags,result=1,cache=0,packet,context,width;
 int wide;
 for (index=0;index<300;++index) if (rrj_read32(m,0x80088E0C+index*4)==id) break;
 if (index==300) return 1;
 descriptor=0x8009DDE0+index*36;
 width=h(m,descriptor+20);wide=width>=257 && width<32768;
 flags=rrj_read32(m,descriptor);
 if (flags&0x00100000) {
  if (rrj_read32(m,0x800A0810)!=id) {
   if (!(flags&0x10000000)) result=0;
   else cache=0x800A0810;
  }
 } else if (flags&0x00200000) {
  link=rrj_read32(m,0x8009CCE4)+signed_byte(b(m,menu+12))*4+28;
  if (rrj_read32(m,0x800A0818)!=id) result=load(m,resource,0x80078F44,descriptor,id);
 } else if (flags&0x00400000) {
  if (h(m,0x8009C5DA)&1) link=rrj_read32(m,0x8009CCE4)+signed_byte(b(m,menu+12))*4+20;
  if (rrj_read32(m,0x800A0814)!=id) cache=0x800A0814;
 } else if (flags&0x00800000) {
  link=rrj_read32(m,0x8009CCE4)+signed_byte(b(m,menu+12))*4+24;
  if (rrj_read32(m,0x800A082C)!=id) cache=0x800A082C;
 } else if (flags&0x10000000) result=load(m,resource,0x8007A400,descriptor,id);
 else result=0;
 if (cache) {
  rrj_write32(m,descriptor,flags&0xdfffffff);
  result=load(m,resource,0x8007A400,descriptor,image_word(m,image,0));
  if (!result) return 0;
  rrj_write32(m,cache,image_word(m,image,0));
 }
 if (!result) return 0;
 context=rrj_read32(m,0x8005B470);packet=rrj_read32(m,context+268);
 if (packet+48>=rrj_read32(m,0x8005B4D0)) {
  uint32_t replacement=sub_80021C98(m,packet,48);
  rrj_write32(m,rrj_read32(m,0x8005B470)+268,replacement);
 }
 context=rrj_read32(m,0x8005B470);packet=rrj_read32(m,context+268);
 rrj_write32(m,context+268,packet+48); /* Reserve 48 even for one 24-byte packet. */
 rrj_write32(m,packet,rrj_read32(m,link)|0x05000000);
 rrj_write32(m,packet+4,(h(m,descriptor+28)&0x9ff)|0xe1000200);
 sh(m,packet+18,h(m,descriptor+30));
 rrj_write32(m,packet+8,image_word(m,image,4)|0x64000000);
 sh(m,packet+12,image_half(m,image,8));sh(m,packet+14,image_half(m,image,10));
 sb(m,packet+16,b(m,descriptor+24));sb(m,packet+17,b(m,descriptor+26));
 sh(m,packet+20,wide?256:h(m,descriptor+20));sh(m,packet+22,h(m,descriptor+22));
 rrj_write32(m,link,packet);
 if (wide) {
  uint32_t format,x,y;
  packet+=24;rrj_write32(m,packet,rrj_read32(m,link)|0x05000000);
  sh(m,packet+18,h(m,descriptor+30));
  rrj_write32(m,packet+8,image_word(m,image,4)|0x64000000);
  sh(m,packet+12,image_half(m,image,8)+256);sh(m,packet+14,image_half(m,image,10));
  sb(m,packet+16,b(m,descriptor+24));sb(m,packet+17,b(m,descriptor+26));
  sh(m,packet+20,h(m,descriptor+20)-256);sh(m,packet+22,h(m,descriptor+22));
  format=rrj_read32(m,descriptor)&3;
  if (format!=3) {
   y=h(m,descriptor+18);x=h(m,descriptor+16)+(64u<<format);
   rrj_write32(m,packet+4,0xe1000200|(format<<7)|((y&256)>>4)|((x&1023)>>6)|((y&512)*4));
  } /* Format 3 leaves the second page word untouched, as in MIPS. */
  rrj_write32(m,link,packet);
 }
 return result;
}
uint32_t sub_F_800700F0(RRJMemory *m,uint32_t menu,uint32_t image,uint32_t link,RRJMenuResource resource)
{
 ImageSource source={image,NULL};
 return draw_image(m,menu,source,link,resource);
}
uint32_t rrj_menu_image_local(RRJMemory *m,uint32_t menu,const uint8_t image[12],uint32_t link,RRJMenuResource resource)
{
 ImageSource source={0,image};
 if(!image) abort();
 return draw_image(m,menu,source,link,resource);
}
uint32_t sub_F_8006E894(RRJMemory *m,uint32_t menu,uint32_t entry,RRJMenuResource resource)
{
 uint32_t link;
 if ((h(m,entry+10)&32) && !(h(m,menu)&2)) return 1;
 link=rrj_read32(m,0x8009CFC8)+signed_byte(b(m,0x8009C5E1))*4+28;
 return sub_F_800700F0(m,menu,entry+16,link,resource);
}
