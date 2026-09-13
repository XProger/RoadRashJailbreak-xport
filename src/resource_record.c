/* Shared traversal verified separately against both 128-byte MIPS functions.
 * 64154 compares record+2 to an input halfword; 641D4 compares record+0 to 4. */
#include "resource_record.h"
static uint32_t find_record(RRJMemory *m,uint32_t field,uint32_t value)
{
    uint32_t object=rrj_read32(m,0x8009C664),table,index,record,count,i;
    if(!object) return 0;
    table=rrj_read32(m,object+4);
    if(!table) return 0;
    index=*(uint8_t *)rrj_at(m,object+3,1);
    if(index>=128) index|=0xffffff00;
    table+=12*index;
    count=rrj_u16(rrj_at(m,table+6,2));
    record=rrj_read32(m,table+8);
    if(count==0 || count>=32768) return 0;
    for(i=0;i<count;++i,record+=28)
        if(rrj_u16(rrj_at(m,record+field,2))==value) return record+4;
    return 0;
}
uint32_t sub_F_80064154(RRJMemory *m,uint32_t subtype)
{return find_record(m,2,subtype&65535);}
uint32_t sub_F_800641D4(RRJMemory *m)
{return find_record(m,0,4);}
