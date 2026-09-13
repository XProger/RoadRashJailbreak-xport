/* 80012524..80012648: MIPS-audited update dispatcher; callee proof is separate. */
#include "race_step.h"
#include <stdlib.h>
uint32_t sub_80012524(RRJMemory *m,RRJRaceStepCall call)
{
    uint32_t state=rrj_read32(m,0x8005B2F8),delta=rrj_read32(m,state+24);
    uint32_t scaled,player,index=0,count,elapsed;
    if(!delta){rrj_write32(m,state+28,0);return 1;}
    if(rrj_s32(delta)>=31)delta=30;
    rrj_write32(m,state+28,delta);
    scaled=218u*delta;
    elapsed=rrj_read32(m,state+16);
    rrj_write32(m,0x8005B580,0);
    rrj_write32(m,state+16,elapsed+delta);
    if(!call)abort();
    (void)call(m,0x8008AB00,scaled,0);
    count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
    if(!count)return 0;
    player=0x800CD898;
    do {
        uint32_t x=rrj_read32(m,player+184),y=rrj_read32(m,player+188),z=rrj_read32(m,player+192);
        rrj_write32(m,player+476,z);
        rrj_write32(m,player+468,x);
        rrj_write32(m,player+472,y);
        (void)call(m,0x800881B4,player,scaled);
        if((rrj_read32(m,player+548)&256) && call(m,0x800A421C,player,0))
            (void)call(m,0x80086E1C,player,0);
        count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
        ++index;player+=1132;
    }while(index<count);
    return 0;
}
