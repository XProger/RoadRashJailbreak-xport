/* MIPS jump-table audit: status/menu/entry-kind/audit.json. */
#include "entry_kind.h"
uint32_t sub_F_80072100(RRJMemory *m,uint32_t entry)
{
    static const uint8_t kinds[80]={
        0,1,9,9,3,9,9,9,0,7,7,1,6,4,9,1,
        9,4,6,2,0,9,9,9,6,1,7,9,9,9,9,9,
        0,9,6,4,1,5,0,9,6,4,1,5,1,6,4,0,
        9,6,4,9,1,5,9,0,9,6,9,2,9,1,4,9,
        9,9,9,9,9,9,9,9,9,9,9,9,0,9,8,8,
    };
    uint32_t type=rrj_u16(rrj_at(m,entry+8,2));
    uint32_t subtype;
    if(type!=12 && type!=13) return 0xffffffff;
    subtype=rrj_u16(rrj_at(m,entry+18,2));
    return subtype<80?kinds[subtype]:9;
}
