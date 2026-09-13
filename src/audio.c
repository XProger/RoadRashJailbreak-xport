/* Literal RRJ audio bookkeeping. SPU output remains a required SDK binding;
 * this module does not silently discard audio requests. See original MIPS. */
#include "audio.h"
static uint32_t asr(uint32_t v, unsigned n)
{ return (v >> n) | ((v & 0x80000000) ? (~0u << (32-n)) : 0); }
static uint32_t merge_mask(RRJMemory *m, uint32_t a, uint32_t mask)
{ uint32_t v=rrj_read32(m,a)|mask; rrj_write32(m,a,v); return v; }
/* Verified twin: same seven words, only field offset +0x14/+0x18 changes. */
uint32_t sub_8001EB28(RRJMemory *m, uint32_t mask) { return merge_mask(m,0x800D6884,mask); }
uint32_t sub_8001EB44(RRJMemory *m, uint32_t mask) { return merge_mask(m,0x800D6888,mask); }
uint32_t sub_8001E86C(RRJMemory *m, uint32_t bank, uint32_t index)
{
 uint32_t count=*(uint8_t *)rrj_at(m,bank+4,1), offset;
 if (rrj_s32(index)<0 || rrj_s32(index)>=(int32_t)count) return 0;
 offset=rrj_read32(m,bank+16+index*4);
 return offset ? bank+offset : 0;
}
uint32_t sub_8001F9C4(RRJMemory *m, uint32_t reserved)
{
 uint32_t top=rrj_read32(m,0x800D69E4), slot=0xffffffff, a, next, voice, sequence, i;
 if (rrj_s32(top)>=0) {
  a=0x800D6920+top*4; slot=rrj_read32(m,a); rrj_write32(m,a,0xffffffff);
  rrj_write32(m,0x800D69E4,rrj_read32(m,0x800D69E4)-1);
 } else {
  uint32_t head=rrj_read32(m,0x800D69E8),tail=rrj_read32(m,0x800D69EC);
  if (head!=tail) {
   a=0x800D6980+tail*4;slot=rrj_read32(m,a);rrj_write32(m,a,0xffffffff);
   next=rrj_read32(m,0x800D69EC)+1;rrj_write32(m,0x800D69EC,next);
   rrj_write32(m,0x800D69EC,rrj_s32(next)<25?next:0);
   sub_8001EB44(m,1u<<(slot&31));
  }
 }
 if (slot==0xffffffff) return 0;
 voice=rrj_read32(m,0x800D687C)+slot*44;
 if (reserved) {
  for (i=0;i<22;++i) if (rrj_read32(m,0x800D68C8+i*4)==0xffffffff) {
   rrj_write32(m,0x800D68C8+i*4,slot);break;
  }
 } else {
  next=rrj_read32(m,0x800D69E8);
  rrj_write32(m,0x800D6980+next*4,slot);++next;
  rrj_write32(m,0x800D69E8,next);rrj_write32(m,0x800D69E8,rrj_s32(next)<25?next:0);
 }
 sequence=(rrj_read32(m,0x8005B4A0)+1)&0x07ffffff;
 rrj_write32(m,0x8005B4A0,sequence);
 if (!sequence) {sequence=1;rrj_write32(m,0x8005B4A0,1);}
 rrj_write32(m,voice+4,sequence);rrj_write32(m,voice+16,1);
 return voice;
}
/* Five original arguments, not IDA's nine. parameters is a packed three-word
 * record: pitch, volume, pan. Read pitch after allocation, preserving aliasing. */
uint32_t sub_8001F174(RRJMemory *m, uint32_t bank, uint32_t sample, uint32_t loop, uint32_t reserved, const void *parameters)
{
 const uint8_t *p=(const uint8_t *)parameters;
 uint32_t base, record, left, right, voice, pitch, channel, handle;
 if (rrj_s32(rrj_read32(m,0x800D6870))<rrj_s32(bank) || rrj_s32(bank)<0) return 0;
 base=rrj_read32(m,rrj_read32(m,0x800D6874)+bank*4);
 if (!base) return 0;
 record=sub_8001E86C(m,base,sample);
 if (!record) return 0;
 record+=4; /* Nonzero third argument is a flag; original resets its index. */
 if (record&3) return 0;
 if (rrj_read32(m,0x800D69F4)) {
  left=rrj_u32(p+4) * *(uint8_t *)rrj_at(m,record,1);right=left;
 } else {
  uint32_t pan=rrj_u32(p+8),folded=rrj_s32(pan)<129?pan:256-pan;
  uint32_t volume=rrj_u32(p+4) * *(uint8_t *)rrj_at(m,record,1);
  uint32_t product=(folded-64)*volume,shifted=asr(product,6);
  left=(volume-(shifted&asr(0-shifted,31)))^asr(pan,31);
  if (rrj_s32(pan)>=129) left=0-left;
  right=volume+(shifted&asr(product,31));
 }
 voice=sub_8001F9C4(m,reserved);
 if (!voice) return 0;
 rrj_write32(m,voice+8,record);
 pitch=rrj_u32(p);
 rrj_write32(m,voice+32,pitch==0xffffffff?rrj_u16(rrj_at(m,record+6,2)):pitch);
 rrj_write32(m,voice+36,left);rrj_write32(m,voice+40,right);
 if (loop) rrj_sdk_call(m,0x80050678,0,1u<<(rrj_read32(m,voice+28)&31));
 channel=rrj_read32(m,voice+28);
 handle=(channel<<27)+(rrj_read32(m,voice+4)&0x07ffffff);
 sub_8001EB28(m,1u<<(channel&31));return handle;
}
uint32_t sub_F_8007EAC0(RRJMemory *m, uint32_t event)
{
 uint8_t parameters[12];
 if (event>=15) return 0;
 rrj_put32(parameters,0xffffffff);rrj_put32(parameters+4,rrj_read32(m,0x800D6C0C));rrj_put32(parameters+8,64);
 return sub_8001F174(m,rrj_read32(m,0x800A0854),event,0,0,parameters);
}
