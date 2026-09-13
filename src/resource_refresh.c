/* Resource refresh pass, derived from IDA and F8006738C MIPS. */
#include "resource_refresh.h"
#include "resource_groups.h"
#include "resource_next.h"
#include "resource_apply.h"
#include "resource_value.h"
#include "menu_first.h"
static uint32_t b(RRJMemory *m,uint32_t a){return *(uint8_t *)rrj_at(m,a,1);}
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t sb(uint32_t v){return v<128?v:v|0xffffff00;}
static uint32_t sh(uint32_t v){return v<32768?v:v|0xffff0000;}
static void put(RRJMemory *m,uint32_t a,uint32_t v){*(uint8_t *)rrj_at(m,a,1)=(uint8_t)v;}
static uint32_t unavailable(RRJMemory *m,uint32_t root)
{
    uint32_t record=rrj_read32(m,root+4)+12*sb(b(m,root+3)),kind,bit,index,flags;
    if(!(b(m,record+3)&b(m,0x8009C5E4))) return 1;
    kind=b(m,record);
    if(kind==4 || kind==8){
        bit=b(m,record+1);index=sb(bit);index=(index>>3)|(index&0x80000000?0xe0000000:0);
        if((b(m,0x800D80D8+(kind==4?240:252)+index)>>(bit&7))&1) return 1;
    }
    flags=b(m,record+2);
    if((flags&64) && !rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) return 1;
    if((flags&32) && rrj_read32(m,0x800D7138+192*sb(b(m,0x8009C5E6)))) return 1;
    return (flags&128) && (b(m,0x800D70E1)>>4)!=8;
}
static void refresh(RRJMemory *m,uint32_t slot,uint32_t apply,RRJSDKCall effect)
{
    uint32_t root=rrj_read32(m,slot);
    if(!root || !rrj_read32(m,root+4)) return;
    if(unavailable(m,root)) put(m,root+3,sub_F_80062D6C(m,rrj_read32(m,slot)));
    if(apply) sub_F_8006310C(m,root,effect);
}
static uint32_t menu_ineligible(RRJMemory *m,uint32_t entry)
{
    uint32_t type=h(m,entry+8),flags;
    if(type!=12 && type!=13 && type!=17) return 1;
    flags=rrj_read32(m,entry);
    if((flags&0x40000000) && (b(m,0x800D70E1)>>4)!=8) return 1;
    if((flags&0x02000000) && rrj_read32(m,0x800D742C+24*sb(b(m,0x8009C5E6)))!=1) return 1;
    if((flags&0x10000000) && !b(m,0x800D80F3)) return 1;
    if((flags&0x08000000) && rrj_read32(m,0x800D80D8)!=4) return 1;
    return (flags&0x04000000) && rrj_read32(m,0x800D80D8)==4;
}
void sub_F_8006738C(RRJMemory *m,uint32_t context,RRJSDKCall effect)
{
    uint32_t mode=b(m,0x800D80DC),menu,entry,i,root,index;
    (void)context;
    if(!(rrj_read32(m,0x800D80D8)&32) && mode!=0 && mode!=2 && mode!=4) put(m,0x800D80DC,0);
    (void)sub_F_80064254(m,sb(b(m,0x800D80DC)));(void)sub_F_80068448(m);
    for(i=0;i<3;++i) refresh(m,0x8009C668+4*i,1,effect);
    if(rrj_read32(m,0x800D80D8)==1) refresh(m,0x8009C4E8,1,effect);
    refresh(m,0x8009C664,0,effect);
    if((b(m,0x800D70E1)>>4)!=8){
        if(rrj_read32(m,0x800D80D8)==8) put(m,0x800D81ED,1);
        else {
            if(b(m,0x800D81ED)==1){put(m,0x800D81ED,2);(void)sub_F_800630C0(m,rrj_read32(m,0x8009C518),2);}
            if(b(m,0x800D8211)==1){put(m,0x800D8211,2);(void)sub_F_800630C0(m,rrj_read32(m,0x8009C51C),2);}
        }
    }
    menu=rrj_read32(m,rrj_read32(m,0x8009C68C)+4*sh(h(m,0x8009C5D0)));
    rrj_write32(m,0x8009C5C8,menu);entry=rrj_read32(m,menu+16)+120*sh(h(m,menu+4));
    if(menu_ineligible(m,entry)){
        uint32_t first=sub_F_80080ACC(m,rrj_read32(m,0x8009C5C8));
        rrj_put16(rrj_at(m,rrj_read32(m,0x8009C5C8)+4,2),(uint16_t)first);
    }
    for(i=0;i<4;++i){
        uint32_t address=0x800D81D8+36*i+8,value=b(m,address);
        if(rrj_read32(m,0x800D7138+192*i)){if(value-3>=2) put(m,address,3);}
        else if(value-3<2) put(m,address,0);
    }
    root=rrj_read32(m,0x8009C534);
    if(root && rrj_read32(m,root+4) && unavailable(m,root)){
        put(m,root+3,sub_F_80062D6C(m,rrj_read32(m,0x8009C534)));
        (void)sub_F_800630C0(m,root,sb(b(m,0x8009C5E6)));
    }
    root=rrj_read32(m,0x8009C538);
    if(root && rrj_read32(m,root+4) && unavailable(m,root)){
        put(m,root+3,sub_F_80062D6C(m,rrj_read32(m,0x8009C538)));
        index=sb(b(m,0x8009C5E6));(void)sub_F_800630C0(m,root,sb(b(m,0x800D81D8+36*index+8)));
    }
}
