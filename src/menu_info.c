#include "wip.h"
/* Information string descriptor selection, checked against F MIPS. */
#include "menu_info.h"
#include "entry_kind.h"
#include "resource_record.h"
#include <stdlib.h>
static uint32_t b(RRJMemory *m,uint32_t a) {return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t h(RRJMemory *m,uint32_t a) {return rrj_u16(rrj_at(m,a,2));}
static uint32_t sx(uint32_t v) {return v<128?v:v|0xffffff00;}
static uint32_t result(RRJMemory *m,uint32_t a,uint32_t value)
{rrj_put16(rrj_at(m,a,2),value);return a;}
uint32_t sub_F_80063C7C(RRJMemory *m,uint32_t selector,RRJModeLabel mode_label)
{
    uint32_t entry,value,mode;
    switch(selector&65535) {
    case 14:
        if(h(m,0x8009C5D8)!=h(m,0x8009C5D0)) return 0;
        entry=rrj_read32(m,0x8009C654);
        if(!entry || h(m,entry+8)!=12) return 0;
        value=0;
        if(sub_F_80072100(m,entry)==1) {
            mode=b(m,0x800D80DC);
            if(mode==1) value=1545;
            else if(mode==3) value=1546;
            else if(mode==5) value=1547;
        }
        if(!value) value=h(m,rrj_read32(m,0x8009C654)+20)+1;
        (void)result(m,0x8009C4A8,value);
        *(uint8_t *)rrj_at(m,0x8009C4AA,1)=3;
        rrj_write32(m,0x8009C4AC,0xA08C8C);
        return 0x8009C4A8;
    case 32:
        value=b(m,0x800D80EF);
        if(value!=0 && value!=2) return 0;
        return result(m,0x80081120,value==2?201:200);
    case 33:return result(m,0x80081120,b(m,0x800D80F1)?6:7);
    case 34:
        if(!mode_label){RRJ_WIP(m,0,"unknown","mode_label","return_null_descriptor",&selector,1);return 0;}
        value=mode_label(m,sx(b(m,0x800D80DC)),rrj_read32(m,0x800D80D8),0);
        return result(m,0x800810F8,value);
    case 35:return result(m,0x80081100,b(m,0x800D80E1)?6:7);
    case 36:return result(m,0x80081108,b(m,0x800D80E2)?6:7);
    case 37:return result(m,0x80081110,b(m,0x800D80E3)?6:7);
    case 38:
        value=b(m,0x800D80EA);
        return result(m,0x80081118,value>=2 && value<=6?value+7:8);
    case 39:return result(m,0x80081120,b(m,0x800D8198+b(m,0x8009C5DE))?6:7);
    case 41:case 42:
        value=b(m,(selector&65535)==41?0x800D81ED:0x800D8211);
        return result(m,0x80081120,value==1?196:value==2?197:198);
    case 43:return result(m,0x80081108,b(m,0x800D80EC)?184:183);
    case 44:return result(m,0x80081120,b(m,0x800D81EF+36*sx(b(m,0x8009C5E6)))?6:7);
    case 56:return result(m,0x80081108,b(m,0x8009C5E9)?4:5);
    default:return sub_F_80064154(m,selector);
    }
}
