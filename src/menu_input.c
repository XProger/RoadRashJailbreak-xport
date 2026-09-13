/* IDA drafts reconciled with F overlay MIPS; controller is still WIP
 * until its frame caller and music replay dependency are integrated. */
#include "menu_input.h"
#include "menu_navigation.h"
#include "audio.h"
#include <stdlib.h>
static uint32_t b(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static void sb(RRJMemory *m,uint32_t a,uint32_t v) {*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static uint32_t h(RRJMemory *m,uint32_t a) {return rrj_u16(rrj_at(m,a,2));}
static void sh(RRJMemory *m,uint32_t a,uint32_t v) {rrj_put16(rrj_at(m,a,2),v);}
static int32_t sx8(uint32_t v) {v&=255;return v<128?(int32_t)v:(int32_t)v-256;}
static int32_t sx16(uint32_t v) {v&=65535;return v<32768?(int32_t)v:(int32_t)v-65536;}
static void flag(RRJMemory *m,uint32_t v) {sb(m,0x8009C5E3,b(m,0x8009C5E3)|v);}
uint32_t sub_F_8006C700(RRJMemory *m,uint32_t menu,uint32_t subtype)
{
 int32_t count=sx16(h(m,menu+8)),wanted=sx16(subtype),i;
 uint32_t entry;
 if (count<=0) return 0xffffffff;
 entry=rrj_read32(m,menu+16);
 for (i=0;i<count;++i,entry+=120) {
  int32_t type=sx16(h(m,entry+8));
  if (type>=12 && type<14 && (int32_t)h(m,entry+18)==wanted) return (uint32_t)i;
 }
 return 0xffffffff;
}
/* The two back-button blocks have identical data/control effects. */
static uint32_t back(RRJMemory *m,uint32_t menu)
{
 uint32_t current=h(m,0x8009C5D0);
 uint32_t entry=0x8009C7B8+(uint32_t)sx16(current)*4;
 if (rrj_read32(m,entry)==0xffffffff) {sub_F_8007EAC0(m,4);return 2;}
 if ((h(m,menu)&0x400) && b(m,0x800D80F0)) {
  uint32_t target=b(m,rrj_read32(m,0x8005B2F8)+4)==4?55:54;
  uint32_t table;
  sh(m,0x8009C5D2,target);
  table=rrj_read32(m,0x8009C68C);
  sb(m,0x8009C5E8,b(m,entry));
  sh(m,rrj_read32(m,table+target*4)+10,current);
  sb(m,rrj_read32(m,table+target*4)+14,b(m,menu+14));
  sb(m,rrj_read32(m,rrj_read32(m,0x8009C68C)+target*4)+15,b(m,menu+15));
  flag(m,4);sub_F_8007EAC0(m,3);return 1;
 }
 entry=0x8009C7B8+(uint32_t)sx16(h(m,0x8009C5D0))*4;
 sh(m,0x8009C5D2,h(m,entry));sub_F_8007EAC0(m,3);flag(m,4);return 1;
}
uint32_t sub_F_8006B03C(RRJMemory *m,uint32_t menu,uint32_t first,uint32_t end,RRJMenuMusic music)
{
 uint32_t player;
 if (!rrj_read32(m,menu+16)) return 0;
 /* Back from either physical pad has priority over the configured pad range. */
 for (player=0;player<2;++player) if (sx8(b(m,0x800D7128+192*player+74))>0) return back(m,menu);
 for (player=first;rrj_s32(player)<rrj_s32(end);++player) {
  uint32_t pad=0x800D7128+192*player,slider,table,entry,value;
  int32_t type;
  if (b(m,pad+42)) {flag(m,16);sub_F_8006C558(m,menu,sub_F_8007EAC0);return 2;}
  if (b(m,pad+50)) {flag(m,32);sub_F_8006C3A4(m,menu,sub_F_8007EAC0);return 2;}
  if (b(m,pad+26) || b(m,pad+34)) {
   int left=b(m,pad+26)!=0;
   slider=rrj_read32(m,0x8009C64C);flag(m,left?64:128);
   if (slider) {
    uint32_t step=(uint32_t)sx16(h(m,slider+8));
    value=rrj_read32(m,slider+12);value=left?value-step:value+step;
    rrj_write32(m,slider+12,value);
    slider=rrj_read32(m,0x8009C64C);value=rrj_read32(m,slider+12);
    table=rrj_read32(m,slider+(left?0:4));
    if (left?rrj_s32(value)<rrj_s32(table):rrj_s32(table)<rrj_s32(value)) rrj_write32(m,slider+12,table);
    table=rrj_read32(m,0x8009C650);
    if (table) rrj_write32(m,table,rrj_read32(m,rrj_read32(m,0x8009C64C)+12));
   }
   return 2;
  }
  if (sx8(b(m,pad+82))>0) {
   flag(m,1);table=0x8009C6C8+(uint32_t)sx16(h(m,0x8009C5D0))*4;
   if (rrj_read32(m,table)!=0xffffffff) {
    sh(m,0x8009C5D2,h(m,table));sub_F_8007EAC0(m,2);return 1;
   }
   entry=rrj_read32(m,menu+16)+(uint32_t)sx16(h(m,menu+4))*120;
   type=sx16(h(m,entry+8));
   if (type>=12 && type<14) {
    uint32_t offset=h(m,entry+18)*4;
    table=0x8009CAF8+offset;
    if (rrj_read32(m,table)!=0xffffffff) {
     sh(m,0x8009C5D2,h(m,table));sub_F_8007EAC0(m,2);return 1;
    }
    table=0x8009C9B0+offset;
   } else if (type==17) table=0x8009C8A8+h(m,entry+100)*4;
   else {sub_F_8007EAC0(m,4);return 2;}
   if (rrj_read32(m,table)==0xffffffff) {sub_F_8007EAC0(m,4);return 2;}
   sh(m,menu+4,sub_F_8006C700(m,menu,(uint32_t)sx16(h(m,table))));
   sub_F_8007EAC0(m,2);return 2;
  }
  if (sx8(b(m,pad+74))>0) return back(m,menu);
  if (sx8(b(m,pad+58))>0) {
   if (h(m,menu)&0x100) {
    if (!b(m,0x8009C5EE)) sub_F_8007EAC0(m,4);
    else if (b(m,0x8009C5DF)) {
     if (!music) abort();
     music(m,b(m,0x8009C5DE));sb(m,0x8009C5E0,1);
    }
    return 2;
   }
   value=b(m,0x8009C5D0);sb(m,0x800D80E8,value);
   table=rrj_read32(m,0x8009C68C)+(uint32_t)sx8(value)*4;
   value=b(m,rrj_read32(m,table)+4);sh(m,0x8009C5D2,42);sb(m,0x800D80E5,value);
   sub_F_8007EAC0(m,2);flag(m,8);return 1;
  }
  if (sx8(b(m,pad+66))>0) {
   if (h(m,menu)&0x200) {sub_F_8007EAC0(m,4);return 2;}
   sub_F_8007EAC0(m,2);sh(m,0x8009C5D2,53);
   table=rrj_read32(m,0x8009C68C);flag(m,2);
   sh(m,rrj_read32(m,table+212)+10,h(m,menu+6));
   sb(m,rrj_read32(m,table+212)+14,b(m,menu+14));
   sb(m,rrj_read32(m,rrj_read32(m,0x8009C68C)+212)+15,b(m,menu+15));
   return 2; /* Original deliberately returns handled, not transition. */
  }
 }
 return 0;
}
uint32_t sub_F_80069418(RRJMemory *m,uint32_t menu,RRJMenuMusic music)
{
 int32_t first=sx8(b(m,menu+14)),end=first+sx8(b(m,menu+15));
 return sub_F_8006B03C(m,menu,(uint32_t)first,(uint32_t)end,music);
}
