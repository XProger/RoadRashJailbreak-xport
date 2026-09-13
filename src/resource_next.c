/* Two-pass circular resource choice, F80062D6C actual MIPS. */
#include "resource_next.h"
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
static uint32_t blocked(RRJMemory *m,uint32_t record,uint32_t kind)
{
    uint32_t bit=b(m,record+1),index=sb(bit);
    index=(index>>3)|(index&0x80000000?0xe0000000:0);
    return (b(m,0x800D80D8+(kind==4?240:252)+index)>>(bit&7))&1;
}
static uint32_t eligible(RRJMemory *m,uint32_t record,uint32_t pass,uint32_t flags)
{
    uint32_t kind;
    if(!(b(m,record+3)&b(m,0x8009C5E4))) return 0;
    kind=b(m,record);
    if((kind==4 || kind==8) && blocked(m,record,kind)) return 0;
    if(pass) flags=b(m,record+2);
    if((flags&64) && !rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) return 0;
    if((flags&32) && rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) return 0;
    if((flags&128) && (b(m,0x800D70E1)>>4)!=8) return 0;
    return 1;
}
uint32_t sub_F_80062D6C(RRJMemory *m,uint32_t resource)
{
    uint32_t pass,seen=0;
    for(pass=0;pass<2;++pass){
        uint32_t count=sb(b(m,resource)),index=sb(b(m,resource+3)),visited=0;
        if(rrj_s32(count)<=0) continue;
        do {
            uint32_t record=rrj_read32(m,resource+4)+12*index,flags=pass?0:b(m,record+2);
            if(pass || (flags&16) || seen){
                if(eligible(m,record,pass,flags)) return index;
                seen=1;
            }
            count=sb(b(m,resource));++index;
            if(rrj_s32(index)>=rrj_s32(count)) index=0;
            ++visited;
        } while(rrj_s32(visited)<rrj_s32(count));
    }
    return 0xffffffff;
}
