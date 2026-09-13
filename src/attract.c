/* F8006A8FC: attract/video menu input and return selection. */
#include "attract.h"
#include <stdlib.h>
static int32_t byte(RRJMemory *m,uint32_t a)
{
    uint32_t b=*(uint8_t *)rrj_at(m,a,1);return b<128?(int32_t)b:(int32_t)b-256;
}
uint32_t sub_F_8006A8FC(RRJMemory *m,uint32_t menu,RRJAttractSound sound)
{
    static const uint8_t offsets[]={82,74,58,66,50,42,26,34,122,130,90,98,106,114};
    int32_t index,end;uint32_t pad,i,current;
    uint32_t phase=rrj_u16(rrj_at(m,menu+2,2));
    rrj_write32(m,0x8005ACAC,0);
    if(phase)return 0;
    if(!(rrj_u16(rrj_at(m,0x8009C5DA,2))&1)){
        current=rrj_u16(rrj_at(m,0x8009C5D0,2));if(current&0x8000)current|=0xffff0000;
        rrj_put16(rrj_at(m,0x8009C5D2,2),rrj_u16(rrj_at(m,0x8009C6C8+4*current,2)));
        return 1;
    }
    index=byte(m,menu+14);end=index+byte(m,menu+15);
    pad=0x800D7128+192*(uint32_t)index;
    while(index<end){
        for(i=0;i<sizeof offsets;++i)if(byte(m,pad+offsets[i])>0){
            if(!sound)abort();(void)sound(m,2);
            rrj_put16(rrj_at(m,0x8009C5DA,2),(uint16_t)(rrj_u16(rrj_at(m,0x8009C5DA,2))|4));break;
        }
        end=byte(m,menu+14)+byte(m,menu+15);++index;pad+=192;
    }
    return 0;
}
