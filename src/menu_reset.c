/* F6883C: player/mode reset, reconciled with full MIPS switch. */
#include "menu_reset.h"
#include "menu_flags.h"
#include <stdlib.h>
static void B(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static void H(RRJMemory *m,uint32_t a,uint32_t v){rrj_put16(rrj_at(m,a,2),(uint16_t)v);}
static uint32_t S(RRJMemory *m,uint32_t a){uint32_t v=*(uint8_t *)rrj_at(m,a,1);return v&128?v|0xffffff00:v;}
static uint32_t C(RRJMemory *m,RRJResetCall cb,uint32_t fn,uint32_t a,uint32_t b,uint32_t c){if(!cb)abort();return cb(m,fn,a,b,c);}
static void R(RRJMemory *m,RRJResetCall cb,uint32_t slot,uint32_t value){(void)C(m,cb,0x800630C0,rrj_read32(m,slot),value,0);}
static uint32_t select_item(RRJMemory *m,RRJResetCall cb,uint32_t id)
{
    uint32_t menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*id),value=C(m,cb,0x80080A70,menu,0,0);
    menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*id);H(m,menu+4,value);return value;
}
uint32_t sub_F_8006883C(RRJMemory *m,uint32_t id,RRJResetCall cb)
{
    uint32_t i,p,v,result;
    (void)sub_8002D250(m);B(m,0x800D80E9,0);B(m,0x800D81DE,127);B(m,0x800D8202,127);
    B(m,0x800D80DD,48);B(m,0x800D80ED,0);B(m,0x800D80EE,0);H(m,0x8009C5D4,40);B(m,0x800D80F5,0);
    for(i=0,p=0x800D81D8;i<6;++i,p+=36){H(m,p+12,0);B(m,p+14,0);B(m,p+15,0);rrj_write32(m,p+16,0);B(m,p+20,0);B(m,p+22,0);}
    (void)C(m,cb,0x8001E100,0x800D8118,0,128);(void)C(m,cb,0x8001E100,0x800D80F8,0,16);(void)C(m,cb,0x8001E100,0x800D8108,0,16);
    switch(id){
    case 7:
        rrj_write32(m,0x800D80D8,32);B(m,0x800D80DE,1);B(m,0x800D81E1,0);R(m,cb,0x8009C4B0,0);
        B(m,0x800D81E2,0);R(m,cb,0x8009C4B4,0);R(m,cb,0x8009C4B8,0);(void)select_item(m,cb,id);
        /* original falls through into mode8 setup */
    case 8:
        rrj_write32(m,0x800D80D8,32);B(m,0x800D80F3,1);B(m,0x800D80DE,1);B(m,0x800D80F0,0);B(m,0x800D80DC,0);v=S(m,0x800D81E1);B(m,0x800D80E0,4);
        if(v==1){B(m,0x800D81DF,9);if(S(m,0x800D81E2)==1)B(m,0x800D80F5,2);}else B(m,0x800D81DF,0);
        R(m,cb,0x8009C4BC,S(m,0x800D80E0));R(m,cb,0x8009C4C0,0);R(m,cb,0x8009C4B8,9);break;
    case 24:
        rrj_write32(m,0x800D80D8,4);B(m,0x8009C5F2,0);B(m,0x800D80EE,1);H(m,0x8009C5D4,24);
        /* fall through */
    case 38:
        rrj_write32(m,0x800D80D8,4);B(m,0x8009C5F2,0);B(m,0x800D80DE,1);B(m,0x800D80E0,56);B(m,0x800D81DF,0);B(m,0x800D80EB,0);B(m,0x800D80DC,0);B(m,0x8009C5F3,0);B(m,0x800D80E3,1);B(m,0x800D80E2,1);B(m,0x800D80E1,1);
        R(m,cb,0x8009C4D0,56);R(m,cb,0x8009C4CC,S(m,0x800D81DF));R(m,cb,0x8009C4E4,S(m,0x800D80E3));R(m,cb,0x8009C4E0,S(m,0x800D80E2));R(m,cb,0x8009C4DC,S(m,0x800D80E1));R(m,cb,0x8009C4C8,S(m,0x800D80DC));R(m,cb,0x8009C4D4,*(uint8_t *)rrj_at(m,0x800D80EA,1));break;
    case 27:
        rrj_write32(m,0x800D80D8,1);B(m,0x800D80F3,1);B(m,0x800D80DE,1);B(m,0x800D80F0,0);B(m,0x800D80DC,0);B(m,0x800D81E1,2);B(m,0x800D80E0,38);B(m,0x800D81DF,18);(void)select_item(m,cb,id);return C(m,cb,0x8002D298,0,0,0);
    case 30:case 32:
        rrj_write32(m,0x800D80D8,id==30?16:17);B(m,0x800D80DE,2);B(m,0x800D80E0,id==30?1:38);B(m,0x800D80F0,0);B(m,0x800D81DF,id==30?0:18);B(m,0x800D8203,9);
        R(m,cb,id==30?0x8009C4EC:0x8009C4F8,id==30?1:38);R(m,cb,id==30?0x8009C4F0:0x8009C4FC,S(m,0x800D81DF));
        p=id==30?0x8009C4F4:(rrj_s32(S(m,0x800D81DF))<18?0x8009C500:0x8009C504);R(m,cb,p,S(m,0x800D8203));R(m,cb,0x8009C4C8,S(m,0x800D80DC));break;
    case 34:
        rrj_write32(m,0x800D80D8,24);B(m,0x800D80E0,1);B(m,0x800D80F0,0);B(m,0x800D80DE,2);B(m,0x800D81DF,7);B(m,0x800D8203,7);
        R(m,cb,0x8009C4EC,1);R(m,cb,0x8009C510,S(m,0x800D81DF));R(m,cb,0x8009C514,S(m,0x800D8203));R(m,cb,0x8009C4C8,S(m,0x800D80DC));result=select_item(m,cb,id);B(m,0x800D81ED,2);B(m,0x800D8211,2);return result;
    case 36:
        rrj_write32(m,0x800D80D8,8);B(m,0x800D80F0,0);B(m,0x800D80F3,1);B(m,0x800D80DE,1);B(m,0x800D80DC,0);B(m,0x800D80E0,4);B(m,0x800D81DF,7);B(m,0x800D8203,7);
        R(m,cb,0x8009C508,4);R(m,cb,0x8009C50C,S(m,0x800D81DF));result=select_item(m,cb,id);B(m,0x800D81ED,1);return result;
    default:return id-7<32?0x80068e68:0x80060000;
    }
    return select_item(m,cb,id);
}
