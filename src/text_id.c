/* Actual MIPS entry 800662CC includes the LUI/LW omitted by IDA. */
#include "text_id.h"
#include "word_wrap.h"
uint32_t sub_F_800662CC(RRJMemory *m,uint32_t font,uint32_t id,uint32_t rect,uint32_t gap,uint32_t link,uint32_t color)
{
    uint32_t table=rrj_read32(m,0x8005B544);
    if(!table) return 0x80060000; /* v0 retains the entry LUI result. */
    gap&=65535;
    if(gap>=32768) gap|=0xffff0000;
    return sub_F_80066318(m,font,rrj_read32(m,table+(id<<2)),rect,gap,link,color);
}
uint32_t rrj_blink_text(RRJMemory *m,uint32_t font,uint32_t id,uint32_t rect,uint32_t link,uint32_t color)
{
    return sub_F_800662CC(m,font,id,rect,0,link,color);
}
