/* Select first signed-byte value in a resource directory. */
#include "resource_value.h"
static uint32_t signed_byte(RRJMemory *m,uint32_t address)
{
    uint32_t value=*(uint8_t *)rrj_at(m,address,1);
    return value<128?value:value|0xffffff00;
}
uint32_t sub_F_800630C0(RRJMemory *m,uint32_t resource,uint32_t value)
{
    uint32_t count=signed_byte(m,resource),table,index;
    if(rrj_s32(count)<=0) return count;
    table=rrj_read32(m,resource+4);
    for(index=0;index<count;++index,table+=12){
        uint32_t current=signed_byte(m,table+1);
        if(current==value){*(uint8_t *)rrj_at(m,resource+3,1)=(uint8_t)index;return current;}
    }
    return 0;
}
