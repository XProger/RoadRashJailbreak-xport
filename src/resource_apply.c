/* Resource -> game settings, F8006310C. Void ABI: v0 is scratch. */
#include "resource_apply.h"
#include "resource_value.h"
#include <stdlib.h>
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
static void put(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static uint32_t value(RRJMemory *m,uint32_t r){return b(m,rrj_read32(m,r+4)+12*sb(b(m,r+3))+1);}
static void call(RRJMemory *m,RRJSDKCall effect,uint32_t fn,uint32_t a,uint32_t c){if(!effect) abort();effect(m,fn,a,c);}
void sub_F_8006310C(RRJMemory *m,uint32_t r,RRJSDKCall effect)
{
    uint32_t kind,dest=0,v,index,other,changed;
    if(!r || !rrj_read32(m,r+4)) return;
    kind=b(m,r+2);
    switch(kind){
    case 0:
        v=value(m,r);index=b(m,0x8009C5E3);put(m,0x800D81E1,v);
        if(index&192){other=rrj_read32(m,v==1?0x8009C4B8:0x8009C4B4);put(m,0x800D81E2,0);(void)sub_F_800630C0(m,other,0);}return;
    case 1:case 2:dest=0x800D81E2;break;
    case 3:case 8:case 15:case 18:case 22:dest=0x800D80E0;break;
    case 4:case 5:case 16:case 19:case 23:case 24:dest=0x800D81DF;break;
    case 6:dest=0x800D80DC;break;
    case 7:case 62:dest=0x800D81D8+36*b(m,0x800D80EB)+7;break;
    case 9:dest=0x800D80EA;break;case 10:dest=0x8009C5EF;break;
    case 11:dest=0x800D80E1;break;case 12:dest=0x800D80E2;break;case 13:dest=0x800D80E3;break;
    case 14:put(m,0x800D80DF,value(m,r));put(m,0x800D80E0,value(m,r)+38);return;
    case 17:case 20:case 21:case 25:dest=0x800D8203;break;
    case 26:dest=0x800D81ED;break;case 27:dest=0x800D8211;break;
    case 28:
        v=value(m,r);changed=b(m,0x800D80EC)!=v;put(m,0x800D80EC,v);
        call(m,effect,0x8001EFDC,v==0,0);
        if(changed) call(m,effect,0x8007ED34,sb(b(m,0x800D80EC)),0);return;
    case 29:dest=0x800D80EF;break;case 30:dest=0x800D80F1;break;case 31:dest=0x8009C5DE;break;
    case 32:
        index=b(m,0x8009C5DE);put(m,0x800D8198+index,value(m,r));index=b(m,0x8009C5DE);
        call(m,effect,0x8002490C,index,sb(b(m,0x800D8198+index)));return;
    case 33:
        other=rrj_read32(m,0x8009C538);v=value(m,r);put(m,0x8009C5E6,v);
        (void)sub_F_800630C0(m,other,sb(b(m,0x800D81D8+36*sb(v)+8)));
        index=sb(b(m,0x8009C5E6));other=rrj_read32(m,0x8009C53C);
        (void)sub_F_800630C0(m,other,sb(b(m,0x800D81D8+36*index+23)));return;
    case 34:dest=0x800D81D8+36*sb(b(m,0x8009C5E6))+8;break;
    case 35:dest=0x800D81D8+36*sb(b(m,0x8009C5E6))+23;break;
    case 36:dest=0x8009C5E9;break;
    default:return;
    }
    put(m,dest,value(m,r));
}
