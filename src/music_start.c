/* Music request/start and CD read bookkeeping, original F MIPS. */
#include "music_start.h"
#include <stdlib.h>
static uint32_t call(RRJMemory *m,RRJMusicCall cb,uint32_t fn,uint32_t a,uint32_t b)
{
    uint32_t args[8]={a,b,0,0,0,0,0,0};if(!cb)abort();return cb(m,fn,args);
}
uint32_t sub_F_8007EF64(RRJMemory *m,RRJMusicCall cb)
{
    uint32_t args[8],offset,index;
    args[3]=0;args[1]=rrj_read32(m,0x800A084C);args[2]=rrj_read32(m,0x800A0850);args[0]=512;
    index=rrj_read32(m,0x800A0864);args[4]=rrj_read32(m,0x800A0870+4*(index&1));
    args[5]=rrj_read32(m,0x800A0864);args[6]=0x8007F028;args[7]=0x4000;if(!cb)abort();
    if(cb(m,0x80022D20,args)==0xffffff9c){(void)call(m,cb,0x80022A78,2,0);(void)cb(m,0x80022D20,args);}
    offset=rrj_read32(m,0x800A0850)+0x4000;index=rrj_read32(m,0x800A0864);
    rrj_write32(m,0x800A0850,offset);rrj_write32(m,0x800A0864,index+1);return offset;
}
uint32_t sub_F_8007EEB8(RRJMemory *m,uint32_t track,RRJMusicCall cb)
{
    uint32_t i,active;if(track>=18)return 0;
    if(rrj_read32(m,0x800A0830)!=track){
        rrj_write32(m,0x800A0864,0);rrj_write32(m,0x800A0858,0);(void)call(m,cb,0x8007F158,1,0);
        active=rrj_read32(m,0x800A0878);rrj_write32(m,0x800A0850,track*458752);
        if(active)(void)call(m,cb,0x80022A1C,2,0);rrj_write32(m,0x800A0878,1);
        for(i=0;i<28;++i)(void)sub_F_8007EF64(m,cb);
    }
    rrj_write32(m,0x800A0830,track);return 0x800a0000;
}
uint32_t sub_F_8007EDE0(RRJMemory *m,uint32_t track,RRJMusicCall cb)
{
    uint32_t args[8]={0},handle;if(track>=18)return 0;
    if(rrj_read32(m,0x800A0830)!=track){(void)sub_F_8007EEB8(m,track,cb);rrj_write32(m,0x800A0838,1);return 1;}
    if(rrj_read32(m,0x800A0834))return 0x800a0000;
    if(rrj_s32(rrj_read32(m,0x800A0858))<12){rrj_write32(m,0x800A0838,1);return 1;}
    args[0]=1;args[1]=1486;args[2]=rrj_read32(m,0x800D6C14);args[3]=0;args[4]=65456;if(!cb)abort();
    handle=cb(m,0x8001F37C,args);rrj_write32(m,0x800A0844,handle);
    args[0]=1;args[3]=127;args[4]=294832;handle=cb(m,0x8001F37C,args);rrj_write32(m,0x800A0848,handle);
    (void)call(m,cb,0x8001F544,2,0x800A0844);rrj_write32(m,0x800A0834,1);rrj_write32(m,0x800A0838,0);return 1;
}
