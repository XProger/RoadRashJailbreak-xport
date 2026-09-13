/* F overlay IDA drafts audited against MIPS. WIP: actual item renderers
 * must be supplied by the native callback resolver; absence is not a no-op. */
#include "menu_draw.h"
#include <stdlib.h>
static uint32_t byte(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static int32_t shalf(RRJMemory *m,uint32_t a)
{ uint32_t v=rrj_u16(rrj_at(m,a,2));return v<32768?(int32_t)v:(int32_t)v-65536; }
static int32_t sbyte(RRJMemory *m,uint32_t a)
{ uint32_t v=byte(m,a);return v<128?(int32_t)v:(int32_t)v-256; }
uint32_t sub_F_8006D3E0(RRJMemory *m,uint32_t menu,RRJMenuDraw draw)
{
 uint32_t table=shalf(m,menu+2)?0x8009CF78:0x8009CF28;
 uint32_t list=rrj_read32(m,menu+16),index=0,offset=0;
 int32_t count;
 if (!list) return 0;
 count=shalf(m,menu+8);
 if (count<=0) return (uint32_t)count;
 do {
  uint32_t entry=rrj_read32(m,menu+16)+offset;
  uint32_t function=rrj_read32(m,table+(uint32_t)shalf(m,entry+8)*4);
  if (function) {
   uint32_t flags=rrj_read32(m,entry+4);
   uint32_t active=1u<<((uint32_t)sbyte(m,0x800D80DC)&31);
   if (((flags&0x3f00)>>8)&active) {
    if ((flags&0x02000000) && rrj_read32(m,0x800D742C+(uint32_t)sbyte(m,0x8009C5E6)*24)!=1) goto next;
    if ((flags&0x08000000) && rrj_read32(m,0x800D80D8)!=4) goto next;
    if ((flags&0x04000000) && rrj_read32(m,0x800D80D8)==4) goto next;
    if ((flags&0x10000000) && !byte(m,0x800D80F3)) goto next;
    if ((flags&0x80000000) && shalf(m,0x8009C5D0)!=shalf(m,0x8009C5D8)) goto next;
    if (!draw) abort();
    draw(m,function,menu,entry);
   }
  }
 next:
  count=shalf(m,menu+8);++index;offset+=120;
 } while (rrj_s32(index)<count);
 return 0; /* Final SLT overwrites any renderer return. */
}
uint32_t sub_F_8006D630(RRJMemory *m,uint32_t menu,RRJMenuDraw draw)
{
 *(uint8_t *)rrj_at(m,0x8009C5E1,1)=(uint8_t)byte(m,menu+12);
 sub_F_8006D3E0(m,menu,draw);
 return 1;
}
