/* IDA draft refined against MIPS; table evidence in resource-select-table.json. */
#include "resource_select.h"
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
uint32_t sub_F_800649BC(RRJMemory *m,uint32_t resource)
{
    int32_t count=rrj_s32(sb(b(m,resource)));
    uint32_t found=0,i;
    for(i=0;rrj_s32(i)<count;++i){
        uint32_t record=rrj_read32(m,resource+4)+12*i,kind,flags,bit,index;
        if(!(b(m,record+3)&b(m,0x8009C5E4))) continue;
        kind=b(m,record);
        if(kind==4 || kind==8){
            bit=b(m,record+1);index=sb(bit);
            index=(index>>3)|(index&0x80000000?0xe0000000:0);
            if((b(m,0x800D80D8+(kind==4?240:252)+index)>>(bit&7))&1) continue;
        }
        flags=b(m,record+2);
        if((flags&64) && !rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) continue;
        if((flags&32) && rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) continue;
        if((flags&128) && (b(m,0x800D70E1)>>4)!=8) continue;
        ++found;
    }
    return found;
}
uint32_t sub_F_800680E8(RRJMemory *m,uint32_t menu,uint32_t entry)
{
    static const uint32_t globals[72]={0x8009C4B0,0,0,0x8009C4BC,0,0,0,0x8009C4C8,0x8009C4CC,0x8009C4D0,0,0,0x8009C4DC,0x8009C4E0,0x8009C4E4,0x8009C4D8,0,0x8009C4E8,0,0,0,0,0,0,0x8009C4C8,0x8009C4EC,0x8009C4F0,0,0x8009C4F4,0,0x8009C4C8,0x8009C4F8,0x8009C4FC,0,0,0,0x8009C508,0x8009C50C,0,0x8009C4C8,0x8009C4EC,0x8009C510,0x8009C518,0,0x8009C514,0x8009C51C,0,0x8009C4C8,0x8009C4D0,0x8009C4D4,0,0,0,0x8009C4CC,0,0,0,0,0,0,0,0,0,0x8009C524,0x8009C528,0x8009C52C,0x8009C520,0,0x8009C534,0x8009C538,0x8009C53C,0x8009C540};
    uint32_t result=0x8009C5D0,address=0,selector,resource,table;
    (void)menu;
    rrj_write32(m,0x8009C664,0);*(uint8_t *)rrj_at(m,0x8009C5E7,1)=0;
    if(!entry) return result;
    if(h(m,entry+8)==13){
        selector=h(m,entry+18);
        if(selector>=9 && selector<=80) address=globals[selector-9];
        if(selector==10 || selector==13){
            uint32_t mode=b(m,0x800D81E1);
            if(mode<=1) address=(selector==10?0x8009C4B4:0x8009C4C0)+4*mode;
        }
        if(selector==43) address=rrj_s32(sb(b(m,0x800D81DF)))<18?0x8009C500:0x8009C504;
        if(address) rrj_write32(m,0x8009C664,rrj_read32(m,address));
    }
    result=rrj_read32(m,0x8009C678);resource=rrj_read32(m,0x8009C664);
    rrj_write32(m,0x8009C674,result);
    if(!resource) return result;
    table=rrj_read32(m,resource+4);if(!table) return result;
    rrj_write32(m,0x8009C678,table+12*sb(b(m,resource+3)));
    result=sub_F_800649BC(m,resource);
    *(uint8_t *)rrj_at(m,0x8009C5E7,1)=(uint8_t)result;
    return result;
}
void rrj_select_menu_image(RRJMemory *m,uint32_t menu,uint32_t entry)
{(void)sub_F_800680E8(m,menu,entry);}
