/* Resident8002305C: refresh pause conditions; callees require native binding. */
#include "race_pause.h"
#include "menu_latch.h"
#include "fixed_math.h"
#include <stdlib.h>
uint32_t sub_80022C64(RRJMemory *m)
{
    uint32_t elapsed;
    if(!rrj_read32(m,0x8005AE20))return 0;
    elapsed=rrj_read32(m,rrj_read32(m,0x8005B2F8)+12)-rrj_read32(m,0x8005AE1C);
    if(rrj_s32(elapsed)<0)elapsed=0u-elapsed;
    if(rrj_s32(elapsed)<241)return 0;
    return rrj_s32(elapsed)<3001?1u:2u;
}
uint32_t sub_80023870(RRJMemory *m,RRJRacePlayerCheck check)
{
    uint32_t result;
    if(!check)abort();
    result=check(m,rrj_read32(m,0x8005B38C))>>31;
    result|=check(m,0x800CD898)>>31;
    if(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)==2){
        result|=check(m,rrj_read32(m,0x8005B21C))>>31;
        result|=check(m,0x800CDD04)>>31;
    }
    return result;
}
uint32_t sub_8002305C(RRJMemory *m,RRJRacePauseCall call)
{
    uint32_t reason,result;
    if(!call)abort();
    (void)call(m,0x800237B8);(void)call(m,0x80030608);
    if(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)==1)(void)call(m,0x800247E8);
    reason=call(m,0x80022C64);
    if(!reason && *(uint8_t *)rrj_at(m,rrj_read32(m,0x8005B2F8),1)!=4)return 4;
    result=2u*(call(m,0x80023870)!=0)|(reason==2);
    rrj_put16(rrj_at(m,rrj_read32(m,0x8005B2F8)+40,2),result);
    return result;
}

/* 80030500: first matching object range, lower inclusive / upper exclusive. */
uint32_t sub_80030500(RRJMemory *m,uint32_t tag,uint32_t position)
{
    uint32_t base=rrj_read32(m,0x8005ACBC),count=rrj_read32(m,base+0xA58);
    uint32_t i,entry=base+44;
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,entry+=36){
        uint32_t object,kind,ranges,j;
        if((rrj_read32(m,entry)&0x12)!=0x12)continue;
        object=rrj_read32(m,entry+20);kind=rrj_read32(m,object)>>28;
        if(kind!=0 && kind!=8)continue;
        ranges=rrj_read32(m,entry+24);
        for(j=0;j<4;++j,ranges+=12){
            uint32_t candidate=rrj_read32(m,ranges);
            if(candidate==0xFFFFFFFF)break;
            if(tag==candidate && rrj_s32(position)>=rrj_s32(rrj_read32(m,ranges+4)) &&
                rrj_s32(position)<rrj_s32(rrj_read32(m,ranges+8)))
                return rrj_read32(m,rrj_read32(m,entry+20))&0x0FFFFFFF;
        }
    }
    return 0xFFFFFFFF;
}

uint32_t sub_800245DC(RRJMemory *m,uint32_t index)
{
    return rrj_read32(m,rrj_read32(m,0x8005AE64)+24)+(index<<4);
}
uint32_t sub_800245F4(RRJMemory *m,uint32_t index)
{
    return rrj_read32(m,rrj_read32(m,0x8005AE64)+20)+((index+(index<<2))<<3);
}
static uint32_t pause_asr(uint32_t value,uint32_t shift)
{
    if(!shift)return value;
    return (value>>shift)|((value&0x80000000)?(0xFFFFFFFFu<<(32-shift)):0);
}
uint32_t sub_80030410(RRJMemory *m,uint32_t input,uint32_t step)
{
    uint32_t tag=rrj_read32(m,input+188),position;
    uint32_t reference=rrj_u16(rrj_at(m,0x800CDA00,2));
    if(!(tag>>16)){
        position=pause_asr(rrj_read32(m,input+196),10);
        step=pause_asr(step<<16,16);
        if(step && (tag&0xFFFF)==reference){
            uint32_t target=pause_asr(rrj_read32(m,0x800CDA08),10),adjusted;
            if(rrj_s32(position)<rrj_s32(target)){
                adjusted=position+step;
                if(rrj_s32(target)<rrj_s32(adjusted))adjusted=target;
            }else{
                adjusted=position-step;
                if(rrj_s32(adjusted)<rrj_s32(target))adjusted=target;
            }
            position=adjusted;
        }
        tag=rrj_u16(rrj_at(m,input+188,2));
    }else{
        uint32_t first=sub_800245F4(m,tag&0xFFFF),second;
        tag=rrj_read32(m,first+8);
        second=sub_800245DC(m,tag);
        position=rrj_s32(rrj_read32(m,first+12))>0?1u:rrj_read32(m,second+4)-1;
    }
    return sub_80030500(m,tag,position);
}
uint32_t sub_8008B99C(RRJMemory *m,uint32_t player)
{
    uint32_t type=rrj_u16(rrj_at(m,player+172,2)),value=0,result;
    if((type>>5)!=6 && ((type>>5)!=4 || (type&31)<30)){
        uint32_t shift=rrj_u16(rrj_at(m,rrj_read32(m,player)+14,2))>>12;
        value=pause_asr(rrj_read32(m,player+40),shift);
    }
    result=sub_80030410(m,player+172,pause_asr(value<<16,16));
    rrj_write32(m,player+176,result);
    return result;
}

uint32_t sub_8002379C(RRJMemory *m,uint32_t index)
{
    rrj_write32(m,0x8005AE34,0x80053478+(index<<7));
    return 0x80053478;
}
uint32_t sub_80023860(uint32_t value){return value;}
uint32_t sub_80023D7C(RRJMemory *m,RRJRacePlayerCall call)
{
    if(!call)abort();
    return call(m,0x80030E58,rrj_read32(m,rrj_read32(m,0x8005AE34)),0,0);
}
uint32_t sub_80023C4C(RRJMemory *m,RRJRacePlayerCall call)
{
    uint32_t result,record;
    if(!call)abort();
    result=call(m,0x800312D0,rrj_read32(m,rrj_read32(m,0x8005AE34)),0,0);
    record=rrj_read32(m,0x8005AE34);
    if(result && !(rrj_read32(m,record+28)&2)){
        rrj_write32(m,record+108,1);return 1;
    }
    rrj_write32(m,record+108,0);return record;
}
uint32_t sub_800237B8(RRJMemory *m,RRJRacePlayerCall call)
{
    uint32_t index=0;
    if(!call)abort();
    while(index<rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)){
        uint32_t record;
        (void)sub_8002379C(m,index);
        (void)call(m,0x80023A14,0,0,0);
        record=rrj_read32(m,0x8005AE34);
        if(rrj_read32(m,record+44))
            (void)call(m,0x80031E1C,rrj_read32(m,record+72),0,index);
        ++index;
        (void)call(m,0x80023FBC,0,0,0);
        (void)sub_80023D7C(m,call);
        (void)sub_80023C4C(m,call);
    }
    return call(m,0x80023CAC,0,0,0);
}

/* 80030608: queued object classification, pending dispatch and record sweep. */
uint32_t sub_80030608(RRJMemory *m,RRJRacePlayerCall call)
{
    uint32_t pending[64],count=0,object,i,record;
    if(!call)abort();
    while((object=call(m,0x80031D70,0,0,0))!=0){
        uint32_t classification=call(m,0x80031BF8,object,*(uint8_t *)rrj_at(m,object+34,1),0);
        if(rrj_s32(classification)<0){
            /* Original has 64 stack slots; overflow corrupts its saved registers. */
            if(count>=64)abort();
            pending[count++]=object;
        }else{
            uint32_t value=rrj_read32(m,rrj_read32(m,object+20)+28)>>1;
            uint32_t saved=rrj_read32(m,object+32),target;
            (void)call(m,0x800313EC,object,0,0);
            record=rrj_read32(m,0x8005ACBC)+44+36*classification;
            value=sub_80023860(value);
            target=rrj_read32(m,record+20);
            rrj_write32(m,target+28,(rrj_read32(m,target+28)&1)|(value<<1));
            rrj_write32(m,record+32,saved);
        }
    }
    for(i=0;i<count;++i){
        uint32_t kind;
        object=pending[i];
        if(rrj_read32(m,object)&0x20)continue;
        kind=rrj_read32(m,rrj_read32(m,object+20))>>28;
        if(kind==1 || kind==2)
            (void)call(m,kind==1?0x800319F8:0x80031B4C,object,*(uint8_t *)rrj_at(m,object+34,1),0);
        else rrj_write32(m,object,rrj_read32(m,object)|0x20);
    }
    record=rrj_read32(m,0x8005ACBC)+44;
    for(i=0;rrj_s32(i)<rrj_s32(rrj_read32(m,rrj_read32(m,0x8005ACBC)+0xA58));++i,record+=36){
        if((rrj_read32(m,record)&0x22)==0x20)
            (void)call(m,0x80031604,record,*(uint8_t *)rrj_at(m,record+34,1),0);
    }
    (void)call(m,0x800320CC,5,0,0);
    if(!rrj_read32(m,0x8005AED8))return 0;
    rrj_write32(m,0x8005AED8,0);
    return call(m,0x800320AC,0,0,0);
}

uint32_t sub_80031D70(RRJMemory *m,RRJRacePlayerCall call)
{
    uint32_t base,object=0;
    if(!call)abort();
    (void)call(m,0x80043DA4,0,0,0);
    base=rrj_read32(m,0x8005ACBC);
    if(rrj_read32(m,base+0x934)){
        uint32_t index,next,rounded,count;
        index=rrj_read32(m,base+0x930);
        object=rrj_read32(m,base+0x938+(index<<2));
        rrj_write32(m,object,rrj_read32(m,object)&0xFFFFFF7F);
        index=rrj_read32(m,base+0x930);next=index+1;
        rounded=rrj_s32(next)<0?index+64:next;
        rounded=pause_asr(rounded,6)<<6;
        count=rrj_read32(m,base+0x934);
        rrj_write32(m,base+0x930,next-rounded);
        rrj_write32(m,base+0x934,count-1);
    }
    (void)call(m,0x80043DB4,0,0,0);
    return object;
}
uint32_t sub_80031BF8(RRJMemory *m,uint32_t object,uint32_t group)
{
    uint32_t base=rrj_read32(m,0x8005ACBC),descriptor=rrj_read32(m,object+20);
    uint32_t word=rrj_read32(m,descriptor),index=rrj_read32(m,base+(group<<3)+0xA40);
    uint32_t record=base+44+36*index,bit=rrj_read32(m,descriptor+28)&1;
    uint32_t last=rrj_read32(m,base+(group<<3)+0xA44);
    while(index<=last){
        if(record!=object && (rrj_read32(m,record)&0x91)==0x11){
            uint32_t other=rrj_read32(m,record+20);
            if(rrj_read32(m,other)==word && ((word>>28)!=1 || (rrj_read32(m,other+28)&1)==bit))return index;
        }
        ++index;record+=36;
    }
    return 0xFFFFFFFF;
}
uint32_t sub_800320CC(RRJMemory *m,uint32_t mode)
{
    uint32_t offset;
    switch(mode){
    case 1:offset=16;break;case 2:offset=8;break;case 3:offset=20;break;
    case 4:offset=4;break;case 7:offset=28;break;case 8:offset=24;break;
    default:return 0xFFFFFFFF;
    }
    return rrj_read32(m,rrj_read32(m,0x8005ACBC)+offset);
}
uint32_t sub_800320AC(RRJMemory *m){return sub_800320CC(m,6);}

uint32_t sub_80031B98(RRJMemory *m,uint32_t left,uint32_t right)
{
    uint32_t i;
    for(i=0;i<32;++i){
        uint8_t a=*(uint8_t *)rrj_at(m,left+i,1),b=*(uint8_t *)rrj_at(m,right+i,1);
        *(uint8_t *)rrj_at(m,left+i,1)=b;
        *(uint8_t *)rrj_at(m,right+i,1)=a;
    }
    return 0;
}
uint32_t sub_80031B4C(RRJMemory *m,uint32_t object)
{
    uint32_t base=rrj_read32(m,object+16),target=base+16352,result;
    (void)sub_80031B98(m,base,target);
    result=rrj_read32(m,object);
    rrj_write32(m,object+20,target);
    result|=0x20;rrj_write32(m,object,result);
    return result;
}
uint32_t sub_800319F8(RRJMemory *m,uint32_t object,uint32_t group)
{
    uint32_t base,index,last,record,descriptor,word,bit,found=0,left,right,data,result;
    result=rrj_read32(m,object)&0x20;if(result)return result;
    base=rrj_read32(m,0x8005ACBC);
    index=rrj_read32(m,base+(group<<3)+0xA40);record=base+44+36*index;
    descriptor=rrj_read32(m,object+20);last=rrj_read32(m,base+(group<<3)+0xA44);
    bit=rrj_read32(m,descriptor+28)&1;word=rrj_read32(m,descriptor);
    while(index<=last){
        if(record!=object && (rrj_read32(m,record)&0x91)==0x11 &&
            rrj_read32(m,rrj_read32(m,record+20))==word){found=record;break;}
        ++index;record+=36;
    }
    if(!found)return 1;
    left=bit?found:object;right=bit?object:found;
    (void)sub_80031B98(m,rrj_read32(m,left+16),rrj_read32(m,right+16)+16224);
    data=rrj_read32(m,right+16);(void)sub_80031B98(m,data,data+16352);
    rrj_write32(m,left+20,rrj_read32(m,right+16)+16224);
    rrj_write32(m,right+20,rrj_read32(m,right+16)+16352);
    rrj_write32(m,left,rrj_read32(m,left)|0x20);
    result=rrj_read32(m,right)|0x20;rrj_write32(m,right,result);
    return result;
}

uint32_t sub_80032190(RRJMemory *m,uint32_t from,uint32_t to)
{
    static const uint32_t offsets[4]={16,0,20,12};
    uint32_t address,result;
    if(from<4){address=rrj_read32(m,0x8005ACBC)+offsets[from];rrj_write32(m,address,rrj_read32(m,address)-1);}
    if(to>=4)return 3;
    address=rrj_read32(m,0x8005ACBC)+offsets[to];result=rrj_read32(m,address)+1;
    rrj_write32(m,address,result);return result;
}
uint32_t sub_800322D0(RRJMemory *m,uint32_t from,uint32_t to,RRJRacePlayerCall call)
{
    uint32_t result;
    if(!call)abort();
    if(!rrj_read32(m,0x800541D0))(void)call(m,0x80043DA4,0,0,0);
    (void)sub_80032190(m,from,to);
    result=rrj_read32(m,0x800541D0);
    if(!result)return call(m,0x80043DB4,0,0,0);
    return result;
}
uint32_t sub_800313EC(RRJMemory *m,uint32_t object,RRJRacePlayerCall call)
{
    uint32_t flags=rrj_read32(m,object),retained=(flags>>12)&15,original_active=flags&1;
    uint32_t result,address,offset;
    if(original_active){
        rrj_write32(m,object,flags&0xFFFFFFDC);
        (void)sub_800322D0(m,1,0,call);
    }else (void)sub_800322D0(m,2,0,call);
    flags=rrj_read32(m,object);result=flags&8;
    if(flags&16){
        offset=original_active?4u:24u;
        address=rrj_read32(m,0x8005ACBC)+offset;
        result=rrj_read32(m,address)-1;rrj_write32(m,address,result);
    }else if(result){
        offset=original_active?8u:28u;
        address=rrj_read32(m,0x8005ACBC)+offset;
        result=rrj_read32(m,address)-1;rrj_write32(m,address,result);
    }
    if(retained){
        rrj_write32(m,object,(rrj_read32(m,object)-4096)|4);
        return sub_800322D0(m,0,2,call);
    }
    rrj_write32(m,object+8,0);rrj_write32(m,object,0);
    rrj_write32(m,object+16,0);rrj_write32(m,object+20,0);
    return result;
}

static uint32_t dispatch_object_call(RRJMemory *m,RRJObjectDispatchCall call,uint32_t fn,
    uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f)
{
    uint32_t args[6]={a,b,c,d,e,f};return call(m,fn,args);
}
uint32_t sub_80031604(RRJMemory *m,uint32_t object,uint32_t group,RRJObjectDispatchCall call,RRJRacePlayerCall critical)
{
    uint32_t flags=rrj_read32(m,object),buffer=rrj_read32(m,object+16);
    uint32_t id=0,kind=0,status,descriptor,extra,value,result,target;
    if(!call || !critical)abort();
    if(flags&8){
        (void)dispatch_object_call(m,call,0x800248E4,rrj_read32(m,object+4),buffer,0,0,0,0);status=1;
    }else{
        descriptor=rrj_read32(m,object+20);extra=rrj_read32(m,object+24);
        value=sub_80023860(rrj_read32(m,descriptor+28)>>1)<<1;
        id=rrj_read32(m,descriptor);kind=id>>28;id&=0x0FFFFFFF;
        value|=rrj_read32(m,descriptor+28)&1;rrj_write32(m,descriptor+28,value);
        switch(kind){
        case 0:case 8:
            (void)dispatch_object_call(m,call,0x80031E1C,rrj_read32(m,rrj_read32(m,0x8005AE34)+72),object,group,0,0,0);
            status=dispatch_object_call(m,call,0x80032A20,buffer,id,kind,extra,object+28,group);break;
        case 9:status=dispatch_object_call(m,call,0x80032A20,buffer,id,kind,0,0,group);break;
        case 1:
            status=(rrj_read32(m,object)&0x20)?dispatch_object_call(m,call,0x80032CC8,buffer,id,value&1,value,descriptor+4,group):0;break;
        case 2:
            status=(rrj_read32(m,object)&0x20)?dispatch_object_call(m,call,0x80032C6C,buffer,id,value,descriptor+4,group,0):0;break;
        case 3:
            status=dispatch_object_call(m,call,0x8003CEC4,buffer,id&0xFFFF,group,value,0,0);
            if(status==3)status=1;break;
        case 4:
            status=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)==1?
                dispatch_object_call(m,call,0x800136BC,buffer,descriptor+4,rrj_read32(m,descriptor),0,0,0):2;break;
        case 10:status=dispatch_object_call(m,call,0x8001A0C0,buffer,id,rrj_read32(m,object+4),0,0,0);break;
        default:status=2;break;
        }
    }
    if(status!=1){
        if(!status)return 2;
        if(status==2 || status==3)return sub_800313EC(m,object,critical);
        return 3;
    }
    flags=rrj_read32(m,object);result=flags|2;
    if(!flags)return result;
    rrj_write32(m,object,result);result&=16;if(!result)return result;
    (void)dispatch_object_call(m,call,0x80023960,rrj_read32(m,object+20),group,0,0,0,0);
    if(kind!=0 && kind!=8)return 8;
    target=rrj_read32(m,0x800D6184);result=rrj_read32(m,target+16);
    if(id!=result){target=rrj_read32(m,0x800D6188);result=rrj_read32(m,target+16);if(id!=result)return result;}
    rrj_write32(m,0x8005B310,target);return result;
}

uint32_t sub_80022218(RRJMemory *m,uint32_t id,uint32_t group)
{
    uint32_t index=24*group,end=index+24,record=0x800D9268+index*48;
    while(rrj_s32(index)<rrj_s32(end)){
        if(rrj_read32(m,record+8)==id)return index;
        ++index;record+=48;
    }
    return 0xFFFFFFFF;
}
uint32_t sub_8002227C(RRJMemory *m,uint32_t channel,uint32_t id,uint32_t pixels0,uint32_t pixels1,uint32_t group,RRJImageUpload upload)
{
    uint32_t count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48),kind=id>>15;
    uint32_t index=sub_80022218(m,id,group),slot,config,bits,parity,result;
    uint8_t rect[8];
    if(index==0xFFFFFFFF)index=sub_80022218(m,0xFFFFFFFF,group);
    slot=0x800D9268+48*index;rrj_write32(m,slot+8,id);
    if(kind>1)return 1;
    if(!upload)abort();
    config=0x800533B4+88*(count-1)+4*(group+(kind?2:0));
    bits=*(uint8_t *)rrj_at(m,config+2,1);
    rrj_put16(rect+4,64);rrj_put16(rect+6,128);
    if(!kind){
        rrj_put16(rect,((channel&15)+(bits&15))<<6);rrj_put16(rect+2,0);
        (void)upload(m,rect,pixels0);
        rrj_put16(rect+2,rrj_u16(rect+2)+128);
        result=upload(m,rect,pixels1);
    }else{
        parity=(channel&1)<<7;
        rrj_put16(rect,((bits&15)<<6)+((channel<<5)&0x3C0));
        rrj_put16(rect+2,*(uint8_t *)rrj_at(m,config+1,1)+((bits&16)<<4)+parity);
        (void)upload(m,rect,pixels0);
        result=0x800D76D0+32*group+8*channel;
        *(uint8_t *)rrj_at(m,result+6,1)=0;*(uint8_t *)rrj_at(m,result+7,1)=(uint8_t)parity;
    }
    rrj_write32(m,slot,channel);return result;
}
uint32_t sub_80031008(RRJMemory *m,uint32_t buffer,RRJRacePlayerCall critical)
{
    uint32_t base=rrj_read32(m,0x8005ACBC),count=rrj_read32(m,base+0xA58);
    uint32_t record=base+44,index;
    for(index=0;rrj_s32(index)<rrj_s32(count);++index,record+=36)
        if(rrj_read32(m,record+16)==buffer)break;
    /* The original cleans the following record when no match exists. */
    return sub_800313EC(m,record,critical);
}
uint32_t sub_800333F4(RRJMemory *m,uint32_t mode,uint32_t group,RRJObjectDispatchCall call)
{
    uint32_t index=12*group,end=index+12,record=0x800D87E8+1344*group;
    if(!call)abort();
    while(rrj_s32(index)<rrj_s32(end)){
        (void)dispatch_object_call(m,call,0x80033F14,record,mode,group,0,0,0);
        ++index;record+=112;
    }
    return 0;
}
uint32_t sub_80034DEC(RRJMemory *m,const uint32_t args[7],RRJObjectDispatchCall call)
{
    uint32_t id=args[0],buffer=args[1],mode=args[2],side=args[3],value=args[4],data=args[5],group=args[6];
    uint32_t index=sub_80022218(m,id,group),record,found;
    if(!call)abort();
    if(index==0xFFFFFFFF)index=sub_80022218(m,0xFFFFFFFF,group);
    else{
        record=0x800D9268+index*48;
        if(!mode && ((!side && rrj_read32(m,record+12)) || (side==1 && rrj_read32(m,record+16))))return 3;
        if(mode==1 && rrj_read32(m,record+12))return 3;
        if(rrj_read32(m,record)!=0xFFFFFFFF)return 3;
    }
    record=0x800D9268+index*48;
    rrj_write32(m,record+8,id);
    if(!mode){rrj_write32(m,record+4,0);rrj_write32(m,record+(side?16:12),buffer);}
    else{rrj_write32(m,record+12,buffer);rrj_write32(m,record+4,mode);}
    rrj_write32(m,record+20,value);
    (void)sub_8001E0B4(m,record+24,data,24);
    (void)dispatch_object_call(m,call,0x800324CC,record,mode,group,0,0,0);
    if(mode!=1 && (mode || !rrj_read32(m,record+12) || !rrj_read32(m,record+16)))return 1;
    found=dispatch_object_call(m,call,0x800335B4,mode,id,group,0,0,0);
    if(rrj_s32(found)<0){
        found=dispatch_object_call(m,call,0x800335B4,mode,0xFFFFFFFF,group,0,0,0);
        if(found!=0xFFFFFFFF)(void)dispatch_object_call(m,call,0x80033B50,found,index,mode,group,0,0);
    }
    return 1;
}
static uint32_t pack_object_id(uint32_t id){return ((id>>16)&0x8000)|((id>>13)&0x7C00)|(id&0x3FF);}
uint32_t sub_80032C6C(RRJMemory *m,const uint32_t input[6],RRJObjectDispatchCall call)
{
    uint32_t args[7]={pack_object_id(input[1])|0x8000,input[0],1,0,input[2],input[3],input[4]};
    return sub_80034DEC(m,args,call);
}
uint32_t sub_80032CC8(RRJMemory *m,const uint32_t input[6],RRJObjectDispatchCall call)
{
    uint32_t args[7]={pack_object_id(input[1]),input[0],0,input[2]!=0,input[3],input[4],input[5]};
    return sub_80034DEC(m,args,call);
}

uint32_t sub_800324CC(RRJMemory *m,uint32_t slot,uint32_t mode,uint32_t group)
{
    uint32_t index=12*group,end=index+12,record=0x800D87E8+index*112,j;
    if(rrj_s32(index)>=rrj_s32(end))return 0x800E0000;
    do{
        for(j=0;j<2;++j){
            if(rrj_read32(m,record+(mode?88:80)+4*j)==rrj_read32(m,slot+8))
                rrj_write32(m,record+(mode?96:104)+4*j,slot);
        }
        ++index;record+=112;
    }while(rrj_s32(index)<rrj_s32(end));
    return 0;
}
uint32_t sub_800335B4(RRJMemory *m,uint32_t mode,uint32_t id,uint32_t group)
{
    uint32_t count=rrj_read32(m,mode?0x8005AEE8:0x8005AEE4),index;
    uint32_t record=mode?0x800D76D0+32*group:0x800D7710+24*group;
    for(index=0;rrj_s32(index)<rrj_s32(count);++index,record+=8)
        if(rrj_read32(m,record)==id)return index;
    return 0xFFFFFFFF;
}
uint32_t sub_80033B50(RRJMemory *m,uint32_t channel,uint32_t index,uint32_t mode,uint32_t group,RRJObjectDispatchCall call)
{
    uint32_t slot=0x800D9268+48*index,record,target,id,value;
    if(!call)abort();
    if(!rrj_read32(m,slot+12))return 1;
    if(rrj_read32(m,slot+16) && !mode){
        (void)dispatch_object_call(m,call,0x8002227C,channel,rrj_read32(m,slot+8),rrj_read32(m,slot+12),rrj_read32(m,slot+16),group,0);
        (void)dispatch_object_call(m,call,0x80031008,rrj_read32(m,slot+12),0,0,0,0,0);
        (void)dispatch_object_call(m,call,0x80031008,rrj_read32(m,slot+16),0,0,0,0,0);
        record=0x800D7710+24*group+8*channel;
        rrj_write32(m,slot+12,0);rrj_write32(m,slot+16,0);rrj_write32(m,slot,channel);
        target=0x800D8768+4*(rrj_u16(rrj_at(m,record+4,2))&31);
        rrj_write32(m,target,rrj_read32(m,slot+8));
        rrj_write32(m,record,rrj_read32(m,slot+8));
        (void)dispatch_object_call(m,call,0x800333F4,0,group,0,0,0,0);
    }else if(rrj_read32(m,slot+12) && mode==1){
        (void)dispatch_object_call(m,call,0x8002227C,channel,rrj_read32(m,slot+8),rrj_read32(m,slot+12),rrj_read32(m,slot+16),group,0);
        (void)dispatch_object_call(m,call,0x80031008,rrj_read32(m,slot+12),0,0,0,0,0);
        rrj_write32(m,slot+12,0);rrj_write32(m,slot+16,0);rrj_write32(m,slot,channel);
        record=0x800D76D0+32*group+8*channel;
        target=0x800D8768+4*(rrj_u16(rrj_at(m,record+4,2))&31);
        id=rrj_read32(m,slot+8);
        if(channel&1)rrj_write32(m,target,(rrj_read32(m,target)&0xFFFF0000)|id);
        else{
            value=rrj_u16(rrj_at(m,target,2))|(id<<16);rrj_write32(m,target,value);
            rrj_put16(rrj_at(m,record+4,2),rrj_u16(rrj_at(m,record+4,2))|0x800);
        }
        rrj_write32(m,record,rrj_read32(m,slot+8));
        (void)dispatch_object_call(m,call,0x800333F4,1,group,0,0,0,0);
    }
    return 1;
}

/* 33F14 local material/lookup helpers retain C stack storage, not PSX addresses. */
static void sub_80033D70(uint32_t ids[3],uint32_t pointers[3],unsigned a,unsigned b)
{
    uint32_t t=ids[b];ids[b]=ids[a];ids[a]=t;
    t=pointers[b];pointers[b]=pointers[a];pointers[a]=t;
}
static void sub_80033DAC(uint32_t ids[3],uint32_t pointers[3])
{
    if(rrj_s32(ids[1])<0)return;
    if(rrj_s32(ids[0])<rrj_s32(ids[1]))sub_80033D70(ids,pointers,0,1);
    if(rrj_s32(ids[2])<0)return;
    if(rrj_s32(ids[1])<rrj_s32(ids[2])){
        sub_80033D70(ids,pointers,1,2);
        if(rrj_s32(ids[0])<rrj_s32(ids[1]))sub_80033D70(ids,pointers,0,1);
    }
}
static uint32_t material_half(RRJMemory *m,uint32_t address)
{
    uint32_t v=rrj_u16(rrj_at(m,address,2));return v&0x8000?v|0xFFFF0000:v;
}
static uint32_t material_byte(RRJMemory *m,uint32_t address){return *(uint8_t *)rrj_at(m,address,1);}
static unsigned material_index(const uint8_t material[9],const uint32_t ids[3])
{
    unsigned i=0;uint32_t id=rrj_u16(material);
    while(rrj_s32(id)<rrj_s32(ids[i])){if(++i>=3)abort();}
    return id==ids[i]?i:0;
}
static void sub_80022758(RRJMemory *m,uint8_t material[9],const uint32_t ids[3],const uint32_t pointers[3],uint32_t group)
{
    uint32_t count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
    uint32_t channel=rrj_read32(m,pointers[material_index(material,ids)]);
    uint32_t cfg=0x80053254+176*(count-1)+8*group,divisor,quotient,remainder,bits,x,y;
    rrj_put16(material+6,rrj_u16(rrj_at(m,0x800D7714+24*group+8*channel,2)));
    divisor=material_byte(m,cfg+4);quotient=divisor?material[2]/divisor:0xFFFFFFFF;remainder=divisor?material[2]%divisor:material[2];
    bits=material_byte(m,0x800533B6+88*(count-1)+4*group);
    x=(((channel&15)+(bits&15))<<6)+material_half(m,cfg)+remainder*material_byte(m,cfg+6);
    y=material_half(m,cfg+2)+quotient;
    rrj_put16(material+4,(y<<6)|((x>>4)&63));
}
static void sub_8002289C(RRJMemory *m,uint8_t material[9],const uint32_t ids[3],const uint32_t pointers[3],uint32_t group)
{
    uint32_t channel=rrj_read32(m,pointers[material_index(material,ids)]);
    uint32_t record=0x800D76D0+32*group+8*channel,count,bits,x,y;
    material[8]=(uint8_t)material_byte(m,record+7);rrj_put16(material+6,rrj_u16(rrj_at(m,record+4,2)));
    count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
    bits=pause_asr(channel,1)+material_byte(m,0x800533B6+88*(count-1)+4*(group+2));
    x=((bits&15)<<2)|3;y=((bits&16)<<4)+((channel&1)<<7)+material[2]+96;
    rrj_put16(material+4,(y<<6)|x);
}
static void material_special(RRJMemory *m,uint8_t material[9])
{
    uint32_t count,cfg,divisor,quotient,remainder,bits,x,y;
    material[8]=(uint8_t)material_byte(m,0x800D6168);rrj_put16(material+6,rrj_u16(rrj_at(m,0x800D6160,2)));
    count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);cfg=0x80053254+176*(count-1);
    divisor=material_byte(m,cfg+68);quotient=divisor?material[2]/divisor:0xFFFFFFFF;remainder=divisor?material[2]%divisor:material[2];
    bits=material_byte(m,0x800533D6+88*(count-1));
    x=((bits&15)<<6)+material_half(m,cfg+64)+remainder*material_byte(m,cfg+70);
    y=((bits&16)<<4)+material_half(m,cfg+66)+quotient;
    /* 8004CD84 / PsyQ getClut: arithmetic shift low six bits are identical. */
    rrj_put16(material+4,(y<<6)|((x>>4)&63));
}
static uint32_t sub_80033E5C(RRJMemory *m,uint32_t polygon)
{
    uint32_t value=rrj_u16(rrj_at(m,polygon+2,2));
    if(value&15)rrj_put16(rrj_at(m,polygon+2,2),value&240);return value&240;
}
static uint32_t sub_80033E7C(RRJMemory *m,uint32_t polygon)
{
    /* lbu followed by (value>>4)<16 always skips the original store. */
    return material_byte(m,polygon)|240;
}
static uint32_t sub_80033EA0(RRJMemory *m,uint32_t object_pointer,uint32_t polygon,const uint8_t material[9])
{
    uint32_t high=material_byte(m,polygon)>>4,result=0;
    if(!(rrj_read32(m,object_pointer+8)&64)){
        if(high>=15){high=30;result=1;}else if(material[8]&128)high+=15;
    }
    *(uint8_t *)rrj_at(m,polygon,1)=(uint8_t)((high<<3)|(material_byte(m,polygon)&15));return result;
}
static void material_add_uv(RRJMemory *m,uint32_t polygon,uint8_t offset,unsigned quad)
{
    static const unsigned positions[4]={5,9,13,15};unsigned i;
    for(i=0;i<3+quad;++i){uint8_t *p=rrj_at(m,polygon+positions[i],1);*p=(uint8_t)(*p+offset);}
}
uint32_t sub_80033F14(RRJMemory *m,uint32_t record,uint32_t mode,uint32_t group)
{
    uint32_t flags,result,first,second,model,polygon,index,table_offset,count,ids[3],pointers[3];
    uint8_t material[9]={0};unsigned j,quad;
    if(rrj_read32(m,record)==0xFFFFFFFF && rrj_read32(m,record+8)==0xFFFFFFFF)return 0xFFFFFFFF;
    flags=rrj_read32(m,record+12);if(!(flags&16) && (flags&3))rrj_write32(m,record+12,flags|16);
    result=rrj_read32(m,record+12)&(mode?8u:4u);if(result)return result;
    first=rrj_read32(m,record+(mode?96:104));if(!first)return 0;
    result=rrj_read32(m,first);if(rrj_s32(result)<0)return result;
    second=rrj_read32(m,record+(mode?100:108));
    if(second){result=rrj_read32(m,second);if(rrj_s32(result)<0)return result;}
    else if(rrj_read32(m,record+(mode?92:84))!=0xFFFFFFFF)return 0xFFFFFFFF;
    model=rrj_read32(m,record+4);
    if(!mode){result=rrj_read32(m,model+60);if(!result)return result;}
    for(j=0;j<2;++j){pointers[j]=rrj_read32(m,record+(mode?96:104)+4*j);ids[j]=rrj_read32(m,record+(mode?88:80)+4*j);}
    ids[2]=0xFFFFFFFF;pointers[2]=0;sub_80033DAC(ids,pointers);
    rrj_write32(m,record+12,rrj_read32(m,record+12)|(mode?8u:4u));
    model=rrj_read32(m,record+4);polygon=rrj_read32(m,model+(mode?56:60));
    index=mode?0:rrj_u16(rrj_at(m,model+4,2))+rrj_u16(rrj_at(m,model+6,2));
    count=rrj_u16(rrj_at(m,model+4,2))+(mode?1u:2u)*rrj_u16(rrj_at(m,model+6,2));
    result=mode?count:2u*rrj_u16(rrj_at(m,model+6,2));table_offset=12*index;
    while(rrj_s32(index)<rrj_s32(count)){
        for(quad=0;quad<2;++quad){
            uint32_t remaining=rrj_read32(m,rrj_read32(m,model+40)+table_offset+4+4*quad);
            while(rrj_s32(remaining)>0){
                uint32_t special;
                rrj_put16(material,rrj_u16(rrj_at(m,polygon+10,2)));material[2]=(uint8_t)material_byte(m,polygon+1);
                special=((rrj_u16(material)>>10)&31)==30;
                if(special)material_special(m,material);
                else if(mode)sub_8002289C(m,material,ids,pointers,group);
                else sub_80022758(m,material,ids,pointers,group);
                if(!mode && special)material_add_uv(m,polygon,material[8],quad);
                rrj_put16(rrj_at(m,polygon+10,2),rrj_u16(material+6));rrj_put16(rrj_at(m,polygon+6,2),rrj_u16(material+4));
                if(mode){if(sub_80033EA0(m,record+4,polygon,material))material_add_uv(m,polygon,material[8],quad);}
                else{(void)sub_80033E7C(m,polygon);(void)sub_80033E5C(m,polygon);}
                --remaining;polygon+=quad?24:20;
            }
            model=rrj_read32(m,record+4);
        }
        ++index;count=rrj_u16(rrj_at(m,model+4,2))+(mode?1u:2u)*rrj_u16(rrj_at(m,model+6,2));
        result=rrj_s32(index)<rrj_s32(count);table_offset+=12;
    }
    if(mode){rrj_write32(m,record+12,rrj_read32(m,record+12)|32);result=rrj_read32(m,record+12)|64;rrj_write32(m,record+12,result);}
    return result;
}

uint32_t sub_80031E1C(RRJMemory *m,uint32_t track,uint32_t object,uint32_t group)
{
    uint32_t index=0,end=0,info;
    if(!object){uint32_t manager=rrj_read32(m,0x8005ACBC);index=rrj_read32(m,manager+8*group+0xA40);end=rrj_read32(m,manager+8*group+0xA44);object=manager+44+36*index;}
    info=sub_800245DC(m,track);
    while(index<=end){
        if((rrj_read32(m,object)&17)==17){
            uint32_t kind=rrj_read32(m,rrj_read32(m,object+20))>>28;
            if(!kind || kind==8){
                uint32_t entries=rrj_read32(m,object+24),i,entry=entries,value;
                for(i=0;i<4;++i,entry+=12)if(rrj_read32(m,entry)==track)break;
                if(i<4)value=pause_asr(rrj_read32(m,entry+4),6);
                else{
                    uint32_t first=rrj_read32(m,entries),first_info=sub_800245DC(m,first),side;
                    value=0xFFFF0000;
                    for(side=0;side<2;++side){
                        uint32_t table=sub_800245F4(m,rrj_read32(m,info+8+4*side));
                        uint32_t count=rrj_read32(m,table+4),j;
                        for(j=0;j<count;++j)if(rrj_read32(m,table+8+8*j)==first)break;
                        if(j<rrj_read32(m,table+4)){
                            value=rrj_s32(rrj_read32(m,table+12+8*j))>0?rrj_read32(m,entries+4):rrj_read32(m,first_info+4)-rrj_read32(m,entries+8);
                            value=pause_asr(value,6);value=side?value+0x10000:0u-value;break;
                        }
                    }
                }
                rrj_write32(m,object+28,value);
            }
        }
        ++index;object+=36;
    }
    return 1;
}

uint32_t sub_800329BC(RRJMemory *m,uint32_t id,uint32_t group)
{
    uint32_t i=12*group,end=i+12,p=0x800D87E8+1344*group;
    for(;rrj_s32(i)<rrj_s32(end);++i,p+=112)if(rrj_read32(m,p)==id)return i;return 0xFFFFFFFF;
}
uint32_t sub_80032338(RRJMemory *m,uint32_t model_pointer,uint32_t buffer)
{rrj_write32(m,rrj_read32(m,model_pointer)+60,buffer);return 0;}
uint32_t sub_8003234C(RRJMemory *m,uint32_t record,uint32_t buffer)
{
    static const uint32_t fields[8]={36,32,40,44,48,52,56,60};
    uint32_t limit=2*(rrj_read32(m,buffer)-13),i,low=0,high=0,model;
    for(i=0;rrj_s32(i)<rrj_s32(limit);++i){
        uint32_t id=rrj_u16(rrj_at(m,buffer+4+2*i,2));
        if(id==0xFFFF)continue;
        if(id&0x8000){if(high<2)rrj_write32(m,record+88+4*high++,id);}
        else if(low<2)rrj_write32(m,record+80+4*low++,id);
    }
    model=buffer+4*rrj_read32(m,buffer);rrj_write32(m,record+4,model);
    *(uint8_t *)rrj_at(m,record+72,1)=(uint8_t)(material_byte(m,model+6)+1);
    *(uint8_t *)rrj_at(m,record+73,1)=(uint8_t)(material_byte(m,model+4)+2*rrj_u16(rrj_at(m,model+6,2)));
    for(i=0;i<8;++i){if(i)model=rrj_read32(m,record+4);rrj_write32(m,model+fields[i],rrj_read32(m,model+fields[i])+buffer);}
    return 1;
}
uint32_t sub_800325BC(RRJMemory *m,uint32_t record,uint32_t group)
{
    uint32_t mode,i;
    for(mode=0;mode<2;++mode)for(i=0;i<2;++i){
        uint32_t id=rrj_read32(m,record+80+8*mode+4*i),index;
        if(id==0xFFFFFFFF)break;index=sub_80022218(m,id,group);
        if(index!=0xFFFFFFFF)rrj_write32(m,record+(mode?96:104)+4*i,0x800D9268+48*index);
    }
    return 1;
}
uint32_t sub_80013204(RRJMemory *m,uint32_t id,uint32_t group)
{
    uint32_t record=0x800D87E8+1344*group,end=record+1344;
    while(record<end){
        uint32_t tag=rrj_read32(m,record),match=rrj_read32(m,record+8),model=rrj_read32(m,record+4);
        if(tag!=0xFFFFFFFF && match!=0xFFFFFFFF && model && match==id)return record;record+=112;
    }
    return 0;
}
uint32_t sub_80013360(RRJMemory *m,uint32_t id,uint32_t group)
{return material_byte(m,rrj_read32(m,0x8005B2F8)+4)&16?sub_80013204(m,id,group==0):0;}
uint32_t sub_80012BA8(RRJMemory *m,uint32_t header,uint32_t id,uint32_t group)
{
    uint32_t out=0x800D43C0+28*group,last;
    rrj_write32(m,out,rrj_read32(m,header+8));rrj_write32(m,out+4,rrj_read32(m,header+20));
    rrj_write32(m,out+8,rrj_read32(m,header+24));rrj_write32(m,out+12,rrj_read32(m,header+28));
    rrj_put16(rrj_at(m,out+16,2),rrj_u16(rrj_at(m,header+14,2)));rrj_put16(rrj_at(m,out+18,2),rrj_u16(rrj_at(m,header+16,2)));
    last=rrj_u16(rrj_at(m,header+18,2));rrj_write32(m,out+24,id);rrj_put16(rrj_at(m,out+22,2),1);rrj_put16(rrj_at(m,out+20,2),last);return out;
}
uint32_t sub_80013110(RRJMemory *m,uint32_t kind,uint32_t point,uint32_t group,uint32_t extra,uint32_t extended)
{
    uint32_t player=0x800CD898+1132*group,x=material_half(m,player+186)-material_half(m,point+2);
    uint32_t y=material_half(m,player+194)-material_half(m,point+10),sign,t,distance,limit;
    sign=pause_asr(x,31);x=(x+sign)^sign;sign=pause_asr(y,31);y=(y+sign)^sign;
    if(rrj_s32(x)<rrj_s32(y)){t=x;x=y;y=t;}
    t=y+pause_asr(y,1);distance=x-pause_asr(x,5)-pause_asr(x,7)+pause_asr(t,2)+pause_asr(t,6);
    if(kind==6)limit=pause_asr(rrj_read32(m,0x8005AD58)+extra,16);
    else limit=kind<2?(extended?350u:300u):(extended?230u:200u);
    return rrj_s32(distance)<rrj_s32(limit);
}
uint32_t sub_80013294(RRJMemory *m,uint32_t kind,uint32_t id,uint32_t group)
{
    uint32_t object,nearby,flags,bit,value;
    if(((id-1)&0xFFFF)>=223)return 0;
    object=rrj_read32(m,0x800CD6C4)+280*(id&31);
    nearby=sub_80013110(m,kind,object+12,group,0,1);flags=material_byte(m,object+3);bit=1u<<(group&31);
    if(flags&bit){if(nearby)return 0;value=flags&(1u<<((group^1)&31));}
    else{if(!nearby)return 0;value=flags|bit;}
    *(uint8_t *)rrj_at(m,object+3,1)=(uint8_t)value;return 1;
}
uint32_t sub_8001339C(RRJMemory *m,uint32_t id,uint32_t group)
{
    static const uint32_t counts[5]={2,0,6,4,8},fields[5]={40,36,48,44,52},strides[5]={76,68,88,64,64};
    uint32_t source,target,result,pass;
    if(!(material_byte(m,rrj_read32(m,0x8005B2F8)+4)&16))return 0;
    source=sub_80013204(m,id,group);result=sub_80013360(m,id,group);if(!source || !result)return result;
    source=rrj_read32(m,rrj_read32(m,source+4)+32);target=rrj_read32(m,rrj_read32(m,result+4)+32);
    for(pass=0;pass<5;++pass){
        uint32_t i=0,offset=0;result=rrj_u16(rrj_at(m,source+counts[pass],2));
        while(i<rrj_u16(rrj_at(m,source+counts[pass],2))){
            uint32_t sp=rrj_read32(m,source+fields[pass]),tp=rrj_read32(m,target+fields[pass]);
            uint32_t state=pass==3?rrj_read32(m,0x8005B2F8):0;
            rrj_put16(rrj_at(m,tp+offset+4,2),rrj_u16(rrj_at(m,sp+offset+4,2)));
            if(pass==2){
                sp=rrj_read32(m,source+48)+offset;
                if(rrj_s32(material_half(m,sp+4))>0)(void)sub_80013294(m,6,rrj_u16(rrj_at(m,sp+4,2)),group^1);
            }
            if(pass==3 && (material_byte(m,state+4)&16) && rrj_u16(rrj_at(m,rrj_read32(m,target+44)+offset+2,2))==50)
                (void)sub_80012BA8(m,target,id,group);
            ++i;result=i<rrj_u16(rrj_at(m,source+counts[pass],2));offset+=strides[pass];
        }
    }
    return result;
}
uint32_t sub_800135E8(RRJMemory *m,uint32_t model_pointer,uint32_t group)
{
    uint32_t header=rrj_read32(m,rrj_read32(m,model_pointer)+32),i,value;
    for(i=0;i<5;++i){value=rrj_u16(rrj_at(m,header+2*i,2))?header+rrj_read32(m,header+36+4*i):0;rrj_write32(m,header+36+4*i,value);}
    return sub_8001339C(m,rrj_read32(m,model_pointer+4),group);
}
uint32_t sub_80032A20(RRJMemory *m,const uint32_t args[6])
{
    uint32_t buffer=args[0],id=args[1],kind=args[2],group=args[5],index,record,flags;
    if(rrj_s32(rrj_read32(m,0x8005AEDC))>=24)return 0;
    index=sub_800329BC(m,id&0x0FFFFFFF,group);
    if(kind==0 || kind==8){
        if(index!=0xFFFFFFFF)return 3;
        index=sub_800329BC(m,0xFFFFFFFF,group);if(index==0xFFFFFFFF)return 0;
        record=0x800D87E8+112*index;rrj_write32(m,record,id);
        if(rrj_read32(m,record+8)==0xFFFFFFFF){
            rrj_write32(m,record+8,id);rrj_write32(m,record+12,0);rrj_write32(m,record+76,args[3]);
            (void)sub_8003234C(m,record,buffer);rrj_write32(m,0x8005AEDC,rrj_read32(m,0x8005AEDC)+1);
            (void)sub_800135E8(m,record+4,group);flags=rrj_read32(m,record+12);rrj_write32(m,record+12,flags|1);
            if(kind)rrj_write32(m,rrj_read32(m,record+4)+60,0);else rrj_write32(m,record+12,flags|3);
            rrj_write32(m,record+68,args[4]);
        }
    }else{
        if(index==0xFFFFFFFF)return 0;record=0x800D87E8+112*index;
        if(!rrj_read32(m,record+4))return 0;if(rrj_read32(m,record+12)&2)return 3;
        (void)sub_80032338(m,record+4,buffer);rrj_write32(m,record+12,rrj_read32(m,record+12)|2);
    }
    (void)sub_800325BC(m,record,group);(void)sub_80033F14(m,record,0,group);(void)sub_80033F14(m,record,1,group);return 1;
}

uint32_t sub_80023960(RRJMemory *m,uint32_t input,uint32_t group)
{
    uint32_t state=0x80053478+128*group,id=rrj_read32(m,state+72),i,value,result;
    for(i=0;i<4;++i)if(material_half(m,input+4+6*i)==id)break;
    if(i==4)return 0;
    value=material_half(m,input+6*i+(rrj_s32(rrj_read32(m,state+64))>0?6:8));
    i=rrj_s32(rrj_read32(m,state+64))>0;rrj_write32(m,state+104,value);
    result=i?value-rrj_read32(m,state+68):rrj_read32(m,state+68)-value;
    rrj_write32(m,state+100,result);return result;
}
uint32_t sub_800136BC(RRJMemory *m,uint32_t buffer,uint32_t input,uint32_t value)
{
    uint32_t manager,record,i,j,last;
    if(rrj_read32(m,0x8005B2D0) || rrj_read32(m,buffer+12)!=0x50414E4F)return 2;
    manager=rrj_read32(m,0x8005B278);if(rrj_s32(material_half(m,manager+34))>=20)return 0;
    record=manager+740;
    for(i=0;i<20;++i,record+=12)if(rrj_read32(m,record)){
        uint32_t candidate=rrj_read32(m,record+4),id=rrj_u16(rrj_at(m,input,2));
        /* Each candidate triple compares to the same input triple in MIPS. */
        for(j=0;j<4;++j,candidate+=6)
            if(rrj_u16(rrj_at(m,candidate,2))!=id || rrj_u16(rrj_at(m,candidate+2,2))!=rrj_u16(rrj_at(m,input+2,2)) || rrj_u16(rrj_at(m,candidate+4,2))!=rrj_u16(rrj_at(m,input+4,2)))break;
        if(j==4)return 3;
    }
    record=rrj_read32(m,0x8005B278)+740;
    for(i=0;i<20;++i,record+=12)if(!rrj_read32(m,record))break;
    if(i==20)return 0;
    last=rrj_u16(rrj_at(m,buffer+4,2));manager=rrj_read32(m,0x8005B278);rrj_write32(m,buffer+24,last);
    last=rrj_read32(m,buffer+8);rrj_write32(m,buffer+36,buffer);rrj_write32(m,buffer+28,last);
    rrj_put16(rrj_at(m,manager+34,2),rrj_u16(rrj_at(m,manager+34,2))+1);
    rrj_write32(m,record,buffer);rrj_write32(m,record+4,input);rrj_write32(m,record+8,value);return 1;
}
uint32_t sub_8003CA50(RRJMemory *m,uint32_t id,uint32_t buffer,uint32_t group)
{
    uint32_t state=rrj_read32(m,0x8005B2F8),free_index=0xFFFFFFFF,i,record;
    for(i=0;i<6;++i){
        record=0x800D4B10+16*i;
        if(rrj_read32(m,record)==id){
            if(!(material_byte(m,state+4)&16) || group==rrj_read32(m,record+4))return 0xFFFFFFFD;
            rrj_write32(m,record+8+4*group,buffer);return 0xFFFFFFFF;
        }
        if(free_index==0xFFFFFFFF && rrj_read32(m,record)==0xFFFFFFFF)free_index=i;
    }
    if(free_index==0xFFFFFFFF)return 0;
    record=0x800D4B10+16*free_index;rrj_write32(m,record,id);rrj_write32(m,record+4,group);
    rrj_write32(m,record+8+4*group,buffer);rrj_write32(m,record+8+4*(group^1),0);
    i=rrj_read32(m,0x8005B320)+1;group=rrj_s32(free_index)<rrj_s32(rrj_read32(m,0x8005B31C));
    rrj_write32(m,0x8005B320,i);if(!group)rrj_write32(m,0x8005B31C,free_index);return 1;
}

uint32_t sub_80039A08(RRJMemory *m,uint32_t id)
{
    uint32_t manager,mapped,threshold,address;
    if(rrj_s32(id)<0)return 0;manager=rrj_read32(m,0x8005B240);
    if(rrj_s32(id)>=rrj_s32(material_half(m,manager+40)))return 0;
    threshold=material_half(m,manager+44);mapped=rrj_s32(id)<rrj_s32(threshold)?id+material_half(m,manager+42):id-threshold;
    manager=rrj_read32(m,0x8005B240);if(rrj_s32(mapped)>=rrj_s32(material_half(m,manager+40)))return 0;
    address=rrj_read32(m,manager+28)+(mapped<<5);return rrj_read32(m,address)==id?address:0;
}
static unsigned segment_tag_equal(RRJMemory *m,uint32_t a,uint32_t b)
{
    unsigned i;for(i=0;i<4;++i){uint32_t x=material_byte(m,a+i),y=material_byte(m,b+i);if(x!=y)return 0;if(!x)return 1;}return 1;
}
uint32_t sub_8003CFCC(RRJMemory *m,uint32_t buffer,uint32_t id)
{
    static const unsigned fields[12]={44,48,52,56,60,64,68,72,92,96,100,104};
    uint32_t record=sub_80039A08(m,id&0xFFFF),p=buffer+108,end=buffer+16384,global,data;unsigned i;
    if(!record)return 0;
    while(p<end){
        for(i=0;i<12;++i)if(segment_tag_equal(m,p,0x8005AEEC+8*i))break;
        if(i==12)break;rrj_write32(m,buffer+fields[i],p+8);p+=rrj_read32(m,p+4);
    }
    global=rrj_read32(m,0x8005AD54);rrj_write32(m,record+12,buffer);if(!global)rrj_write32(m,0x8005AD54,buffer);
    if((material_byte(m,rrj_read32(m,0x8005B2F8)+4)&16) && buffer && rrj_read32(m,buffer)==11){
        data=rrj_read32(m,buffer+96)+1560;rrj_write32(m,data+8,0x0B897B7D);rrj_write32(m,data+12,0xFFF0DC54);rrj_write32(m,data+16,0x1538BC84);
        data=rrj_read32(m,buffer+96)+400;rrj_write32(m,data+8,0x0B775E29);rrj_write32(m,data+12,0xFFF0BB49);rrj_write32(m,data+16,0x1515ACCF);
        data=rrj_read32(m,buffer+96)+1000;rrj_write32(m,data+8,0x0B38FCEC);rrj_write32(m,data+12,0xFFF0BB49);rrj_write32(m,data+16,0x15361E79);
        data=rrj_read32(m,buffer+96)+1360;rrj_write32(m,data+12,rrj_read32(m,data+24));
    }
    if(buffer && rrj_read32(m,buffer)==105){
        data=rrj_read32(m,buffer+96)+2520;rrj_write32(m,data+8,0xED2431AD);rrj_write32(m,data+12,0xFE062800);rrj_write32(m,data+16,0xEBC76784);
        data=rrj_read32(m,buffer+96)+2480;rrj_write32(m,data+20,0xED2431AD);rrj_write32(m,data+24,0xFE062800);rrj_write32(m,data+28,0xEBC76784);
        rrj_write32(m,data+8,0xED08D8CA);rrj_write32(m,data+12,0xFE0A3D4D);rrj_write32(m,data+16,0xEBC6469D);
        data=rrj_read32(m,buffer+96)+2440;rrj_write32(m,data+20,0xED08D8CA);rrj_write32(m,data+24,0xFE0A3D4D);rrj_write32(m,data+28,0xEBC6469D);
        rrj_write32(m,data+8,0xECED651D);rrj_write32(m,data+12,0xFE09E617);rrj_write32(m,data+16,0xEBBF1AEC);
    }
    return 1;
}
/* Effects-only ABI: all audited callers discard the residual MIPS v0. */
void sub_80093F94(RRJMemory *m,uint32_t object,uint32_t previous,RRJReverbCall reverb)
{
    uint32_t active;
    if(!object)return;
    active=rrj_u16(rrj_at(m,object+320,2));
    if((active&1u)==(previous&1u))return;
    if(active)(void)sub_8009432C(m,object);
    else sub_80093FE4(m,object,reverb);
}
void sub_80093ED4(RRJMemory *m,uint32_t object,uint32_t force,RRJReverbCall reverb)
{
    uint32_t previous,value;
    if(!rrj_u16(rrj_at(m,object+320,2)) &&
       rrj_u16(rrj_at(m,object+172,2))>=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48) &&
       (material_byte(m,rrj_read32(m,object+1084)+1)&15u)==2u &&
       !(material_byte(m,object+928)&16u))return;
    previous=material_half(m,object+320);
    value=force?0:sub_80039F68(m,object+172);
    rrj_put16(rrj_at(m,object+320,2),(uint16_t)((0u-value)&((rrj_u16(rrj_at(m,object+320,2))&2u)|1u)));
    sub_80093F94(m,object,previous,reverb);
}
/* 80037338: live scratchpad actor list shared with the frame's other passes. */
uint32_t sub_80037338(RRJMemory *m)
{
    uint32_t cursor=0x1f800004u,end=rrj_read32(m,0x1f800000u),actor,flags,state;
    for(;;){
        /* The original fetches the sentinel entry before comparing the end. */
        actor=rrj_read32(m,cursor);
        if(cursor>=end)return 0;
        if(rrj_read32(m,actor+828)){
            flags=rrj_read32(m,actor+388)&0xffffff7fu;
            state=rrj_read32(m,actor+564)&0xfbffffffu;
            rrj_write32(m,actor+388,flags);rrj_write32(m,actor+564,state);
        }else{
            rrj_write32(m,actor+388,rrj_read32(m,actor+388)|0x80u);
            rrj_write32(m,actor+564,rrj_read32(m,actor+564)|0x04000000u);
        }
        if(rrj_read32(m,actor+560)&0x08000000u)flags=rrj_read32(m,actor+388)&0xffffffdfu;
        else{sub_800396A8(m,actor);flags=rrj_read32(m,actor+388)|0x20u;}
        rrj_write32(m,actor+388,flags);
        (void)sub_8003701C(m,actor);
        if(rrj_read32(m,actor+564)&0x40000u)
            rrj_write32(m,actor+364,rrj_read32(m,rrj_read32(m,actor+832)+192));
        (void)sub_80037104(m,actor);
        cursor+=4;end=rrj_read32(m,0x1f800000u);
    }
}
/* 8003AE24: route assignment, opposite-direction marker and road zones. */
uint32_t sub_8003AE24(RRJMemory *m)
{
    uint32_t cursor=0x1F800004,end=rrj_read32(m,0x1F800000),actor,linked,descriptor,word,record;
    actor=rrj_read32(m,cursor);
    /* The initial branch delay slot overwrites v0 with LUI 800D. */
    if(cursor>=end)return 0x800D0000;
    do{
        sub_8003AF9C(m,actor+172,1,0);
        linked=rrj_read32(m,actor+856);
        if(linked)rrj_write32(m,linked+428,rrj_read32(m,actor+428));
        descriptor=rrj_read32(m,actor+1084);
        *(uint8_t *)rrj_at(m,descriptor,1)=(uint8_t)(material_byte(m,descriptor)&127u);
        if(material_half(m,0x800D6182)!=0xffffffffu &&
           rrj_read32(m,rrj_read32(m,actor+852)+604)<3u &&
           (rrj_u16(rrj_at(m,actor+172,2))<rrj_read32(m,rrj_read32(m,0x8005B2F8)+48) ||
            (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)!=2u)){
            word=rrj_read32(m,actor+360);
            if(!(word>>16)){
                record=sub_8003B4B0(m,rrj_read32(m,actor+428),word&65535u);
                if(record && ((rrj_read32(m,record+4)^rrj_read32(m,actor+364))&0x80000000u)){
                    descriptor=rrj_read32(m,actor+1084);
                    *(uint8_t *)rrj_at(m,descriptor,1)=(uint8_t)(material_byte(m,descriptor)|128u);
                }
            }
        }
        (void)sub_8003A9D8(m,actor);
        cursor+=4;end=rrj_read32(m,0x1F800000);actor=rrj_read32(m,cursor);
    }while(cursor<end);
    return 0;
}
uint32_t sub_8003B520(RRJMemory *m)
{
    uint32_t cursor=0x1F800004,end=rrj_read32(m,0x1F800000),actor,previous,value,limit,descriptor,linked;
    for(;;){
        actor=rrj_read32(m,cursor);
        if(cursor>=end)return 0;
        previous=rrj_read32(m,actor+324);value=sub_8003B61C(m,actor+172);
        rrj_write32(m,actor+324,value);limit=0x649581;
        while(rrj_s32(value)<rrj_s32(limit)){
            descriptor=rrj_read32(m,actor+1084);
            if(material_byte(m,descriptor+68)&1u)break;
            if(rrj_s32(limit)<rrj_s32(previous))
                *(uint8_t *)rrj_at(m,descriptor+68,1)=(uint8_t)(material_byte(m,descriptor+68)|1u);
            else limit+=0xFFE6DAA0u;
            value=rrj_read32(m,actor+324);
        }
        linked=rrj_read32(m,actor+856);
        if(linked)rrj_write32(m,linked+324,rrj_read32(m,actor+324));
        cursor+=4;end=rrj_read32(m,0x1F800000);
    }
}
/* 8003E150: refresh contacts for queued actors and their linked actors. */
uint32_t sub_8003E150(RRJMemory *m)
{
    uint32_t cursor=0x1F800004,end=rrj_read32(m,0x1F800000),actor,linked;
    for(;;){
        actor=rrj_read32(m,cursor); /* Sentinel load precedes unsigned test. */
        if(cursor>=end)return 0;
        (void)sub_8003DFF4(m,actor);
        linked=rrj_read32(m,actor+0x358);
        if(linked)(void)sub_8003DFF4(m,linked);
        if(rrj_read32(m,actor+0x238)&0x800u)
            rrj_write32(m,actor+0x184,rrj_read32(m,actor+0x184)|0x40u);
        cursor+=4;end=rrj_read32(m,0x1F800000);
    }
}
uint32_t sub_80093E6C(RRJMemory *m,RRJReverbCall reverb)
{
    uint32_t result=rrj_read32(m,0x800CE4DC),count=rrj_read32(m,result),object=rrj_read32(m,0x800CE4D0);
    while(rrj_s32(count)>=0){sub_80093ED4(m,object,0,reverb);result=rrj_read32(m,0x800CE4D4);--count;object+=result;}
    return result;
}
uint32_t rrj_probe_actor_activity(RRJMemory *m,RRJRacePlayerCall call)
{
    uint32_t result=rrj_read32(m,0x800CE4DC),count=rrj_read32(m,result),object=rrj_read32(m,0x800CE4D0);
    if(!call)abort();
    while(rrj_s32(count)>=0){(void)call(m,0x80093ED4,object,0,0);result=rrj_read32(m,0x800CE4D4);--count;object+=result;}
    return result;
}
static uint32_t update_activity_list(RRJMemory *m,RRJRacePlayerCall diagnostic,RRJReverbCall reverb)
{
    uint32_t result=rrj_read32(m,0x800CE4DC),count=rrj_read32(m,result),object=rrj_read32(m,0x800CE4D0),body;
    while(rrj_s32(count)>=0){
        body=rrj_read32(m,object+852);if(rrj_read32(m,body+604)-3<2){if(diagnostic)(void)diagnostic(m,0x800951B8,body,0,0);else sub_800951B8(m,body,0,reverb);}
        body=rrj_read32(m,object+852);
        if(material_byte(m,body+572)&16){body=rrj_read32(m,rrj_read32(m,object+856)+852);if(rrj_read32(m,body+604)-3<2){if(diagnostic)(void)diagnostic(m,0x800951B8,body,0,0);else sub_800951B8(m,body,0,reverb);}}
        result=rrj_read32(m,0x800CE4D4);--count;object+=result;
    }
    return result;
}
uint32_t sub_800950E8(RRJMemory *m,RRJReverbCall reverb)
{return update_activity_list(m,NULL,reverb);}
uint32_t rrj_probe_activity_list(RRJMemory *m,RRJRacePlayerCall diagnostic)
{if(!diagnostic)abort();return update_activity_list(m,diagnostic,NULL);}
static uint32_t load_segment(RRJMemory *m,uint32_t buffer,uint32_t id,uint32_t group,RRJRacePlayerCall diagnostic,RRJReverbCall reverb)
{
    uint32_t result,player;id&=0xFFFF;
    if(rrj_s32(id)>=rrj_s32(material_half(m,rrj_read32(m,0x8005B240)+40)))return 2;
    result=sub_8003CA50(m,id,buffer,group);if(rrj_s32(result)<=0)return rrj_s32(result)<0?0u-result:result;
    if(!sub_8003CFCC(m,buffer,id))return 0;
    player=rrj_read32(m,0x8005B38C);
    if(player && !rrj_read32(m,rrj_read32(m,player+1084)+40))goto refresh;
    player=rrj_read32(m,0x8005B21C);
    if(!player || rrj_read32(m,rrj_read32(m,player+1084)+40))return 1;
refresh:
    if(diagnostic){(void)rrj_probe_actor_activity(m,diagnostic);(void)rrj_probe_activity_list(m,diagnostic);}
    else{(void)sub_80093E6C(m,reverb);(void)sub_800950E8(m,reverb);}
    return 1;
}

uint32_t sub_8003CEC4(RRJMemory *m,uint32_t buffer,uint32_t id,uint32_t group,RRJReverbCall reverb)
{return load_segment(m,buffer,id,group,NULL,reverb);}
uint32_t rrj_probe_segment_load(RRJMemory *m,uint32_t buffer,uint32_t id,uint32_t group,RRJRacePlayerCall diagnostic)
{if(!diagnostic)abort();return load_segment(m,buffer,id,group,diagnostic,NULL);}

/* RASHCDG 80099D48: follow the linked object's road/junction position. */
uint32_t sub_80099D48(RRJMemory *m,uint32_t object)
{
    uint32_t road=0,linked=rrj_read32(m,object+596),route=rrj_read32(m,object+360);
    uint32_t id=route&0xFFFF,linked_id=rrj_u16(rrj_at(m,linked+360,2));
    uint32_t junction=0,linked_road=0,linked_junction=0,step=0,unmatched=1;
    uint32_t p,end,length,zero_progress=0,previous,progress,result;
    if(id==linked_id){
        if(!(route>>16))step=rrj_s32(rrj_read32(m,object+368))<rrj_s32(rrj_read32(m,linked+368))?0x50000:0xFFFB0000;
        goto finish;
    }
    if(route>>16)junction=sub_800245F4(m,id);else road=sub_800245DC(m,id);
    if(rrj_u16(rrj_at(m,linked+362,2)))linked_junction=sub_800245F4(m,linked_id);else linked_road=sub_800245DC(m,linked_id);
    if(linked_road){
        length=(rrj_read32(m,linked_road+4)>>6)<<16;
        if(junction){
            p=junction+8;end=p+(rrj_read32(m,junction+4)<<3);
            while(p<end){
                if(rrj_read32(m,p)==linked_id){
                    road=linked_road;rrj_write32(m,object+360,linked_id);unmatched=0;
                    zero_progress=rrj_s32(rrj_read32(m,p+4))>0;break;
                }
                p+=8;
            }
        }else{
            uint32_t head=rrj_read32(m,road+8),linked_head=rrj_read32(m,linked_road+8),linked_tail=rrj_read32(m,linked_road+12);
            if(head==linked_head || head==linked_tail){unmatched=0;step=0xFFFB0000;}
            else{
                uint32_t tail=rrj_read32(m,road+12);
                if(tail==linked_head || tail==linked_tail){unmatched=0;step=0x50000;}
            }
        }
        if(unmatched){
            road=linked_road;rrj_write32(m,object+360,linked_id);
            zero_progress=rrj_s32(pause_asr(length,1))<rrj_s32(rrj_read32(m,linked+368));
        }
        if(!step)rrj_write32(m,object+368,zero_progress?0:length);
    }else{
        p=linked_junction+8;
        if(road){
            end=p+(rrj_read32(m,linked_junction+4)<<3);
            while(p<end){
                if(rrj_read32(m,p)==id){step=rrj_s32(rrj_read32(m,p+4))<0?0x50000:0xFFFB0000;unmatched=0;break;}
                p+=8;
            }
        }
        if(unmatched){
            id=rrj_read32(m,linked_junction+8);rrj_write32(m,object+360,id);road=sub_800245DC(m,id);
            rrj_write32(m,object+368,rrj_s32(rrj_read32(m,linked_junction+12))<0?0:(rrj_read32(m,road+4)>>6)<<16);
        }
    }
finish:
    previous=rrj_read32(m,object+368);progress=previous+step;rrj_write32(m,object+368,progress);
    if(!road)return previous;
    if(rrj_s32(progress)<0)result=rrj_u16(rrj_at(m,road+8,2))|0x10000;
    else{
        length=(rrj_read32(m,road+4)>>6)<<16;
        if(rrj_s32(length)>=rrj_s32(progress))return 0;
        result=rrj_u16(rrj_at(m,road+12,2))|0x10000;
    }
    rrj_write32(m,object+360,result);return result;
}

uint32_t sub_80039C90(RRJMemory *m,uint32_t buffer,uint32_t id)
{
    uint32_t p,end;
    if(!buffer || material_half(m,buffer+16)!=1)return 0;
    p=rrj_read32(m,buffer+44);end=p+(material_half(m,buffer+18)<<5);
    while(p<end){if(!(material_half(m,p+2)|(rrj_read32(m,p+12)-id)))return p;p+=32;}
    return 0;
}
uint32_t sub_80039CFC(RRJMemory *m,uint32_t record,uint32_t position)
{
    uint32_t buffer,kind,tag,p;int32_t progress;
    if(!position || !record)return 0;buffer=rrj_read32(m,record+12);if(!buffer)return 0;
    kind=material_half(m,record+4);tag=(kind<<16)|rrj_u16(rrj_at(m,record+16,2));
    progress=rrj_s32(material_half(m,position+10));
    if(!kind)return tag==rrj_read32(m,position) && progress>=rrj_s32(material_half(m,record+22)) && progress<=rrj_s32(material_half(m,record+26));
    if(tag==rrj_read32(m,position))return !rrj_read32(m,record+28);
    p=sub_80039C90(m,buffer,rrj_read32(m,position)&0xFFFF);
    return p && !material_half(m,p+2) && progress>=rrj_s32(material_half(m,p+26)) && progress<=rrj_s32(material_half(m,p+30));
}
uint32_t sub_80039DFC(RRJMemory *m,uint32_t object_position,uint32_t position)
{
    uint32_t prior_id=0xFFFFFFFF,prior_buffer=0,record,id,i,tag;
    if(object_position && material_half(m,object_position+148)){
        record=sub_80039A08(m,rrj_read32(m,rrj_read32(m,object_position+156)));if(!record)return 0;
        tag=rrj_u16(rrj_at(m,object_position,2));
        if(((tag>>5)<2 && rrj_s32(tag&31)<rrj_s32(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48))) || sub_80039CFC(m,record,position))return rrj_read32(m,record+12);
        prior_buffer=rrj_read32(m,record+12);prior_id=rrj_read32(m,rrj_read32(m,object_position+156));
    }
    for(i=0;rrj_s32(i)<=rrj_s32(rrj_read32(m,0x8005B31C));++i){
        id=rrj_read32(m,0x800D4B10+16*i);
        if(id==0xFFFFFFFF || id==prior_id)continue;
        record=sub_80039A08(m,id);
        if(record && sub_80039CFC(m,record,position))return prior_id==0xFFFFFFFF?rrj_read32(m,record+12):prior_buffer;
    }
    return 0;
}

uint32_t sub_8003F3B4(RRJMemory *m,uint32_t id)
{
    uint32_t count=material_half(m,0x800D6182),p,i;
    if(rrj_s32(count)<=0)return 0;p=rrj_read32(m,0x800D6194);
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,p+=120)if(rrj_read32(m,p)==id)return p;
    return 0;
}
uint32_t sub_80039B60(RRJMemory *m,uint32_t id)
{
    uint32_t manager,count,p,i;
    if(rrj_s32(id)<0)return 0;manager=rrj_read32(m,0x8005B240);count=material_half(m,manager+60);p=rrj_read32(m,manager+48);
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,p+=12)if(material_half(m,p)==id)return p;
    return 0;
}
static uint32_t project_vector(RRJMemory *m,uint32_t origin,uint32_t direction,uint32_t scale,uint32_t address,uint32_t *local)
{
    uint32_t i,result=0;
    for(i=0;i<3;++i){
        int64_t product=(int64_t)rrj_s32(material_half(m,direction+2*i)<<4)*(int64_t)rrj_s32(scale);
        result=(uint32_t)((uint64_t)product>>16)+rrj_read32(m,origin+4*i);
        if(local)local[i]=result;else rrj_write32(m,address+4*i,result);
    }
    return result;
}
uint32_t sub_8002EAD8(RRJMemory *m,uint32_t origin,uint32_t direction,uint32_t scale,uint32_t output)
{return project_vector(m,origin,direction,scale,output,NULL);}
/* Sequential loads preserve shifted aliases. Upper return word is residual v1,
 * not a carry from the origin addition (8002E570..8002E600). */
uint64_t sub_8002E570(RRJMemory *m,uint32_t origin,uint32_t velocity,uint32_t scale,uint32_t output)
{
    unsigned i;uint64_t product=0;uint32_t value=0;
    for(i=0;i<3;++i){
        product=(uint64_t)sub_8001FC90(rrj_s32(rrj_read32(m,velocity+4*i)),rrj_s32(scale));
        value=(uint32_t)product+rrj_read32(m,origin+4*i);
        rrj_write32(m,output+4*i,value);
    }
    return (product&UINT64_C(0xFFFFFFFF00000000))|value;
}
/* The original captures all six halfwords before any output store. scale_b
 * is the fifth argument at entry SP+16, absent from the IDA draft signature. */
static uint64_t combine_short_vectors(RRJMemory *m,uint32_t input_a,uint32_t input_b,uint32_t output,uint16_t *local,uint32_t scale_a,uint32_t scale_b)
{
    uint32_t a[3],b[3],value=0;uint64_t product=0;unsigned i;
    for(i=0;i<3;++i){a[i]=material_half(m,input_a+2*i)<<4;b[i]=material_half(m,input_b+2*i)<<4;}
    for(i=0;i<3;++i){
        uint32_t first=(uint32_t)sub_8001FC90(rrj_s32(scale_a),rrj_s32(a[i]));
        product=(uint64_t)sub_8001FC90(rrj_s32(scale_b),rrj_s32(b[i]));
        value=pause_asr(first+(uint32_t)product,4);
        if(local)local[i]=(uint16_t)value;else rrj_put16(rrj_at(m,output+2*i,2),(uint16_t)value);
    }
    return (product&UINT64_C(0xFFFFFFFF00000000))|value;
}
uint64_t sub_8002EB78(RRJMemory *m,uint32_t input_a,uint32_t input_b,uint32_t output,uint32_t scale_a,uint32_t scale_b)
{return combine_short_vectors(m,input_a,input_b,output,NULL,scale_a,scale_b);}
uint32_t rrj_project_vector_local(RRJMemory *m,uint32_t origin,uint32_t direction,uint32_t scale,uint32_t output[3])
{return project_vector(m,origin,direction,scale,0,output);}

uint32_t sub_80037524(RRJMemory *m,uint32_t record,uint32_t delta)
{
    uint32_t subrecord,segment,route,distance,last,current,progress,index,candidate=0,limit;
    if(!record)return 0;
    subrecord=rrj_read32(m,record+8);segment=rrj_read32(m,record);route=rrj_read32(m,record+4);
    distance=rrj_read32(m,segment+68);
    last=rrj_read32(m,segment+52)+52*(material_half(m,subrecord+8)+material_half(m,subrecord+10))-52;
    current=rrj_read32(m,rrj_read32(m,record+12)+40);progress=rrj_s32(delta)>0?current+delta:current-delta;
    if(material_half(m,route+2) || rrj_s32(material_half(m,segment+28))<=0 || !distance)return 0;
    if(rrj_s32(pause_asr(progress,16))>=rrj_s32(pause_asr(rrj_read32(m,route+24),16)) && rrj_s32(pause_asr(progress,16))<=rrj_s32(material_half(m,route+30))){
        index=(uint32_t)(rrj_s32(pause_asr(progress-rrj_read32(m,route+24),16))/50)-1;
        limit=material_half(m,subrecord+22)-1;if(rrj_s32(limit)<rrj_s32(index))index=limit;
        if(rrj_s32(index)<0)candidate=rrj_read32(m,segment+52)+52*material_half(m,subrecord+8);
        else candidate=rrj_read32(m,segment+52)+52*(material_half(m,subrecord+8)+material_half(m,distance+4*(material_half(m,subrecord+20)+index)+2));
        limit=material_half(m,subrecord+10);
        while(rrj_s32(material_half(m,candidate))<rrj_s32(limit)){
            if(rrj_s32(rrj_read32(m,candidate+32))>=rrj_s32(progress-rrj_read32(m,candidate+40)))break;
            candidate+=52;
        }
        if(candidate<=last){rrj_write32(m,record+12,candidate);rrj_write32(m,record+20,progress-rrj_read32(m,candidate+40));return candidate;}
    }else if(rrj_s32(pause_asr(progress,16))<rrj_s32(material_half(m,route+30))){
        candidate=rrj_read32(m,segment+52)+52*material_half(m,subrecord+8);rrj_write32(m,record+20,0);rrj_write32(m,record+12,candidate);return candidate;
    }
    rrj_write32(m,record+12,last);rrj_write32(m,record+20,rrj_read32(m,last+32));return candidate;
}
/* The native caller owns its temporary output word; no emulated stack allocation. */
static uint32_t track_link_match(RRJMemory *m,uint32_t record,uint32_t position,uint32_t selector,uint32_t *value,unsigned *written)
{
    uint32_t route,segment,i,j,piece,metadata,manager,entry,link,index;
    *written=0;if(!record || selector==0xFFFFFFFF)return 0;
    route=rrj_read32(m,record+4);segment=rrj_read32(m,record);
    for(i=0;rrj_s32(i)<rrj_s32(material_half(m,route+22));++i){
        piece=rrj_read32(m,segment+48)+28*(material_half(m,route+20)+i);
        if(pause_asr(rrj_read32(m,piece+12)+0x8000,16)!=pause_asr(position,12))continue;
        metadata=sub_80039B60(m,rrj_read32(m,segment));if(!metadata)continue;
        for(j=0;rrj_s32(j)<rrj_s32(material_half(m,metadata+8));++j){
            index=material_half(m,metadata+6)+j;manager=rrj_read32(m,0x8005B240);entry=rrj_read32(m,manager+52)+12*index;
            if(material_half(m,entry+10)!=selector)continue;
            link=rrj_read32(m,manager+56)+12*material_half(m,entry+2);
            if(piece!=rrj_read32(m,segment+48)+28*(material_half(m,route+20)+material_half(m,link+4)))continue;
            *value=material_half(m,link+8);if(rrj_s32(material_half(m,entry+6))<=0)*value=0u-*value;*written=1;return piece;
        }
    }
    return 0;
}
uint32_t sub_8003BE1C(RRJMemory *m,uint32_t record,uint32_t position,uint32_t selector,uint32_t output)
{
    uint32_t value=0,result;unsigned written;
    result=track_link_match(m,record,position,selector,&value,&written);if(written)rrj_write32(m,output,value);return result;
}
uint32_t sub_8003A700(RRJMemory *m,uint32_t segment,uint32_t position,uint32_t output)
{
    uint32_t route_word,high,route,subrecord,record,metadata,entry=0,base1,base2,pieces,i,count,start,link,value;unsigned written;
    if(!segment)return 0;route_word=rrj_read32(m,position);high=route_word>>16;
    if(!high){
        route=material_half(m,segment+16)?sub_80039C90(m,segment,route_word&0xFFFF):rrj_read32(m,segment+44);if(!route)return 0;
        subrecord=rrj_read32(m,segment+48)+28*material_half(m,route+20);
        rrj_write32(m,output,segment);rrj_write32(m,output+4,route);rrj_write32(m,output+8,subrecord);
        record=rrj_read32(m,segment+52);value=material_half(m,subrecord+8);
        rrj_write32(m,output+16,0);rrj_write32(m,output+20,0);rrj_write32(m,output+24,0);rrj_write32(m,output+28,0);rrj_write32(m,output+12,record+52*value);
        (void)sub_80037524(m,output,rrj_read32(m,position+8)-rrj_read32(m,route+24));return 1;
    }
    if(material_half(m,segment+16)!=high || rrj_read32(m,segment+8)!=(route_word&0xFFFF) || rrj_read32(m,segment+12))return 0;
    route=rrj_read32(m,segment+44);subrecord=rrj_read32(m,segment+48);record=sub_8003F3B4(m,rrj_read32(m,segment+8));if(!record)return 0;
    if(rrj_s32(rrj_read32(m,record+16))>0){
        rrj_write32(m,output,segment);rrj_write32(m,output+4,route);
        subrecord=track_link_match(m,output,rrj_read32(m,record+8),rrj_read32(m,record+84),&value,&written);
    }
    if(!subrecord)return 0;
    rrj_write32(m,output,segment);rrj_write32(m,output+4,route);rrj_write32(m,output+8,subrecord);
    rrj_write32(m,output+12,rrj_read32(m,segment+52)+52*material_half(m,subrecord+8));metadata=sub_80039B60(m,rrj_read32(m,segment));
    if(metadata){
        count=material_half(m,metadata+8);
        if(rrj_s32(count)>0){
            start=material_half(m,metadata+6);record=rrj_read32(m,0x8005B240);pieces=rrj_read32(m,segment+48);base1=rrj_read32(m,record+52);base2=rrj_read32(m,record+56);
            for(i=0;rrj_s32(i)<rrj_s32(count);++i){
                entry=base1+12*(start+i);link=base2+12*material_half(m,entry+2);
                if(material_half(m,link+6) && pieces+28*material_half(m,link+4)==subrecord)break;
            }
        }
        rrj_write32(m,output+24,metadata);rrj_write32(m,output+28,entry);
    }else{rrj_write32(m,output+24,0);rrj_write32(m,output+28,0);}
    rrj_write32(m,output+20,0);rrj_write32(m,output+16,0);return 1;
}

uint32_t sub_80039F68(RRJMemory *m,uint32_t input)
{
    uint32_t segment=sub_80039DFC(m,input,input+188),state=0,aggregate=0,tag,object,piece,i,count,minimum=999;
    if(!segment)return 0;state=material_half(m,input+148);tag=rrj_u16(rrj_at(m,input,2));
    if((tag>>5)<2 && !state){
        if(!sub_8003A700(m,segment,input+188,input+156))return 0;
        piece=rrj_read32(m,input+168);
        (void)sub_8002EAD8(m,piece+20,piece+14,rrj_read32(m,input+176),input+12);
        (void)sub_8002EAD8(m,input+12,piece+2,rrj_read32(m,input+172),input+12);
    }
    tag=rrj_u16(rrj_at(m,input,2));count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
    if((tag>>5)<2 && rrj_s32(tag&31)<rrj_s32(count)){
        if(tag>>5)state=1;
        else{
            object=rrj_read32(m,0x8005B3A0)+1096*tag;
            if(object){if(!rrj_read32(m,object+1088))object=rrj_read32(m,object+856);if(object && rrj_read32(m,rrj_read32(m,object+852)+604)<3)state=1;}
        }
        if(state)return state;
    }
    /* Original computes this minimum but does not use it to select a branch. */
    i=0;do{
        uint32_t player=0x800CD898+1132*i,x=material_half(m,player+186)-material_half(m,input+14),y=material_half(m,player+194)-material_half(m,input+22),sign,t,d;
        sign=pause_asr(x,31);x=(x+sign)^sign;sign=pause_asr(y,31);y=(y+sign)^sign;
        if(rrj_s32(x)<rrj_s32(y)){t=x;x=y;y=t;}t=y+pause_asr(y,1);d=x-pause_asr(x,5)-pause_asr(x,7)+pause_asr(t,2)+pause_asr(t,6);
        if(rrj_s32(d)<rrj_s32(minimum))minimum=d;
        count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);++i;
    }while(i<count);(void)minimum;
    if(state){
        for(i=0;rrj_s32(i)<rrj_s32(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48));++i)aggregate|=sub_80013110(m,rrj_u16(rrj_at(m,input,2))>>5,input+12,i,0x140000,1);
        state=aggregate;
    }
    if(!material_half(m,input+148)){
        for(i=0;rrj_s32(i)<rrj_s32(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48));++i)aggregate|=sub_80013110(m,rrj_u16(rrj_at(m,input,2))>>5,input+12,i,0,0);
        state=aggregate;
    }
    if(!state)return 0;tag=rrj_u16(rrj_at(m,input,2));if(tag>>5)return state;
    segment=rrj_read32(m,0x8005B2F8);count=rrj_read32(m,segment+48);object=rrj_read32(m,0x8005B3A0)+1096*tag;
    if(rrj_u16(rrj_at(m,object+172,2))<count || (material_byte(m,rrj_read32(m,object+1084)+1)&15)!=2)return state;
    {
        uint32_t distances[2]={0,0};
        for(i=0;rrj_s32(i)<rrj_s32(rrj_read32(m,segment+48));++i){
            uint32_t d=pause_asr(rrj_read32(m,object+324)-rrj_read32(m,rrj_read32(m,0x8005B268+4*i)+324),12);
            /* Original has exactly two local words; >2 players corrupt its stack. */
            if(i>=2)abort();if(rrj_s32(d)<0)d=0u-d;distances[i]=d;
        }
        if(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)==2 && rrj_s32(distances[0])<201)return state;
        i=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)==2?1u:0u;
        if(rrj_s32(distances[i])>=201)*(uint8_t *)rrj_at(m,object+928,1)=(uint8_t)(material_byte(m,object+928)|32);
    }
    return state;
}

uint32_t sub_8009DAF8(RRJMemory *m,uint32_t change)
{
    uint32_t value=rrj_read32(m,0x800D86F0)+(rrj_s32(change)>0?1u:0xFFFFFFFF),limit;
    rrj_write32(m,0x800D86F0,value);limit=rrj_read32(m,0x800D86F4);value=rrj_read32(m,0x800D86F0);
    if(rrj_s32(value)>=rrj_s32(limit))value=limit;rrj_write32(m,0x800D86F0,value);
    if(rrj_s32(value)<0)value=0;rrj_write32(m,0x800D86F0,value);return value;
}
uint32_t sub_8002820C(RRJMemory *m,uint32_t object)
{uint32_t result=rrj_read32(m,object+36)&0xC7FFFFFF;rrj_write32(m,object+36,result);return result;}
uint32_t sub_8002A738(RRJMemory *m,uint32_t object,uint32_t previous,uint32_t entry,uint32_t next)
{
    uint32_t kind=(rrj_read32(m,entry)>>6)&15,value,shift=0,mask=0,result;
    switch(kind){
    case 1:case 3:shift=25;mask=3;break;
    case 2:if(((rrj_read32(m,entry)>>14)&255)==2){shift=23;mask=3;}else{shift=19;mask=15;}break;
    case 4:shift=30;mask=3;break;
    case 5:(void)sub_8002820C(m,object);break;
    case 7:shift=19;mask=15;break;
    default:break;
    }
    if(mask){value=rrj_read32(m,object+36);rrj_write32(m,object+36,(value&~(mask<<shift))|(((((value>>shift)&mask)-1)&mask)<<shift));}
    value=rrj_read32(m,entry);*(uint8_t *)rrj_at(m,entry+60,1)=0;result=(value&0xFFFFC000)|63;rrj_write32(m,entry,result);
    if(previous){result=(rrj_read32(m,previous)&0xFFFFFFC0)|(next&63);rrj_write32(m,previous,result);}
    else *(uint8_t *)rrj_at(m,object+73,1)=(uint8_t)next;
    return result;
}
uint32_t sub_8002847C(RRJMemory *m,uint32_t object)
{
    uint32_t index=(material_byte(m,object+73)^128)-128,entry=0x800D39B0+112*index,result=0xFFFFFFFF,next;
    if(index==0xFFFFFFFF)return result;
    while(entry){
        next=pause_asr(rrj_read32(m,entry)<<26,26);(void)sub_8002A738(m,object,0,entry,next);result=next<<3;
        if(next==0xFFFFFFFF)entry=0;else{result=(result-next)<<4;entry=0x800D39B0+result;}
    }
    return result;
}

uint32_t sub_80010028(uint32_t numerator,uint32_t denominator)
{
    uint32_t divisor=denominator>>1,reciprocal=divisor?0x80000000u/divisor:0xFFFFFFFF;
    return (uint32_t)(((uint64_t)numerator*reciprocal)>>16);
}
uint32_t sub_8001FF3C(RRJMemory *m,uint32_t input)
{
    uint32_t negative=rrj_s32(input)<0,magnitude=negative?0u-input:input,result=1024,base,index=0,shift,q,address,lo,hi,product;
    if(rrj_s32(magnitude)<=65535){
        base=65528;shift=0;
        if(rrj_s32(magnitude)>=65528)index=52;
        else{
            base=32768;shift=15;
            while(rrj_s32(magnitude)>=rrj_s32(base)){--shift;base+=1u<<(shift&31);index+=4;}
            base-=1u<<(shift&31);shift-=2;
        }
        q=pause_asr(magnitude-base,shift&31);base+=q<<(shift&31);index+=q;address=0x800527E0+2*index;
        lo=rrj_u16(rrj_at(m,address,2));hi=rrj_u16(rrj_at(m,address+2,2));product=(magnitude-base)*(hi-lo);
        result=pause_asr(pause_asr(product,shift&31)+8+lo,4);
    }
    return negative?0u-result:result;
}
uint32_t sub_80020018(RRJMemory *m,uint32_t x,uint32_t y)
{
    uint32_t sign,octant,n,d,ratio,address,base,step,value;int64_t product;
    if(!x && !y)return 0;
    sign=pause_asr(y,31);octant=sign&2;if(rrj_s32(x)<0)octant|=4;y=(y+sign)^sign;
    sign=pause_asr(x,31);x=(x+sign)^sign;
    if(rrj_s32(y)<rrj_s32(x)){octant|=1;n=y;d=x;}else{n=x;d=y;}
    ratio=sub_80010028(rrj_s32(n)>0?n:0u-n,rrj_s32(d)>0?d:0u-d);
    if((rrj_s32(n)>0)!=(rrj_s32(d)>0))ratio=0u-ratio;
    address=0x8005285C+4*pause_asr(ratio,12);base=rrj_read32(m,address);step=((rrj_read32(m,address+4)-base)<<4)+8;
    product=(int64_t)(ratio&4095)*(int64_t)rrj_s32(step);value=pause_asr((uint32_t)((uint64_t)product>>16)+base,20);
    switch(octant){case 0:return value;case 1:return 1024-value;case 2:return 2048-value;case 3:return value+1024;case 4:return 0u-value;case 5:return value-1024;case 6:return value-2048;case 7:return 0xFFFFFC00-value;default:return 0;}
}

/* 2E698 typed return effect: MVMVA color row1 * IR vector, sf=0, then MAC1 ASR8. */
static uint32_t short_dot_ref(RRJMemory *m,uint32_t left,const uint16_t *local_left,uint32_t right)
{
    uint32_t i,sum=0,value;
    for(i=0;i<3;++i){
        value=local_left?local_left[i]:material_half(m,left+2*i);
        if(value&0x8000u)value|=0xffff0000u;
        sum+=(uint32_t)((int64_t)rrj_s32(value)*rrj_s32(material_half(m,right+2*i)));
    }
    return pause_asr(sum,8);
}
uint32_t sub_8002E698(RRJMemory *m,uint32_t left,uint32_t right)
{
    return short_dot_ref(m,left,NULL,right);
}
static int64_t scale_short_vector(RRJMemory *m,uint32_t scale,uint32_t direction,uint32_t output,uint32_t *local)
{
    unsigned i;int64_t result=0;
    for(i=0;i<3;++i){
        result=sub_8001FC90(rrj_s32(scale),rrj_s32(material_half(m,direction+2*i)<<4));
        if(local)local[i]=(uint32_t)result;else rrj_write32(m,output+4*i,(uint32_t)result);
    }
    return result;
}
int64_t sub_8002EE50(RRJMemory *m,uint32_t scale,uint32_t direction,uint32_t output)
{return scale_short_vector(m,scale,direction,output,NULL);}
int64_t rrj_scale_short_local(RRJMemory *m,uint32_t scale,uint32_t direction,uint32_t output[3])
{return scale_short_vector(m,scale,direction,0,output);}
static int64_t scale_long_vector(RRJMemory *m,uint32_t scale,uint32_t input,uint32_t output,uint32_t *local)
{
    unsigned i;int64_t result=0;
    for(i=0;i<3;++i){
        result=sub_8001FC90(rrj_s32(scale),rrj_s32(local?local[i]:rrj_read32(m,input+4*i)));
        if(local)local[i]=(uint32_t)result;else rrj_write32(m,output+4*i,(uint32_t)result);
    }
    return result;
}
int64_t sub_8002E810(RRJMemory *m,uint32_t scale,uint32_t input,uint32_t output)
{return scale_long_vector(m,scale,input,output,NULL);}
int64_t rrj_scale_long_local(RRJMemory *m,uint32_t scale,uint32_t values[3])
{return scale_long_vector(m,scale,0,0,values);}

/* Collision target response; return contract is the caller-observed low word. */
uint32_t sub_8002076C(RRJMemory *m,uint32_t object)
{
    uint32_t target=rrj_read32(m,object+832),tag,kind,record,value,other,dot;
    if(!target)return 3;
    tag=rrj_u16(rrj_at(m,target,2));kind=tag>>5;
    if(kind==0){
        record=rrj_read32(m,0x8005B3A0)+1096*(tag&31);
        value=rrj_read32(m,record+560)&0xFDFFFFFF;rrj_write32(m,record+560,value);return value;
    }
    if(kind<3)return 1;
    if(kind>4)return 4;
    if(kind==4){
        record=rrj_read32(m,0x800CD6D4)+596*(tag&31);
        value=(rrj_u16(rrj_at(m,rrj_read32(m,record)+14,2))&0xF80)>>7;
        if(value-3>=3)return 0;
        if(rrj_read32(m,record+592)&0x200)rrj_write32(m,record+556,0x10000);
        return 0x10000;
    }
    value=rrj_read32(m,object+564)&0x60000;if(value!=0x40000)return value;
    other=0x800CF660+512*(tag&31);dot=sub_8002E698(m,other+450,object+450);
    value=rrj_read32(m,object+576)+(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,other+480)),rrj_s32(dot));
    rrj_write32(m,object+576,value);if(rrj_s32(value)<146486)value=146486;
    rrj_write32(m,object+576,value);rrj_write32(m,object+480,value);
    return (uint32_t)sub_8002EE50(m,value,object+450,object+456);
}

uint32_t sub_8002090C(RRJMemory *m,uint32_t object)
{
    static const uint16_t first[]={780,776,772,768,764,760,756,752,748,728,724,720,716,712,704,696,700,688,676,680,684,692,656,660,672,644,636};
    static const uint16_t second[]={640,616,620,624,584,744,488,608,604,596,588,580,576,484};
    static const uint16_t halves[]={846,844,842,840,838,836,826};
    uint32_t config=rrj_read32(m,object+556),value,n,d,positive_n,positive_d,angle,x,y,z,linked;unsigned i;
    if(rrj_read32(m,object+832))(void)sub_8002076C(m,object);
    value=rrj_read32(m,object+560);
    for(i=0;i<sizeof(first)/sizeof(first[0]);++i)rrj_write32(m,object+first[i],0);
    rrj_write32(m,object+560,value&0xF8000000);
    rrj_write32(m,object+564,rrj_read32(m,object+564)&0xC0000000);
    rrj_write32(m,object+568,rrj_read32(m,object+568)&0xF7B00000);
    for(i=0;i<sizeof(second)/sizeof(second[0]);++i)rrj_write32(m,object+second[i],0);
    n=(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,config+12)),643207);positive_n=rrj_s32(n)>0;
    positive_d=rrj_s32(rrj_read32(m,config+16))>0;
    n=(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,config+12)),643207);d=rrj_read32(m,config+16);
    value=sub_80010028(positive_n?n:0u-n,positive_d?d:0u-d);
    if(positive_n!=positive_d)value=0u-value;rrj_write32(m,object+600,value);
    value=rrj_read32(m,config+188);angle=material_half(m,object+530);
    *(uint8_t *)rrj_at(m,object+849,1)=0;rrj_write32(m,object+828,0);rrj_write32(m,object+932,0);rrj_write32(m,object+708,0);
    for(i=0;i<sizeof(halves)/sizeof(halves[0]);++i)rrj_put16(rrj_at(m,object+halves[i],2),0);
    rrj_write32(m,object+592,value);value=sub_8001FF3C(m,angle<<4);
    x=material_half(m,object+518);rrj_put16(rrj_at(m,object+842,2),value);x*=157;
    y=material_half(m,object+524);rrj_write32(m,object+732,x);y*=157;
    z=material_half(m,object+530);rrj_write32(m,object+736,y);rrj_write32(m,object+740,z*157);
    value=sub_80020018(m,x,y);linked=rrj_read32(m,object+856);
    rrj_write32(m,object+668,0u-pause_asr(value*25736,8));
    if(linked && rrj_read32(m,object+1088))rrj_write32(m,linked+488,0);
    value=rrj_read32(m,object+668)+rrj_read32(m,object+636);rrj_write32(m,object+652,value);return value;
}

uint32_t sub_80012838(RRJMemory *m,uint32_t object,uint32_t child,uint32_t kind,uint32_t index)
{
    uint32_t entry=object+(index<<3);
    *(uint8_t *)rrj_at(m,child+72,1)=1;rrj_write32(m,entry+56,child);rrj_write32(m,entry+60,kind);rrj_write32(m,child+52,object);return 1;
}
uint32_t sub_80086AF8(RRJMemory *m,uint32_t object)
{
    uint32_t value=rrj_read32(m,rrj_read32(m,object+568)+480);
    rrj_write32(m,object+736,0);rrj_write32(m,object+720,0);rrj_write32(m,object+732,0);rrj_write32(m,object+708,0);rrj_write32(m,object+724,value);return value;
}
uint32_t sub_800235B0(RRJMemory *m,uint32_t index,uint32_t object)
{rrj_write32(m,0x80053478+(index<<7)+4,object);return 0x80053478;}
uint32_t sub_8001E100(RRJMemory *m,uint32_t destination,uint32_t value,uint32_t length)
{
    uint32_t address=destination,word=value|(value<<8)|(value<<16)|(value<<24);
    while(length){rrj_write32(m,address,word);length-=4;address+=4;}
    return destination;
}
uint32_t sub_800BCD10(RRJMemory *m,uint32_t object)
{
    uint32_t length=((material_byte(m,object+946)^128)-128)<<3;
    uint32_t result=sub_8001E100(m,object+956,0,length);
    *(uint8_t *)rrj_at(m,object+946,1)=0;return result;
}

uint32_t sub_80068D20(RRJMemory *m,uint32_t object,uint32_t expected,uint32_t index)
{
    uint32_t entry=object+(index<<3),child=rrj_read32(m,entry+56);
    if(child==expected){*(uint8_t *)rrj_at(m,child+72,1)=0;rrj_write32(m,entry+56,0);rrj_write32(m,entry+60,0);rrj_write32(m,child+52,0);}
    return child;
}
uint32_t sub_80095AEC(RRJMemory *m,uint32_t object)
{
    uint32_t slot=(material_byte(m,object+571)^128)-128,bit=1u<<(slot&31),flags=rrj_read32(m,0x8005AD50),attachment;
    if(!(flags&bit))return ~bit;
    rrj_write32(m,0x8005AD50,flags&~bit);slot=(material_byte(m,object+571)^128)-128;
    (void)sub_80068D20(m,object,0x800CF018+172*slot,0);
    attachment=rrj_read32(m,object+556);
    if(attachment){rrj_write32(m,attachment+36,0);rrj_write32(m,0x800CE178,rrj_read32(m,0x800CE178)-1);}
    if(material_byte(m,object+572)&128){
        slot=(material_byte(m,object+571)^128)-128;(void)sub_8002847C(m,0x800CF018+172*slot);
        *(uint8_t *)rrj_at(m,object+572,1)=(uint8_t)(material_byte(m,object+572)&127);
    }
    rrj_write32(m,object+556,0);*(uint8_t *)rrj_at(m,object+571,1)=255;return 0xFFFFFFFF;
}

uint32_t sub_80018440(RRJMemory *m,uint32_t index,uint32_t pending,RRJReverbCall reverb)
{
    uint32_t offset=index*132,base=rrj_read32(m,0x8005B42C),slot=base+offset;
    if(!base)return offset;
    if(pending){rrj_write32(m,slot+124,7);return 7;}
    (void)sub_8001F7EC(m,rrj_read32(m,slot+36),reverb);
    (void)sub_8001F7EC(m,rrj_read32(m,slot+40),reverb);
    rrj_write32(m,slot+84,0xFFFFFFFF);rrj_write32(m,slot+88,0xFFFFFFFF);rrj_write32(m,slot+124,0);return 0xFFFFFFFF;
}

static void direction_op(RRJMemory *m,uint32_t diagonal_address,uint32_t direction_address,uint32_t output)
{
    int32_t diagonal[3],direction[3];int64_t cross[3];uint32_t i;
    for(i=0;i<3;++i){diagonal[i]=rrj_s32(material_half(m,diagonal_address+2*i));direction[i]=rrj_s32(material_half(m,direction_address+2*i));}
    cross[0]=(int64_t)direction[2]*diagonal[1]-(int64_t)direction[1]*diagonal[2];
    cross[1]=(int64_t)direction[0]*diagonal[2]-(int64_t)direction[2]*diagonal[0];
    cross[2]=(int64_t)direction[1]*diagonal[0]-(int64_t)direction[0]*diagonal[1];
    for(i=0;i<3;++i){int64_t v=cross[i]/4096-(cross[i]<0 && cross[i]%4096!=0);if(v>32767)v=32767;if(v< -32768)v= -32768;rrj_put16(rrj_at(m,output+2*i,2),(uint32_t)v);}
}
/* OP(sf=1,lm=0) RAM/return effects; transient GTE registers are not exposed. */
uint32_t sub_8007EC30(RRJMemory *m,uint32_t object)
{
    uint32_t piece=rrj_read32(m,object+340),linked,values[9],i;
    for(i=0;i<3;++i)values[i]=rrj_u16(rrj_at(m,object+450+2*i,2));
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+528+2*i,2),values[i]);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+522+2*i,2),0u-rrj_u16(rrj_at(m,piece+8+2*i,2)));
    direction_op(m,object+522,object+528,object+516);
    for(i=0;i<9;++i)values[i]=rrj_u16(rrj_at(m,object+516+2*i,2));linked=rrj_read32(m,object+856);
    rrj_write32(m,object+676,0);rrj_write32(m,object+616,0);rrj_write32(m,object+636,0);rrj_write32(m,object+672,0);rrj_write32(m,object+488,0);
    for(i=0;i<9;++i)rrj_put16(rrj_at(m,object+432+2*i,2),values[i]);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+814+2*i,2),values[i]);
    if(linked)rrj_write32(m,linked+488,0);return values[0];
}

uint32_t sub_8003B4B0(RRJMemory *m,uint32_t table,uint32_t id)
{
    uint32_t count,i,address;
    if(material_half(m,0x800D6182)==0xFFFFFFFF || !table)return 0;
    count=rrj_read32(m,table+12);if(rrj_s32(count)<=0)return 0;
    for(i=0,address=table+20;i<count;++i,address+=16)if(rrj_read32(m,address)==id)return address;
    return 0;
}
uint32_t sub_8003B8F4(RRJMemory *m,uint32_t record)
{
    uint32_t table,id;if(!record)return 0;table=rrj_read32(m,record+256);if(!table)return 0;
    id=rrj_read32(m,record+188);if((id>>16)==1)return (id&65535)==rrj_read32(m,table);
    return sub_8003B4B0(m,table,id&65535)!=0;
}
uint32_t sub_80039AFC(RRJMemory *m,uint32_t index)
{
    uint32_t header,record;if(rrj_s32(index)<0)return 0;header=rrj_read32(m,0x8005B240);
    if(rrj_s32(index)>=rrj_s32(material_half(m,header+44)))return 0;
    record=rrj_read32(m,header+36)+104*index;return material_half(m,record)==index?record:0;
}
uint32_t sub_8003A37C(RRJMemory *m,uint32_t record,uint32_t id)
{
    uint32_t count,i,address;if(!record)return 0;count=material_half(m,record+2);
    if(rrj_s32(count)<=0 || rrj_s32(count)>=5)return 0;
    for(i=0,address=record+8;i<count;++i,address+=24)if(material_half(m,address+4)==id)return address;
    return 0;
}

static uint32_t direction_leading_sign(uint32_t value)
{
    uint32_t count=0;if(value&0x80000000)value=~value;
    while(count<32 && !(value&0x80000000)){++count;value<<=1;}return count;
}
uint32_t rrj_normalize_vector32(RRJMemory *m,uint32_t vector[3])
{
    uint32_t scaled[3],magnitude[3],maximum,bit,shift,sum=0,entry,expanded,scale,i;
    for(i=0;i<3;++i){uint32_t sign=pause_asr(vector[i],31);magnitude[i]=(vector[i]+sign)^sign;}
    maximum=rrj_s32(magnitude[1])<rrj_s32(magnitude[0])?magnitude[0]:magnitude[1];
    if(rrj_s32(magnitude[2])>=rrj_s32(maximum))maximum=magnitude[2];
    bit=maximum?31-direction_leading_sign(maximum):0;
    for(i=0;i<3;++i){
        scaled[i]=rrj_s32(bit)<21?vector[i]<<((20-bit)&31):pause_asr(vector[i],(bit-20)&31);
        sum+=(uint32_t)(((uint64_t)((int64_t)rrj_s32(scaled[i])*rrj_s32(scaled[i])))>>16);
    }
    if(rrj_s32(sum)<=0)return sum;
    shift=22-(direction_leading_sign(sum)&~1u);if(rrj_s32(shift)<=0)shift=0;
    entry=rrj_u16(rrj_at(m,rrj_read32(m,0x8005B560)+2*pause_asr(sum,shift),2));
    expanded=(entry>>5)<<(entry&31);scale=pause_asr(expanded,shift>>1);
    for(i=0;i<3;++i)vector[i]=(uint32_t)((uint64_t)((int64_t)rrj_s32(scaled[i])*rrj_s32(scale))>>16);
    return expanded;
}
uint32_t sub_8002E14C(RRJMemory *m,uint32_t address)
{
    uint32_t vector[3],result,i;for(i=0;i<3;++i)vector[i]=rrj_read32(m,address+4*i);
    result=rrj_normalize_vector32(m,vector);
    for(i=0;i<3;++i)rrj_write32(m,address+4*i,vector[i]);return result;
}

uint32_t sub_8002E468(RRJMemory *m,uint32_t address)
{
    uint32_t vector[3],i,sum,shift,entry,scale;uint64_t squared=0;
    for(i=0;i<3;++i){vector[i]=material_half(m,address+2*i);squared+=(uint64_t)((int64_t)rrj_s32(vector[i])*rrj_s32(vector[i]));}
    /* Original ADD traps above INT_MAX. CPU exception delivery is not ported. */
    if(squared>0x7FFFFFFF)abort();
    sum=(uint32_t)squared;shift=22-(direction_leading_sign(sum)&~1u);if(rrj_s32(shift)<=0)shift=0;
    entry=rrj_u16(rrj_at(m,rrj_read32(m,0x8005B560)+2*pause_asr(sum,shift),2));
    scale=pause_asr((entry>>5)<<(entry&31),shift>>1);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,address+2*i,2),pause_asr(vector[i]*scale,12));
    return sum;
}
/* 8004CF74: PsyQ table square root; signed LZCS and signed table loads. */
uint32_t sub_8004CF74(RRJMemory *m,uint32_t input)
{
    uint32_t leading=direction_leading_sign(input),even,shift,normalized,value;
    if(leading==32)return 0;
    even=leading&~1u;shift=pause_asr(19-even,1);
    normalized=even>=24?input<<((even-24)&31):pause_asr(input,24-even);
    value=material_half(m,0x800560CC+2*(normalized-64));
    return rrj_s32(shift)<0?value>>((0u-shift)&31):value<<(shift&31);
}

uint32_t sub_80039AA0(RRJMemory *m,uint32_t index)
{
    uint32_t header=rrj_read32(m,0x8005B240),relative=index-material_half(m,header+44),address;
    if(rrj_s32(index)<0 || rrj_s32(relative)>=rrj_s32(material_half(m,header+42)))return 0;
    address=rrj_read32(m,header+32)+(relative<<3);return material_half(m,address)==index?address:0;
}
/* References to track words may point to PSX RAM or native local records. */
typedef struct RRJTrackWords { uint32_t address; uint32_t *local; } RRJTrackWords;
static RRJTrackWords track_ram(uint32_t address)
{
    RRJTrackWords ref;ref.address=address;ref.local=NULL;return ref;
}
static RRJTrackWords track_local(uint32_t *words)
{
    RRJTrackWords ref;ref.address=0;ref.local=words;return ref;
}
static RRJTrackWords track_offset(RRJTrackWords ref,uint32_t bytes)
{
    if(ref.local)ref.local+=bytes/4;else ref.address+=bytes;return ref;
}
static uint32_t track_read(RRJMemory *m,RRJTrackWords ref,uint32_t offset)
{
    return ref.local?ref.local[offset/4]:rrj_read32(m,ref.address+offset);
}
static void track_write(RRJMemory *m,RRJTrackWords ref,uint32_t offset,uint32_t value)
{
    if(ref.local)ref.local[offset/4]=value;else rrj_write32(m,ref.address+offset,value);
}
static uint32_t track_exit_ref(RRJMemory *m,RRJTrackWords track,uint32_t direction)
{
    uint32_t root,segment,lookup,point,target;
    if(!track_read(m,track,0))return 1;
    if(material_half(m,track_read(m,track,4)+2)==1)return 0;
    segment=material_half(m,track_read(m,track,12));
    if(segment || rrj_s32(direction)>=0){
        if(rrj_s32(segment)<rrj_s32(material_half(m,track_read(m,track,8)+10)-1) || rrj_s32(direction)<=0)return 0;
    }
    root=track_read(m,track,0);
    if(!material_half(m,root+16)){
        lookup=sub_80039AA0(m,rrj_read32(m,root));if(!lookup)return 1;
        target=material_half(m,lookup+4);if(rrj_s32(direction)>0)target=material_half(m,lookup+2);
    }else{
        lookup=sub_80039AFC(m,rrj_read32(m,root));if(!lookup)return 1;
        point=sub_8003A37C(m,lookup,rrj_read32(m,track_read(m,track,4)+12));if(!point)return 1;
        if(rrj_read32(m,root+12)==1){
            if(rrj_s32(direction)>0)target=rrj_s32(material_half(m,point+2))>0?material_half(m,point):material_half(m,lookup+6);
            else target=rrj_s32(material_half(m,point+2))<=0?material_half(m,point):material_half(m,lookup+6);
        }else{
            if(rrj_s32(direction^material_half(m,point+2))<0)return 0;
            target=material_half(m,lookup+6);
        }
    }
    if(target==0xFFFFFFFF)return 1;lookup=sub_80039A08(m,target);if(!lookup)return 1;
    return rrj_read32(m,lookup+12)==0;
}

uint32_t sub_800394F0(RRJMemory *m,uint32_t track,uint32_t direction)
{
    return track_exit_ref(m,track_ram(track),direction);
}
uint32_t rrj_track_exit_local(RRJMemory *m,uint32_t track[8],uint32_t direction)
{
    return track_exit_ref(m,track_local(track),direction);
}

static uint32_t update_track_direction_ref(RRJMemory *m,uint32_t direction,const uint16_t *local_direction,RRJTrackWords track,RRJTrackWords output)
{
    uint32_t record=track_read(m,track,4),dot,prior,sign,magnitude,value,progress,root,segment,last,limit;
    value=((uint32_t)rrj_u16(rrj_at(m,record+2,2))<<16)|rrj_u16(rrj_at(m,record+12,2));track_write(m,output,0,value);
    dot=short_dot_ref(m,direction,local_direction,track_read(m,track,12)+14);prior=track_read(m,output,4);sign=pause_asr(prior,31);magnitude=(prior+sign)^sign;
    if(rrj_s32(dot)>=0)value=(magnitude==5?3u:1u)+(rrj_s32(prior)<1);
    else value=(magnitude==5?0xFFFFFFFDu:0xFFFFFFFFu)-(rrj_s32(prior)>=0);
    track_write(m,output,4,value);
    progress=rrj_read32(m,track_read(m,track,12)+40)+track_read(m,track,20);track_write(m,output,8,progress);
    if(material_half(m,track_read(m,track,4)+2))return 1;
    root=track_read(m,track,0);
    if(material_half(m,root+16)==1 && !rrj_read32(m,root+12)){
        value=rrj_s32(progress)<0?0:progress;track_write(m,output,8,value);record=track_read(m,track,4);
        if(rrj_read32(m,record+24)){limit=rrj_read32(m,record+28);if(rrj_s32(limit)<rrj_s32(value))track_write(m,output,8,limit);}
    }
    segment=material_half(m,track_read(m,track,12));
    if(segment){last=material_half(m,track_read(m,track,8)+10)-1;if(segment!=last)return last;}
    value=track_exit_ref(m,track,track_read(m,output,4));if(!value)return 0;
    record=track_read(m,track,4);value=track_read(m,output,8);limit=rrj_read32(m,record+28);
    if(rrj_s32(limit)<rrj_s32(value))value=limit;track_write(m,output,8,value);
    if(rrj_s32(value)<0)value=0;track_write(m,output,8,value);return value;
}

uint32_t sub_8003662C(RRJMemory *m,uint32_t direction,uint32_t track,uint32_t output)
{
    return update_track_direction_ref(m,direction,NULL,track_ram(track),track_ram(output));
}
uint32_t rrj_update_track_direction_local(RRJMemory *m,const uint16_t direction[3],uint32_t track[8],uint32_t output[3])
{
    return update_track_direction_ref(m,0,direction,track_local(track),track_local(output));
}

uint32_t sub_80096564(RRJMemory *m,uint32_t object)
{
    uint32_t flip=0,selected,secondary,id,dot,record,route,lookup,point,vector[3],i;
    if(sub_8003B8F4(m,object+172)){
        secondary=rrj_read32(m,object+852);selected=rrj_read32(m,secondary+604)>=3?secondary:object;
        id=rrj_read32(m,selected+360);
        if(!(id>>16)){
            dot=sub_8002E698(m,rrj_read32(m,selected+340)+14,object+528);
            record=sub_8003B4B0(m,rrj_read32(m,selected+428),rrj_u16(rrj_at(m,selected+360,2)));
            if(record && rrj_s32(rrj_read32(m,record+4))<0 && rrj_s32(dot)>0)flip=1;
        }else{
            route=sub_8003F3B4(m,id&65535);lookup=sub_80039AFC(m,rrj_read32(m,rrj_read32(m,selected+328)));
            if(route && lookup){
                point=sub_8003A37C(m,lookup,rrj_read32(m,route+84));
                if(point){
                    for(i=0;i<3;++i)vector[i]=rrj_read32(m,point+12+4*i)-rrj_read32(m,selected+184+4*i);
                    (void)rrj_normalize_vector32(m,vector);
                    for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+528+2*i,2),pause_asr(vector[i],4));
                    direction_op(m,object+522,object+528,object+516);
                    (void)sub_8002E468(m,object+516);
                    direction_op(m,object+516,object+522,object+528);
                }
            }
        }
    }else if(rrj_read32(m,object+568)&0x400000)flip=1;
    if(flip){
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+528+2*i,2),0u-rrj_u16(rrj_at(m,object+528+2*i,2)));
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+516+2*i,2),0u-rrj_u16(rrj_at(m,object+516+2*i,2)));
    }
    return sub_8003662C(m,object+528,object+328,object+360);
}

/* Overlay800C2FF4: commit state class, frame stamp and animation flags. */
uint32_t sub_800C2FF4(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags)
{
    uint32_t kind,state,global,value,stamp;
    if((rrj_u16(rrj_at(m,object+172,2))>>5)!=1){
        value=rrj_read32(m,object+552);
        rrj_put16(rrj_at(m,object+544,2),type);
        rrj_write32(m,object+552,((value|0x20000000u)&0xffff00ffu)|0x300u);
        return 0xffff00ffu;
    }
    kind=rrj_u16(rrj_at(m,0x800541D4+8*(type&65535u)+2,2));
    state=kind==0?0u:kind<5?1u:kind==5?2u:kind==6?3u:4u;
    global=rrj_read32(m,0x8005B2F8);
    value=rrj_read32(m,object+552);
    rrj_write32(m,object+604,state);
    rrj_put16(rrj_at(m,object+544,2),type);
    stamp=rrj_read32(m,global+16);
    value|=0x02000000u;
    rrj_write32(m,object+552,value);
    rrj_write32(m,object+548,stamp);
    value=(state-2u)<2u?value|0x20000000u:value&0xdfffffffu;
    rrj_write32(m,object+552,value);
    value=rrj_read32(m,object+552);
    value=(flags&0x100u)?value|0x08000000u:value&0xf7ffffffu;
    rrj_write32(m,object+552,value);
    *(uint8_t *)rrj_at(m,object+546,1)=0;
    return value;
}
/* Overlay8005BE0C: clear animation flag bit1, return the stored word. */
uint32_t sub_8005BE0C(RRJMemory *m,uint32_t animation)
{
    uint32_t value=rrj_read32(m,animation+36)&0xfffffffdu;
    rrj_write32(m,animation+36,value);
    return value;
}

/* Resident80012858: bind animation set, preserving pointer reloads. */
uint32_t sub_80012858(RRJMemory *m,uint32_t object,uint32_t set)
{
    uint32_t header=rrj_read32(m,object),animation;
    rrj_write32(m,object+40,set);
    rrj_put16(rrj_at(m,object+48,2),0);
    *(uint8_t *)rrj_at(m,header+9,1)|=4;
    animation=rrj_read32(m,object+4);
    *(uint8_t *)rrj_at(m,animation+1,1)=5;
    return 5;
}
/* Overlay8005BEF4: test whether the animation transition may proceed. */
uint32_t sub_8005BEF4(RRJMemory *m,uint32_t object)
{
    uint32_t animation=rrj_read32(m,object+4);
    if(*(uint8_t *)rrj_at(m,animation+1,1)==5)return 1;
    if(rrj_read32(m,object+12)==1 && *(uint8_t *)rrj_at(m,animation+13,1)==5)return 1;
    if(*(uint8_t *)rrj_at(m,animation+1,1)>=2)return 0;
    if(*(uint8_t *)rrj_at(m,animation+13,1)==3)return 0;
    return rrj_read32(m,object+12)==0;
}

/* Overlay8005BE58: animation completion, signed frame comparison. */
uint32_t sub_8005BE58(RRJMemory *m,uint32_t object)
{
    uint32_t animation,record,type;
    if(!(rrj_read32(m,object+36)&2u))return 1;
    record=12u*rrj_read32(m,object+12);
    animation=rrj_read32(m,object+4);record+=animation;
    type=*(uint8_t *)rrj_at(m,record+1,1);
    if(type==5)return 1;
    if(type!=1)return 0;
    if(*(uint8_t *)rrj_at(m,animation+1,1)==3)return 0;
    if(*(uint8_t *)rrj_at(m,animation+13,1)==3)return 0;
    return rrj_s32(rrj_read32(m,object+16))>=((int32_t)(rrj_u16(rrj_at(m,record+6,2))^32768u)-32768);
}
/* Overlay8005C418: resolve jump/advance animation records with original bound. */
uint32_t sub_8005C418(RRJMemory *m,uint32_t object)
{
    uint32_t animation=rrj_read32(m,object+4),index=rrj_read32(m,object+12);
    uint32_t count=rrj_read32(m,object+8),attempt=index,done=0;
    for(;;){
        uint32_t record=animation+12u*index;
        uint32_t type=*(uint8_t *)rrj_at(m,record+1,1),prior;
        switch(type){
        case 0:case 1:case 3:done=1;break;
        case 2:index=(uint32_t)((int32_t)(rrj_u16(rrj_at(m,record+4,2))^32768u)-32768);break;
        case 4:++index;if(index==count)index=0;break;
        case 5:done=1;rrj_write32(m,object+36,rrj_read32(m,object+36)&0xfffffffdu);break;
        default:break;
        }
        prior=attempt++;
        if(rrj_s32(count)<rrj_s32(prior)){
            rrj_write32(m,object+36,rrj_read32(m,object+36)&0xfffffffdu);
            return index;
        }
        if(done)return index;
    }
}

/* Overlay8005E2BC: initialize raw/compressed animation channel caches. */
uint32_t sub_8005E2BC(RRJMemory *m,uint32_t object,uint32_t output)
{
    uint32_t dest=output+4,source=rrj_read32(m,object+44),data=source+24;
    uint32_t remaining,stride,channels,i,p,value,result;
    rrj_put16(rrj_at(m,output+2,2),4u**(uint8_t *)rrj_at(m,source+15,1)+3);
    remaining=rrj_read32(m,source+4);
    stride=2u*rrj_u16(rrj_at(m,source+16,2));
    channels=4u**(uint8_t *)rrj_at(m,source+15,1);
    rrj_put16(rrj_at(m,output,2),0xffff);
    remaining-=24;
    if(*(uint8_t *)rrj_at(m,source+13,1)&1){
        p=output+10;
        for(i=0;i<3;++i){
            remaining-=stride;
            *(uint8_t *)rrj_at(m,p+14,1)&=0xfd;
            rrj_write32(m,dest,data);
            value=rrj_u16(rrj_at(m,data,2));dest+=24;
            rrj_put16(rrj_at(m,p-2,2),value);rrj_put16(rrj_at(m,p,2),value);
            data+=2u*rrj_u16(rrj_at(m,source+16,2));p+=24;
        }
    }else{
        p=output+8;
        for(i=0;i<3;++i){
            dest+=24;value=*(uint8_t *)rrj_at(m,p+16,1);
            rrj_put16(rrj_at(m,p+2,2),0);rrj_put16(rrj_at(m,p,2),0);
            *(uint8_t *)rrj_at(m,p+16,1)=(uint8_t)((value&0xfc)|2);p+=24;
        }
    }
    result=rrj_s32(remaining)<3;
    if(*(uint8_t *)rrj_at(m,source+13,1)&2){
        if(result)return 0x800d0000;
        p=dest+6;
        do{
            value=*(uint8_t *)rrj_at(m,p+14,1)|2;
            *(uint8_t *)rrj_at(m,p+14,1)=(uint8_t)value;
            value=(value&0xfe)|(rrj_u16(rrj_at(m,data+2,2))>>15);
            *(uint8_t *)rrj_at(m,p+14,1)=(uint8_t)value;
            value=rrj_u16(rrj_at(m,data+2,2))&0x7fff;
            rrj_write32(m,p+2,value);
            if(value&0x4000)rrj_write32(m,p+2,value|0xffff8000);
            if(*(uint8_t *)rrj_at(m,p+14,1)&1){
                uint32_t product,bits,table;
                value=rrj_u16(rrj_at(m,data+4,2))&0x7fff;
                rrj_put16(rrj_at(m,p+10,2),value);
                product=rrj_read32(m,p+2)*value;
                bits=(rrj_u16(rrj_at(m,data+6,2))&0x7fff)>>8;
                *(uint8_t *)rrj_at(m,p+16,1)=(uint8_t)bits;
                rrj_put16(rrj_at(m,p-2,2),pause_asr(product,9));
                rrj_write32(m,dest,data+6);
                *(uint8_t *)rrj_at(m,p+17,1)=8;
                value=*(uint8_t *)rrj_at(m,p+14,1);bits=*(uint8_t *)rrj_at(m,p+16,1);
                rrj_write32(m,p+6,0);*(uint8_t *)rrj_at(m,p+12,1)=0;
                *(uint8_t *)rrj_at(m,p+14,1)=(uint8_t)(value&0xfb);
                table=rrj_read32(m,0x800CC654+4*bits);
                value=*(uint8_t *)rrj_at(m,table,1);
                *(uint8_t *)rrj_at(m,p+13,1)=(uint8_t)((value-1)&((1u<<(bits&31))-1));
            }else{
                value=rrj_read32(m,p+2)<<6;
                rrj_put16(rrj_at(m,p-2,2),value);rrj_put16(rrj_at(m,p,2),value);
            }
            value=rrj_u16(rrj_at(m,p-2,2));dest+=24;rrj_put16(rrj_at(m,p,2),value);
            value=pause_asr(rrj_u16(rrj_at(m,data,2))<<16,16);p+=24;
            remaining-=2u*(value+1);data+=2u*(value+1);
            result=rrj_s32(remaining)<3;
        }while(!result);
    }else if(channels){
        p=dest+6;
        for(i=0;i<channels;++i){
            *(uint8_t *)rrj_at(m,p+14,1)&=0xfd;
            rrj_write32(m,dest,data);value=rrj_u16(rrj_at(m,data,2));dest+=24;
            rrj_put16(rrj_at(m,p-2,2),value);rrj_put16(rrj_at(m,p,2),value);
            data+=2u*rrj_u16(rrj_at(m,source+16,2));p+=24;
        }
        result=0;
    }
    return result;
}
uint32_t sub_8005C39C(RRJMemory *m,uint32_t object,uint32_t id)
{
    uint32_t cached=rrj_read32(m,object+44);
    id&=255;
    if(!cached || *(uint8_t *)rrj_at(m,cached+14,1)!=id || rrj_u16(rrj_at(m,object+48,2))!=65535){
        uint32_t table=rrj_read32(m,rrj_read32(m,object+40)+4);
        rrj_write32(m,object+44,rrj_read32(m,table+4*id));
        (void)sub_8005E2BC(m,object,object+48);
    }
    return rrj_read32(m,object+44);
}

/* Overlay800714FC: Q14 quaternion product, wrapped32 sums before ASR14. */
uint32_t rrj_pose_quaternion_product(const uint16_t left[4],const uint16_t right[4],uint16_t output[4])
{
    uint32_t a[4],b[4],r[4],i;
    for(i=0;i<4;++i){a[i]=pause_asr((uint32_t)left[i]<<16,16);b[i]=pause_asr((uint32_t)right[i]<<16,16);}
    r[3]=a[3]*b[3]-a[0]*b[0]-a[1]*b[1]-a[2]*b[2];
    r[0]=a[3]*b[0]+a[0]*b[3]+a[1]*b[2]-a[2]*b[1];
    r[1]=a[3]*b[1]+a[1]*b[3]+a[2]*b[0]-a[0]*b[2];
    r[2]=a[3]*b[2]+a[2]*b[3]+a[0]*b[1]-a[1]*b[0];
    for(i=0;i<4;++i)output[i]=(uint16_t)pause_asr(r[i],14);
    return output[3];
}
uint32_t sub_800714FC(RRJMemory *m,uint32_t left,uint32_t right,uint32_t output)
{
    uint16_t a[4],b[4],result[4];uint32_t i,value;
    for(i=0;i<4;++i){a[i]=rrj_u16(rrj_at(m,left+2*i,2));b[i]=rrj_u16(rrj_at(m,right+2*i,2));}
    value=rrj_pose_quaternion_product(a,b,result);
    for(i=0;i<4;++i)rrj_put16(rrj_at(m,output+2*i,2),result[i]);
    return value;
}

static uint32_t pose_joint_block(RRJMemory *m,uint32_t object,uint32_t source,uint32_t output,uint32_t mirror,uint32_t correction)
{
    uint32_t i=0,mask=0;
    if(!*(uint8_t *)rrj_at(m,source+15,1))return 0;
    do{
        uint32_t joint=mirror?*(uint8_t *)rrj_at(m,0x800CC1B0+i,1):i;
        uint32_t kind=(rrj_u16(rrj_at(m,rrj_read32(m,rrj_read32(m,object))+14,2))&0x78)>>3;
        uint16_t q[4],rotation[4],v[4];uint32_t k,dest=output+16*joint;
        mask|=1u<<(joint&31);
        for(k=0;k<4;++k)q[k]=rrj_u16(rrj_at(m,object+128+96*i+24*k,2));
        if((kind==1||kind==4)&&correction==1&&((rrj_read32(m,0x800CC1C4)>>(joint&31))&1)){
            for(k=0;k<4;++k)rotation[k]=rrj_u16(rrj_at(m,0x800CC1CE+8*joint+2*k,2));
            (void)rrj_pose_quaternion_product(rotation,q,q);
        }
        for(k=0;k<4;++k)v[k]=q[k];
        if(mirror){
            if((kind==1||kind==4)&&joint==0){
                v[0]=(uint16_t)(0u-q[2]);v[1]=(uint16_t)(0u-q[3]);
                v[2]=(uint16_t)(0u-q[0]);v[3]=(uint16_t)(0u-q[1]);
            }else if(kind==1||kind==4||kind==5){
                v[0]=(uint16_t)(0u-q[0]);v[1]=(uint16_t)(0u-q[1]);
            }else{v[1]=(uint16_t)(0u-q[1]);v[2]=(uint16_t)(0u-q[2]);}
        }
        for(k=0;k<4;++k)rrj_put16(rrj_at(m,dest+2*k,2),v[k]);
        ++i;
    }while(i<*(uint8_t *)rrj_at(m,source+15,1));
    return mask;
}
/* Overlay8005CB70: capture two corrected/mirrored poses, including cache switch. */
uint32_t sub_8005CB70(RRJMemory *m,uint32_t object,uint32_t record,uint32_t output)
{
    uint32_t parent,flags,id,correction=0,xoffset=0,yoffset=0,source=0,mirror,masks[2],part,k;
    uint16_t translation[3];
    rrj_write32(m,output+4,10);
    parent=rrj_read32(m,object);flags=rrj_read32(m,parent+36);
    id=*(uint8_t *)rrj_at(m,record+28,1);
    if((flags&0x40000)&&rrj_read32(m,parent+52)){
        xoffset=1148;yoffset=rrj_s32(rrj_read32(m,parent+76))>65535?10u:140u;
    }else{
        parent=rrj_read32(m,object);
        if(((rrj_u16(rrj_at(m,rrj_read32(m,parent)+14,2))&0x78)>>3)==1){
            uint32_t linked=rrj_read32(m,parent+52);
            if(linked)correction=(rrj_u16(rrj_at(m,rrj_read32(m,linked)+14,2))&0xf80)>>7;
        }
    }
    for(part=0;part<2;++part){
        if(part)source=sub_8005C39C(m,object,id);
        translation[0]=rrj_u16(rrj_at(m,object+56,2));
        if(!part)source=rrj_read32(m,object+44);
        mirror=*(uint8_t *)rrj_at(m,record+(part?26:14),1)&1;
        translation[1]=rrj_u16(rrj_at(m,object+80,2));translation[2]=rrj_u16(rrj_at(m,object+104,2));
        if(correction==1)for(k=0;k<3;++k)translation[k]=(uint16_t)(translation[k]+rrj_u16(rrj_at(m,0x800CC1C8+2*k,2)));
        rrj_put16(rrj_at(m,output+12+6*part,2),mirror?xoffset-translation[0]:translation[0]);
        rrj_put16(rrj_at(m,output+14+6*part,2),translation[1]+yoffset);
        rrj_put16(rrj_at(m,output+16+6*part,2),translation[2]);
        if(part)*(uint8_t *)rrj_at(m,output,1)=20;
        masks[part]=pose_joint_block(m,object,source,output+24+8*part,mirror,correction);
    }
    flags=masks[0]&masks[1]&rrj_read32(m,object+1760);
    rrj_write32(m,output+8,flags);return flags;
}

/* Overlay8005C8F4: initialize the selected animation record. */
uint32_t sub_8005C8F4(RRJMemory *m,uint32_t object)
{
    uint32_t record=rrj_read32(m,object+4)+12u*rrj_read32(m,object+12);
    uint32_t type=*(uint8_t *)rrj_at(m,record+1,1),value,source;
    if(type==3){
        value=*(uint8_t *)rrj_at(m,record+2,1);
        if(!(value&2)||!(value>>7)){
            (void)sub_8005CB70(m,object,record,object+1764);
            *(uint8_t *)rrj_at(m,record+2,1)|=2;
        }
        rrj_write32(m,object+16,0);
        rrj_write32(m,object+1756,rrj_read32(m,record+8));
        value=(*(uint8_t *)rrj_at(m,record+2,1)&8)?*(uint8_t *)rrj_at(m,record+3,1):rrj_read32(m,object+1768);
        if(!value)value=10;
        rrj_write32(m,object+24,value);return value;
    }
    if(type!=0&&type!=1)return 3;
    source=sub_8005C39C(m,object,*(uint8_t *)rrj_at(m,record,1));
    if(type==0)rrj_write32(m,object+16,0);
    value=(*(uint8_t *)rrj_at(m,record+2,1)&8)?*(uint8_t *)rrj_at(m,record+3,1):rrj_u16(rrj_at(m,source+18,2));
    if(!value)value=10;
    rrj_write32(m,object+24,value);rrj_write32(m,object+28,0);
    if(type==0){
        rrj_write32(m,object+20,pause_asr(rrj_u16(rrj_at(m,record+4,2))<<16,16));
        value=rrj_read32(m,record+8);rrj_write32(m,object+1756,value);return value;
    }
    rrj_write32(m,object+1756,rrj_read32(m,record+8));
    value=rrj_u16(rrj_at(m,source+16,2));
    if(rrj_s32(pause_asr(rrj_u16(rrj_at(m,record+6,2))<<16,16))>=(int32_t)value)rrj_put16(rrj_at(m,record+6,2),value-1);
    value=pause_asr(rrj_u16(rrj_at(m,record+4,2))<<16,16);
    rrj_write32(m,object+16,value);return value;
}
/* Overlay8005BD74: reset and activate using full native record resolution/update. */
uint32_t sub_8005BD74(RRJMemory *m,uint32_t object)
{
    uint32_t value=rrj_read32(m,object+36),i;
    static const uint32_t offsets[]={12,16,20,28,32,1756};
    for(i=0;i<6;++i)rrj_write32(m,object+offsets[i],0);
    rrj_write32(m,object+24,10);rrj_write32(m,object+36,value&0xe5);
    value=sub_8005C418(m,object);rrj_write32(m,object+12,value);
    (void)sub_8005C8F4(m,object);
    value=rrj_read32(m,object+4);if(!value)return value;
    value=rrj_read32(m,object+8);if(!value)return value;
    value=(rrj_read32(m,object+36)&0xfffffffdu)|2u;
    rrj_write32(m,object+36,value);return value;
}

static void animation_command_tail(RRJMemory *m,uint32_t animation,uint32_t kind)
{
    uint32_t flags=*(uint8_t *)rrj_at(m,animation+2,1),payload=rrj_read32(m,animation+8);
    uint32_t id=*(uint8_t *)rrj_at(m,animation,1),speed=*(uint8_t *)rrj_at(m,animation+3,1);
    if(kind==2){rrj_put16(rrj_at(m,animation+4,2),0xffff);rrj_put16(rrj_at(m,animation+6,2),0);}
    *(uint8_t *)rrj_at(m,animation+13,1)=(uint8_t)kind;
    rrj_put16(rrj_at(m,animation+16,2),0);rrj_put16(rrj_at(m,animation+18,2),0);
    rrj_write32(m,animation+20,payload);
    *(uint8_t *)rrj_at(m,animation+14,1)=(uint8_t)flags;
    *(uint8_t *)rrj_at(m,animation+12,1)=(uint8_t)id;
    *(uint8_t *)rrj_at(m,animation+15,1)=(uint8_t)speed;
}
/* Overlay8005BF6C: finite animation command, five real arguments. */
uint32_t sub_8005BF6C(RRJMemory *m,uint32_t object,uint32_t id,uint32_t flags,uint32_t speed,uint32_t payload)
{
    uint32_t set=rrj_read32(m,object+40),animation=rrj_read32(m,object+4);
    uint32_t entry=rrj_read32(m,rrj_read32(m,set+4)+4u*id);
    uint32_t length=rrj_u16(rrj_at(m,entry+16,2));
    *(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)flags;
    *(uint8_t *)rrj_at(m,animation+1,1)=1;
    *(uint8_t *)rrj_at(m,animation,1)=(uint8_t)id;
    rrj_put16(rrj_at(m,animation+4,2),0);rrj_write32(m,animation+8,payload);
    rrj_put16(rrj_at(m,animation+6,2),length-1);
    *(uint8_t *)rrj_at(m,animation+3,1)=(uint8_t)((flags&8)?speed:0);
    animation_command_tail(m,animation,5);
    return sub_8005BD74(m,object);
}
/* Overlay8005C0B0: looping animation command, five real arguments. */
uint32_t sub_8005C0B0(RRJMemory *m,uint32_t object,uint32_t id,uint32_t flags,uint32_t speed,uint32_t payload)
{
    uint32_t animation=rrj_read32(m,object+4);
    *(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)flags;
    rrj_write32(m,animation+8,payload);
    *(uint8_t *)rrj_at(m,animation,1)=(uint8_t)id;
    *(uint8_t *)rrj_at(m,animation+1,1)=0;
    *(uint8_t *)rrj_at(m,animation+3,1)=(uint8_t)((flags&8)?speed:0);
    animation_command_tail(m,animation,2);
    return sub_8005BD74(m,object);
}

/* Overlay8005C018: explicit frame interval, seven real arguments. */
uint32_t sub_8005C018(RRJMemory *m,uint32_t object,uint32_t id,uint32_t flags,uint32_t start,uint32_t end,uint32_t speed,uint32_t payload)
{
    uint32_t animation=rrj_read32(m,object+4);
    *(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)flags;
    *(uint8_t *)rrj_at(m,animation,1)=(uint8_t)id;
    *(uint8_t *)rrj_at(m,animation+1,1)=1;
    rrj_put16(rrj_at(m,animation+4,2),start);rrj_put16(rrj_at(m,animation+6,2),end);
    rrj_write32(m,animation+8,payload);
    *(uint8_t *)rrj_at(m,animation+3,1)=(uint8_t)((flags&8)?speed:0);
    animation_command_tail(m,animation,5);return sub_8005BD74(m,object);
}
/* Overlay8005C140: queue or immediately activate a pose transition. */
uint32_t sub_8005C140(RRJMemory *m,uint32_t object,uint32_t target,uint32_t mode,uint32_t flags,uint32_t first_id,uint32_t extra,uint32_t speed,uint32_t payload)
{
    uint32_t animation=rrj_read32(m,object+4),new_bit=flags&1;
    uint32_t old_bit=*(uint8_t *)rrj_at(m,animation+2,1)&1;
    uint32_t old_id=*(uint8_t *)rrj_at(m,animation,1),frame,value,captured_speed;
    first_id&=255;
    if(mode==1){
        if(sub_8005BE58(m,object))mode=0;
        else if(!*(uint8_t *)rrj_at(m,animation+1,1))*(uint8_t *)rrj_at(m,animation+1,1)=1;
    }
    if(!mode)frame=rrj_read32(m,object+16);
    else if(mode==1){
        uint32_t source=rrj_read32(m,rrj_read32(m,rrj_read32(m,object+40)+4)+4*old_id);
        frame=pause_asr((rrj_u16(rrj_at(m,source+16,2))-1)<<16,16);animation+=12;
    }else{frame=0xffffffff;new_bit=0;}
    *(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)flags;
    if(flags&8){*(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)(flags|8);*(uint8_t *)rrj_at(m,animation+3,1)=(uint8_t)speed;}
    else *(uint8_t *)rrj_at(m,animation+3,1)=0;
    value=*(uint8_t *)rrj_at(m,animation+2,1);
    *(uint8_t *)rrj_at(m,animation,1)=(uint8_t)first_id;*(uint8_t *)rrj_at(m,animation+1,1)=3;
    rrj_put16(rrj_at(m,animation+4,2),5);rrj_put16(rrj_at(m,animation+6,2),0);rrj_write32(m,animation+8,0);
    *(uint8_t *)rrj_at(m,animation+12,1)=0;*(uint8_t *)rrj_at(m,animation+14,1)=(uint8_t)flags;*(uint8_t *)rrj_at(m,animation+13,1)=4;
    rrj_put16(rrj_at(m,animation+16,2),old_id);rrj_put16(rrj_at(m,animation+18,2),frame);rrj_write32(m,animation+20,0);
    *(uint8_t *)rrj_at(m,animation+26,1)=0;*(uint8_t *)rrj_at(m,animation+24,1)=0;
    *(uint8_t *)rrj_at(m,animation+2,1)=(uint8_t)((extra<<5)|(value&0xdf)|0x84);
    *(uint8_t *)rrj_at(m,animation+14,1)=(uint8_t)((flags&0xfe)|old_bit);
    if(flags&8)*(uint8_t *)rrj_at(m,animation+26,1)|=8;
    captured_speed=*(uint8_t *)rrj_at(m,animation+3,1);value=*(uint8_t *)rrj_at(m,animation+26,1);
    *(uint8_t *)rrj_at(m,animation+25,1)=4;rrj_put16(rrj_at(m,animation+28,2),target);
    rrj_put16(rrj_at(m,animation+30,2),0);*(uint8_t *)rrj_at(m,animation+36,1)=0;*(uint8_t *)rrj_at(m,animation+37,1)=5;
    rrj_write32(m,animation+32,payload);*(uint8_t *)rrj_at(m,animation+27,1)=(uint8_t)captured_speed;
    *(uint8_t *)rrj_at(m,animation+26,1)=(uint8_t)((value&0xfe)|new_bit);
    return mode?payload:sub_8005BD74(m,object);
}

/* Overlay800C37B0: choose queued intermediary states from byte tables. */
uint32_t sub_800C37B0(RRJMemory *m,uint32_t object,uint32_t target)
{
    uint32_t pair=0x800CCBAC,state=rrj_u16(rrj_at(m,object+544,2)),offset,entry,count,i;
    uint16_t sequence[257];
    while(*(uint8_t *)rrj_at(m,pair,1)<state)pair+=2;
    if(*(uint8_t *)rrj_at(m,pair,1)!=rrj_u16(rrj_at(m,object+544,2)))return 224;
    offset=*(uint8_t *)rrj_at(m,pair+1,1);entry=0x800CCBBC+offset+2;target&=65535;
    while(*(uint8_t *)rrj_at(m,entry,1)<target)++entry;
    if(*(uint8_t *)rrj_at(m,entry,1)!=target)return 224;
    sequence[0]=(uint16_t)target;i=0;
    while(i<*(uint8_t *)rrj_at(m,0x800CCBBC+offset,1)){
        ++i;sequence[i]=*(uint8_t *)rrj_at(m,0x800CCBBC+offset+i,1);
    }
    count=*(uint8_t *)rrj_at(m,0x800CCBBC+offset,1);
    sequence[count+1]=rrj_u16(rrj_at(m,object+544,2));
    count=*(uint8_t *)rrj_at(m,0x800CCBBC+offset,1);
    for(i=0;i<count;++i)rrj_put16(rrj_at(m,object+610+2*i,2),sequence[i]);
    rrj_put16(rrj_at(m,object+608,2),count);return sequence[count];
}

static uint32_t prepare_state_transition(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags_address,uint32_t *local_flags)
{
    uint32_t flags=local_flags?*local_flags:rrj_read32(m,flags_address),mode=flags&6,anim_flags=flags>>8;
    uint32_t first=flags&1,speed=(flags>>16)&255,old=rrj_u16(rrj_at(m,object+544,2));
    uint32_t animation=rrj_read32(m,object+540),actor=((rrj_u16(rrj_at(m,object+172,2))>>5)==1)?object:0;
    uint32_t gate=flags&24,route=224,allowed=0,old_entry,new_entry,value,payload,anim_id;
    type&=65535;
    if(!rrj_u16(rrj_at(m,object+320,2)))return 0;
    if(type==11||type==77||type==13||type==78||type==4){flags&=0xfffffffe;anim_flags|=64;}
    old_entry=0x800541D4+8*old;
    if(rrj_u16(rrj_at(m,old_entry+2,2))==2 && mode==4)mode=2;
    if(old==type&&(type==11||type==77||type==13||type==78||type==4))return 0;
    if(!actor||gate==16||old==224)allowed=1;
    else if(gate==8||gate==0){
        value=rrj_u16(rrj_at(m,0x800541D4+8*type+2,2));
        if(rrj_read32(m,old_entry+4)&(1u<<(value&31)))allowed=1;
        else if(!gate&&!(flags&32)){
            route=sub_800C37B0(m,actor,type)&65535;
            if(route!=224)allowed=1;
        }
    }
    if(!allowed)return 0;
    if(route!=224)type=route;
    else if(actor&&!(flags&32)&&rrj_u16(rrj_at(m,actor+608,2)))rrj_put16(rrj_at(m,actor+608,2),0);
    if(!old&&type!=4)return 0;
    value=((flags&64)<<7)|(rrj_read32(m,animation+36)&0xffffff7f);rrj_write32(m,animation+36,value);
    old_entry=0x800541D4+8*old;new_entry=0x800541D4+8*type;
    if((rrj_read32(m,old_entry)&15)!=(rrj_read32(m,new_entry)&15)){
        value=rrj_read32(m,new_entry)&15;
        (void)sub_80012858(m,animation,rrj_read32(m,0x800CE190+4*value));
        if(rrj_u16(rrj_at(m,old_entry+2,2))==5&&rrj_u16(rrj_at(m,new_entry+2,2))==6){
            uint32_t parent=rrj_read32(m,actor+596),slot=0;
            if(*(uint8_t *)rrj_at(m,actor+572,1)&32){parent=rrj_read32(m,parent+856);slot=1;}
            (void)sub_80068D20(m,parent,actor,slot);*(uint8_t *)rrj_at(m,actor+72,1)=3;
        }
    }
    value=rrj_read32(m,object+552)&0xefffffff;rrj_write32(m,object+552,value);
    if(mode){
        uint32_t queue_mode=1;
        if(mode==2){rrj_write32(m,object+552,value|0x10000000);queue_mode=0;}
        else if(!sub_8005BEF4(m,animation)){
            if(route!=224){value=rrj_u16(rrj_at(m,actor+608,2));rrj_put16(rrj_at(m,actor+608,2),value+1);rrj_put16(rrj_at(m,actor+610+2*value,2),type);}
            return 0;
        }
        anim_id=(rrj_read32(m,new_entry)>>4)&4095;
        value=rrj_read32(m,0x8005B3E4);payload=value?rrj_read32(m,value+4*type):0;
        (void)sub_8005C140(m,animation,anim_id,queue_mode,anim_flags&255,first==0,(flags>>7)&1,speed,payload);
    }else{
        anim_id=(rrj_read32(m,new_entry)>>4)&4095;
        value=rrj_read32(m,0x8005B3E4);payload=value?rrj_read32(m,value+4*type):0;
        if(first)(void)sub_8005C0B0(m,animation,anim_id,anim_flags&255,speed,payload);
        else if(flags&0x8000)(void)sub_8005C018(m,animation,anim_id,anim_flags&255,6,11,speed,payload);
        else (void)sub_8005BF6C(m,animation,anim_id,anim_flags&255,speed,payload);
    }
    if(local_flags)*local_flags=flags;else rrj_write32(m,flags_address,flags);
    if(route!=224){if(local_flags)*local_flags=flags|32;else rrj_write32(m,flags_address,flags|32);}
    return 1;
}
uint32_t sub_800C3E9C(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags_address)
{return prepare_state_transition(m,type,object,flags_address,NULL);}
uint32_t rrj_prepare_state_transition_local(RRJMemory *m,uint32_t type,uint32_t object,uint32_t *flags)
{return prepare_state_transition(m,type,object,0,flags);}

/* Overlay 800C2F84: select/restart the auxiliary rider animation.
 * The null-object path leaves MIPS v0 untouched; callers use this as void. */
void sub_800C2F84(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags,uint32_t speed)
{
    uint32_t animation=rrj_read32(m,object+556);
    if(!animation)return;
    if(rrj_u16(rrj_at(m,object+544,2))==(type&65535)){
        (void)sub_8005BD74(m,animation);
    }else if(((type-145)&65535)<41){
        (void)sub_8005BF6C(m,rrj_read32(m,object+556),(type&65535)-145,flags&255,speed,0);
    }
}

/* Resident 80012884: allocate a 2108-byte animation slot. */
uint32_t sub_80012884(RRJMemory *m,uint32_t pool,uint32_t object)
{
    uint32_t count=rrj_read32(m,pool+8),capacity=rrj_read32(m,pool+12),index=0,slot,flags,kind,value;
    if(count==capacity)return 0;
    if(rrj_s32(capacity)>0){
        slot=rrj_read32(m,pool);
        while(rrj_read32(m,slot+36)){
            ++index;slot+=2108;
            if(rrj_s32(index)>=rrj_s32(capacity))break;
        }
    }
    slot=rrj_read32(m,pool)+2108*index;flags=rrj_read32(m,slot+36);
    rrj_write32(m,slot,object);rrj_write32(m,slot+1760,0xffffffff);rrj_write32(m,slot+36,flags|5);
    kind=(rrj_u16(rrj_at(m,rrj_read32(m,object)+14,2))&120)>>3;
    if(kind==1||kind==4){
        value=rrj_read32(m,0x80052390+4*(kind==4));flags=rrj_read32(m,slot+36);
        rrj_write32(m,slot+1760,value);rrj_write32(m,slot+36,(flags&0xfffffffb)|4);
    }
    *(uint8_t *)rrj_at(m,rrj_read32(m,slot+4)+1,1)=5;
    count=rrj_read32(m,pool+8);rrj_write32(m,pool+8,count+1);return slot;
}
/* Resident 8001298C: switch model and rebind its joint records. */
uint32_t sub_8001298C(RRJMemory *m,uint32_t object,uint32_t id)
{
    uint32_t old=(uint32_t)(int32_t)(int8_t)*(uint8_t *)rrj_at(m,object+8,1),set,offset,model,value,count,i,source,destination;
    static const uint32_t zero_offsets[6]={6,8,10,14,16,18};
    static const uint32_t diagonal_offsets[3]={4,12,20};
    if(id==old)return old;
    set=rrj_read32(m,object+96);
    if(rrj_s32(id)>=*(uint8_t *)rrj_at(m,set+4,1))return old;
    set=rrj_read32(m,object+96);offset=12*id;
    *(uint8_t *)rrj_at(m,object+8,1)=(uint8_t)id;
    model=rrj_read32(m,rrj_read32(m,set+8)+offset);rrj_write32(m,object,model);
    value=rrj_read32(m,model+16);model=rrj_read32(m,object);rrj_write32(m,object+40,value);
    count=rrj_u16(rrj_at(m,model+24,2));
    for(i=0;i<count;++i){
        set=rrj_read32(m,object+96);source=rrj_read32(m,set+8);destination=rrj_read32(m,object+4)+24*i;
        source=rrj_read32(m,source+offset+4);value=rrj_read32(m,source+4*i);rrj_write32(m,destination,value);
    }
    model=rrj_read32(m,object);
    if(((rrj_u16(rrj_at(m,model+14,2))&120)>>3)==5&&(old==0||old==4)){
        for(i=0;i<6;++i)rrj_put16(rrj_at(m,rrj_read32(m,object+4)+zero_offsets[i],2),0);
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,rrj_read32(m,object+4)+diagonal_offsets[i],2),4096);
    }
    return old;
}

/* Resident counter mirror and effect allocation/linking, audited MIPS. */
uint32_t sub_80043F00(RRJMemory *m,uint32_t id)
{
    id&=65535;if(id>=3)return 0;
    return rrj_u16(rrj_at(m,rrj_read32(m,0x800549B8)+16*id,2));
}
uint32_t sub_8002705C(RRJMemory *m,uint32_t object,uint32_t mode)
{
    uint32_t value=sub_80043F00(m,0xf2000002)&255;
    rrj_put16(rrj_at(m,object+56,2),value<<4);
    if(!mode){rrj_put16(rrj_at(m,object+58,2),50);return 50;}
    if(rrj_s32(mode)<18){rrj_put16(rrj_at(m,object+58,2),120);return 120;}
    value=sub_80043F00(m,0xf2000002)&255;
    rrj_put16(rrj_at(m,object+58,2),(uint32_t)(int32_t)(int8_t)(((255*value)>>8)-127));return value;
}
uint32_t sub_80027178(RRJMemory *m)
{
    uint32_t slot=rrj_read32(m,0x800D8068),i;
    for(i=0;i<20;++i,slot+=112)if(!((rrj_read32(m,slot)>>6)&15))return i;
    return 0xffffffff;
}
uint32_t sub_800271CC(RRJMemory *m,uint32_t object,uint32_t id)
{
    uint32_t current=(uint32_t)(int32_t)(int8_t)*(uint8_t *)rrj_at(m,object+73,1),slot,value,next;
    if(current==0xffffffff){*(uint8_t *)rrj_at(m,object+73,1)=(uint8_t)id;return 0xfffffff8;}
    slot=0x800D39B0+112*current;value=rrj_read32(m,slot);
    while((value&63)!=63){
        next=value&63;if(next&32)next|=0xffffffc0;
        slot=0x800D39B0+112*next;value=rrj_read32(m,slot);
    }
    value=(rrj_read32(m,slot)&0xffffffc0)|(id&63);rrj_write32(m,slot,value);return value;
}
uint32_t sub_800273EC(RRJMemory *m,uint32_t object,uint32_t subtype,uint32_t duration,uint32_t kind,uint32_t flags)
{
    uint32_t identity,global,excluded,index,slot,value,stamp;
    if(flags&2){
        identity=rrj_u16(rrj_at(m,object+172,2));global=rrj_read32(m,0x8005B2F8);
        excluded=rrj_s32(identity&31)>=rrj_s32(rrj_read32(m,global+48));
        if((identity>>5)>=2||excluded)return excluded;
    }
    index=sub_80027178(m);if(index==0xffffffff)return index;
    slot=0x800D39B0+112*index;value=rrj_read32(m,slot);
    *(uint8_t *)rrj_at(m,slot+60,1)=(uint8_t)flags;
    value=(value&0xfffffc3f)|((kind&15)<<6);rrj_write32(m,slot,value);
    value=(((value&0xffc03fff)|((subtype&255)<<14))&0xffffc3ff)|0x400;
    global=rrj_read32(m,0x8005B2F8);rrj_write32(m,slot,value);stamp=rrj_read32(m,global+16);
    rrj_write32(m,slot+52,duration);rrj_write32(m,slot+48,stamp);(void)sub_8002705C(m,slot,120);
    rrj_write32(m,slot+36,0);rrj_write32(m,slot+44,0);rrj_write32(m,slot+40,0);
    *(uint8_t *)rrj_at(m,slot+61,1)=0;rrj_put16(rrj_at(m,slot+62,2),30);
    return sub_800271CC(m,object,index);
}

uint32_t sub_800CB84C(RRJMemory *m,uint32_t object)
{
    uint32_t id=(material_byte(m,object+567)^128)-128,mask,value;
    if(id==0xffffffff)return id;
    mask=1u<<(id&31);value=rrj_read32(m,0x8005AD50);
    if(!(value&mask))return ~mask;
    rrj_write32(m,0x8005AD50,value&~mask);id=(material_byte(m,object+567)^128)-128;
    return sub_80068D20(m,object,0x800CF018+172*id,0);
}
uint32_t sub_800CC0B0(RRJMemory *m,uint32_t object)
{
    uint32_t animation=rrj_read32(m,object+540),value;
    if(animation){rrj_write32(m,animation+36,0);value=rrj_read32(m,0x800CE178);rrj_write32(m,0x800CE178,value-1);rrj_write32(m,object+540,0);}
    value=material_byte(m,object+566);return value?value:sub_800CB84C(m,object);
}
/* Overlay8008C000: release object bookkeeping for kinds2..6. */
uint32_t sub_8008C000(RRJMemory *m,uint32_t identity,uint32_t kind)
{
    uint32_t index=rrj_u16(rrj_at(m,identity,2))&31,pool,stride,base_offset,active_offset,increment=0;
    uint32_t value,last,account,base,result,cell;
    if(kind<2||kind>6)return 0x80060000;
    switch(kind){
    case 2:
        value=rrj_u16(rrj_at(m,identity,2))&31;
        (void)sub_800CC0B0(m,rrj_read32(m,0x800D4B80)+572*value);
        pool=0x800D4B70;stride=572;base_offset=16;active_offset=172;break;
    case 3:
        if(!rrj_read32(m,identity+8)){
            value=0x800CF660+512*(rrj_u16(rrj_at(m,identity,2))&31);
            if(rrj_read32(m,value+36)&0x08000000){(void)sub_8002847C(m,value);(void)sub_8002820C(m,value);}
        }
        pool=0x800CF650;stride=512;base_offset=0;active_offset=172;break;
    case 4:pool=0x800CD6C8;stride=596;base_offset=12;active_offset=172;increment=596;break;
    case 5:pool=0x800CE598;stride=0u-452;base_offset=12;active_offset=172;increment=452;break;
    default:pool=0x800CD6A8;stride=280;base_offset=28;active_offset=0;break;
    }
    value=rrj_read32(m,pool);if(rrj_s32(value)>0)rrj_write32(m,pool,value-1);
    if(rrj_read32(m,pool+8)==index){
        do{
            last=rrj_read32(m,pool+8);account=rrj_read32(m,0x800D1814)+increment;--last;
            rrj_write32(m,pool+8,last);rrj_write32(m,0x800D1814,account);
            if(rrj_s32(last)<0)break;
            base=kind==3?pool+16:rrj_read32(m,pool+base_offset);
        }while(!rrj_u16(rrj_at(m,base+stride*last+active_offset,2)));
    }
    result=rrj_s32(index)<rrj_s32(rrj_read32(m,pool+4));if(result)rrj_write32(m,pool+4,index);
    if(kind==4||kind==5){
        base=rrj_read32(m,pool+12);cell=rrj_read32(m,base+stride*index+4);rrj_write32(m,cell,0);
        result=(cell-0x800D1664)*0xaaaaaaab;value=pause_asr(result,3);
        if(rrj_s32(value)<rrj_s32(rrj_read32(m,0x800D1660)))rrj_write32(m,0x800D1660,value);
    }
    rrj_put16(rrj_at(m,identity,2),0);return result;
}
uint32_t sub_8008CC94(RRJMemory *m)
{
    uint32_t best=0,distance=0,count=rrj_read32(m,rrj_read32(m,0x800CE4FC)),object=rrj_read32(m,0x800CE4F0),player,value,sign,animation;
    if(rrj_s32(count)>=0){
        player=rrj_read32(m,0x8005B38C);
        do{
            if(rrj_u16(rrj_at(m,object+172,2))){
                value=rrj_read32(m,player+324)-rrj_read32(m,object+324);sign=pause_asr(value,31);value=(value+sign)^sign;
                if(rrj_s32(distance)<rrj_s32(value)){distance=value;best=object;}
            }
            value=rrj_read32(m,0x800CE4F4);--count;object+=value;
        }while(rrj_s32(count)>=0);
    }
    if(!best)return 0;
    animation=rrj_read32(m,best+540);(void)sub_8008C000(m,best+172,2);return animation;
}

uint32_t sub_800958F0(RRJMemory *m,uint32_t object,uint32_t mode)
{
    uint32_t i,slot=0x800CF018,mask,value,parent,descriptor,kind,animation;
    if(rrj_read32(m,0x8005AD50)==255)return 0;
    for(i=0;i<8;++i,slot+=172){
        value=rrj_read32(m,0x8005AD50);mask=1u<<i;
        if(value&mask)continue;
        parent=rrj_read32(m,object+596);rrj_write32(m,0x8005AD50,value|mask);*(uint8_t *)rrj_at(m,object+571,1)=(uint8_t)i;
        descriptor=rrj_read32(m,parent+1084);kind=material_byte(m,descriptor+46);
        if(!kind||kind==4){
            if(rrj_read32(m,0x8005B254)&&rrj_u16(rrj_at(m,parent+172,2))<rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)&&rrj_read32(m,0x800CE178)==rrj_read32(m,0x800CE17C))
                (void)sub_8008CC94(m);
            animation=sub_80012884(m,0x800CE170,slot);rrj_write32(m,object+556,animation);
            if(!animation){
                value=(material_byte(m,object+571)^128)-128;mask=1u<<(value&31);
                rrj_write32(m,0x8005AD50,rrj_read32(m,0x8005AD50)&~mask);*(uint8_t *)rrj_at(m,object+571,1)=255;return 0;
            }
            (void)sub_80012858(m,animation,rrj_read32(m,0x800CE19C));
        }
        descriptor=rrj_read32(m,rrj_read32(m,object+596)+1084);kind=material_byte(m,descriptor+46);
        if(kind!=((material_byte(m,slot+8)^128)-128))(void)sub_8001298C(m,slot,kind);
        (void)sub_80012838(m,object,slot,mode?7:10,0);
        descriptor=rrj_read32(m,rrj_read32(m,object+596)+1084);
        if(material_byte(m,descriptor+47)){
            (void)sub_800273EC(m,slot,0,1500,6,0);*(uint8_t *)rrj_at(m,object+572,1)=(uint8_t)(material_byte(m,object+572)|128);
        }
        return 1;
    }
    return 0;
}
void sub_800BFC5C(RRJMemory *m,uint32_t object,uint32_t previous,uint32_t type,uint32_t flags)
{
    if(rrj_u16(rrj_at(m,0x800541D4+8*(previous&65535)+2,2))!=3){
        if(!(material_byte(m,rrj_read32(m,rrj_read32(m,object+596)+1084)+60)&128))return;
        (void)sub_800958F0(m,object,flags&256);
    }
    if(material_byte(m,rrj_read32(m,rrj_read32(m,object+596)+1084)+60)&128)
        sub_800C2F84(m,type&65535,object,(flags>>8)&255,(flags>>16)&255);
}
void sub_800C4454(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags)
{
    uint32_t kind,old,animation;type&=65535;
    kind=rrj_u16(rrj_at(m,0x800541D4+8*type+2,2));old=rrj_u16(rrj_at(m,object+544,2));
    if(kind==3){sub_800BFC5C(m,object,old,type,flags);return;}
    if(type)return;
    animation=rrj_read32(m,rrj_read32(m,object+596)+540);
    if(flags&0x8000)(void)sub_8005C018(m,animation,0,(flags>>8)&255,6,11,0,0);
    else (void)sub_8005BF6C(m,animation,0,(flags>>8)&255,0,0);
}

void sub_800BC8DC(RRJMemory *m,uint32_t object)
{
    uint32_t count=(material_byte(m,object+946)^128)-128,last,kind,descriptor,linked,target,global,index,mask,value;
    if(!count)return;
    last=object+956+8*(count-1);kind=rrj_u16(rrj_at(m,last,2));
    if(kind-10<7){descriptor=rrj_read32(m,object+1084);*(uint8_t *)rrj_at(m,descriptor+61,1)=(uint8_t)(material_byte(m,descriptor+61)&240);}
    if(rrj_read32(m,object+560)&0x08000000){
        linked=rrj_read32(m,object+852);rrj_write32(m,object+704,0);
        kind=rrj_u16(rrj_at(m,linked+544,2));
        if(rrj_u16(rrj_at(m,0x800541D4+8*kind+2,2))==3){
            index=material_byte(m,linked+569);target=rrj_u16(rrj_at(m,rrj_read32(m,0x8005AD4C)+12*index,2));
            (void)sub_800C4550(m,target,linked,2);
        }
    }
    (void)sub_8001E100(m,last,0,8);count=material_byte(m,object+946)-1;
    last=object+956+8*((((count&255)^128)-128)-1);*(uint8_t *)rrj_at(m,object+946,1)=(uint8_t)count;
    if(rrj_u16(rrj_at(m,last,2))!=16)return;
    global=rrj_read32(m,0x8005B2F8);index=rrj_u16(rrj_at(m,last+2,2));
    if(index>=rrj_read32(m,global+48))return;
    value=material_byte(m,global+4);if(value==36||value==44)return;
    mask=rrj_u16(rrj_at(m,0x800CCAC0+2*index,2));
    if(mask&&(mask&(0u-mask))!=mask)sub_800BC8DC(m,object);
}
uint32_t sub_800C1014(RRJMemory *m,uint32_t object)
{
    uint32_t parent=rrj_read32(m,object+596),count,last,global,index,value,animation;
    *(uint8_t *)rrj_at(m,object+569,1)=41;*(uint8_t *)rrj_at(m,object+570,1)=0;
    if(!(rrj_read32(m,parent+560)&0x08000000)&&!(material_byte(m,object+572)&64)){
        count=(material_byte(m,parent+946)^128)-128;last=parent+956+8*(count-1);
        if(rrj_u16(rrj_at(m,last,2))==16){
            global=rrj_read32(m,0x8005B2F8);index=rrj_u16(rrj_at(m,parent+172,2));
            if(index<rrj_read32(m,global+48)){
                value=rrj_u16(rrj_at(m,last+2,2));rrj_put16(rrj_at(m,0x800CCB6C+2*index,2),value);
                index=rrj_u16(rrj_at(m,parent+172,2));value=rrj_read32(m,global+16);rrj_write32(m,0x800CCB70+4*index,value);
            }
            sub_800BC8DC(m,parent);
        }
    }
    animation=rrj_read32(m,object+540);rrj_write32(m,animation+36,rrj_read32(m,animation+36)&0xffffff7f);
    value=material_byte(m,object+572)&253;*(uint8_t *)rrj_at(m,object+572,1)=(uint8_t)value;return value;
}
uint32_t sub_800BFD24(RRJMemory *m,uint32_t object,uint32_t previous,uint32_t type)
{
    (void)previous;
    if(rrj_u16(rrj_at(m,0x800541D4+8*(type&65535)+2,2))==3)return 3;
    (void)sub_80095AEC(m,object);return sub_800C1014(m,object);
}
uint32_t sub_800C4500(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags)
{
    uint32_t previous=rrj_u16(rrj_at(m,object+544,2));(void)flags;
    if(rrj_u16(rrj_at(m,0x800541D4+8*previous+2,2))!=3)return 3;
    return sub_800BFD24(m,object,previous,type&65535);
}
uint32_t sub_800C4550(RRJMemory *m,uint32_t type,uint32_t object,uint32_t flags)
{
    uint32_t result;type&=65535;
    result=rrj_prepare_state_transition_local(m,type,object,&flags);
    if(result==1){(void)sub_800C4500(m,type,object,flags);sub_800C4454(m,type,object,flags);(void)sub_800C2FF4(m,type,object,flags);}
    return result;
}

static uint8_t *queued_event_at(RRJMemory *m,uint32_t address,uint8_t *local,uint32_t offset,uint32_t bytes)
{return local?local+offset:(uint8_t *)rrj_at(m,address+offset,bytes);}
static uint32_t enqueue_actor_event(RRJMemory *m,uint32_t address,uint8_t *local,uint32_t mode,uint32_t object)
{
    uint32_t count,search,destination,first,second,global,value,linked,type;
    count=(material_byte(m,object+946)^128)-128;
    if(rrj_s32(count)>=16){*(uint8_t *)rrj_at(m,object+946,1)=1;(void)sub_8001E100(m,object+964,0,120);}
    count=(material_byte(m,object+946)^128)-128;search=material_byte(m,object+946);
    destination=object+956+8*count;
    if(count&&mode==1){
        value=destination-8;
        if(rrj_u16(rrj_at(m,value,2))==rrj_u16(queued_event_at(m,address,local,0,2))&&rrj_u16(rrj_at(m,value+2,2))==rrj_u16(queued_event_at(m,address,local,2,2)))return 1;
    }else if(count&&mode==2){
        value=object+956+8*(count-1);
        for(;;){
            if(rrj_u16(rrj_at(m,value,2))==rrj_u16(queued_event_at(m,address,local,0,2))&&rrj_u16(rrj_at(m,value+2,2))==rrj_u16(queued_event_at(m,address,local,2,2)))break;
            --search;count=((search&255)^128)-128;
            if(rrj_s32(count)<=0)break;
            value=object+956+8*(count-1);
        }
        count=((search&255)^128)-128;
        if(rrj_s32(count)>0){
            if(count==((material_byte(m,object+946)^128)-128))return 1;
            rrj_put16(queued_event_at(m,address,local,6,2),rrj_u16(rrj_at(m,value+6,2)));
            do{
                ++search;first=rrj_read32(m,value+8);second=rrj_read32(m,value+12);
                rrj_write32(m,value,first);rrj_write32(m,value+4,second);
                count=(material_byte(m,object+946)^128)-128;value+=8;
            }while(rrj_s32(((search&255)^128)-128)<rrj_s32(count));
            first=rrj_u32(queued_event_at(m,address,local,0,4));second=rrj_u32(queued_event_at(m,address,local,4,4));
            rrj_write32(m,value,first);rrj_write32(m,value+4,second);return 1;
        }
        count=(material_byte(m,object+946)^128)-128;destination=object+956+8*count;
    }
    *(uint8_t *)rrj_at(m,object+946,1)=(uint8_t)(material_byte(m,object+946)+1);
    global=rrj_read32(m,0x8005B2F8);
    first=rrj_u32(queued_event_at(m,address,local,0,4));second=rrj_u32(queued_event_at(m,address,local,4,4));
    rrj_write32(m,destination,first);rrj_write32(m,destination+4,second);
    rrj_put16(rrj_at(m,destination+4,2),0);rrj_put16(rrj_at(m,destination+6,2),0);
    value=((2180*rrj_read32(m,global+16)+0x8000)>>16)|0xc000;rrj_put16(rrj_at(m,destination+6,2),value);
    if(rrj_read32(m,object+560)&0x08000000){
        linked=rrj_read32(m,object+852);type=rrj_u16(rrj_at(m,linked+544,2));
        if(rrj_u16(rrj_at(m,0x800541D4+8*type+2,2))==3){
            value=material_byte(m,linked+569);type=rrj_u16(rrj_at(m,rrj_read32(m,0x8005AD4C)+12*value,2));
            (void)sub_800C4550(m,type,linked,2);
        }
    }
    return 1;
}
uint32_t sub_800BCA68(RRJMemory *m,uint32_t event,uint32_t mode,uint32_t object)
{return enqueue_actor_event(m,event,NULL,mode,object);}
uint32_t rrj_enqueue_actor_event_local(RRJMemory *m,uint8_t event[8],uint32_t mode,uint32_t object)
{return enqueue_actor_event(m,0,event,mode,object);}
uint32_t sub_800C3104(RRJMemory *m,uint32_t object,uint32_t inactive)
{
    uint32_t active=rrj_u16(rrj_at(m,object+320,2)),type,animation;
    rrj_write32(m,object+604,0);rrj_put16(rrj_at(m,object+608,2),0);
    if(active){
        type=rrj_u16(rrj_at(m,object+544,2));if(type==6||type==77)return 77;
        return sub_800C4550(m,(material_byte(m,object+572)&32)?77:6,object,16);
    }
    (void)sub_800C4500(m,224,object,0);animation=rrj_read32(m,object+540);
    if(animation)(void)sub_8005BE0C(m,animation);
    if(inactive){rrj_put16(rrj_at(m,object+544,2),224);return 224;}
    rrj_put16(rrj_at(m,object+544,2),73);rrj_write32(m,object+604,4);return 4;
}

/* Overlay800903F4: restore actor physics, attachments and player state. */
uint32_t sub_800903F4(RRJMemory *m,uint32_t object,uint32_t reset,RRJReverbCall reverb)
{
    uint32_t piece,x,y,z,secondary,sibling,other,value,flags,global,descriptor,player,index,i;
    uint16_t axes[6];uint8_t event[8]={0};
    if(reset){
        if(rrj_u16(rrj_at(m,object+320,2))){
            piece=rrj_read32(m,object+340);
            (void)sub_8002EAD8(m,piece+20,piece+14,rrj_read32(m,object+348),object+504);
            x=rrj_read32(m,object+504);y=rrj_read32(m,object+508);z=rrj_read32(m,object+512);piece=rrj_read32(m,object+340);
            rrj_write32(m,object+184,x);rrj_write32(m,object+188,y);rrj_write32(m,object+192,z);
            for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+450+2*i,2),rrj_u16(rrj_at(m,piece+14+2*i,2)));
            if(rrj_read32(m,object+568)&0x00400000){
                x=rrj_u16(rrj_at(m,object+450,2));rrj_write32(m,object+364,0xfffffffe);z=rrj_u16(rrj_at(m,object+454,2));
                rrj_put16(rrj_at(m,object+450,2),0u-x);y=rrj_u16(rrj_at(m,object+452,2));
                rrj_put16(rrj_at(m,object+454,2),0u-z);rrj_put16(rrj_at(m,object+452,2),0u-y);
            }else rrj_write32(m,object+364,2);
            (void)sub_8007EC30(m,object);
        }
        rrj_write32(m,object+344,0);
    }
    (void)sub_8002090C(m,object);
    flags=rrj_read32(m,object+568);secondary=rrj_read32(m,object+852);rrj_write32(m,object+568,flags|0x08000000);
    rrj_write32(m,secondary+552,rrj_read32(m,secondary+552)&0xbfe67fc7);
    secondary=rrj_read32(m,object+852);rrj_put16(rrj_at(m,secondary+320,2),rrj_u16(rrj_at(m,object+320,2)));
    secondary=rrj_read32(m,object+852);
    if(rrj_read32(m,secondary+556)){(void)sub_80095AEC(m,secondary);rrj_write32(m,rrj_read32(m,object+852)+556,0);}
    (void)sub_80012838(m,object,rrj_read32(m,object+852),2,0);
    secondary=rrj_read32(m,object+852);*(uint8_t *)rrj_at(m,object+72,1)=0;(void)sub_800C3104(m,secondary,1);
    secondary=rrj_read32(m,object+852);
    if((material_byte(m,secondary+572)&16)&&material_byte(m,rrj_read32(m,0x8005B2F8)+57)!=1){
        sibling=rrj_read32(m,object+856);other=rrj_read32(m,sibling+852);
        value=rrj_u16(rrj_at(m,object+320,2));flags=rrj_read32(m,other+552);rrj_put16(rrj_at(m,other+320,2),value);rrj_write32(m,other+552,flags&0xbfe67fc7);
        (void)sub_80012838(m,object,other,2,1);(void)sub_800C3104(m,other,1);rrj_write32(m,other+604,1);
        rrj_write32(m,rrj_read32(m,object+856)+720,0);
    }
    (void)sub_800BCD10(m,object);descriptor=rrj_read32(m,object+1084);
    value=(rrj_read32(m,descriptor+40)||material_byte(m,descriptor+39)>=248)?2:4;
    rrj_put16(event,value);rrj_put16(event+2,224);(void)rrj_enqueue_actor_event_local(m,event,1,object);
    if(rrj_u16(rrj_at(m,object+320,2))){
        (void)sub_80096564(m,object);
        for(i=0;i<3;++i)axes[i]=rrj_u16(rrj_at(m,object+528+2*i,2));
        for(i=0;i<3;++i)axes[i+3]=rrj_u16(rrj_at(m,object+516+2*i,2));
        secondary=rrj_read32(m,object+852);
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+450+2*i,2),axes[i]);
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,object+814+2*i,2),axes[i+3]);
        if((material_byte(m,secondary+572)&16)&&material_byte(m,rrj_read32(m,0x8005B2F8)+57)!=1){
            sibling=rrj_read32(m,object+856);value=rrj_read32(m,object+344);rrj_write32(m,sibling+344,value);
            flags=rrj_read32(m,object+364);sibling=rrj_read32(m,object+856);
            x=rrj_read32(m,object+304);y=rrj_read32(m,sibling+304);value=rrj_read32(m,sibling+344);
            rrj_write32(m,sibling+344,rrj_s32(flags)<0?value-(x+y):value+x+y);
            sibling=rrj_read32(m,object+856);value=rrj_read32(m,object+364);rrj_write32(m,sibling+364,value);
        }
    }
    global=rrj_read32(m,0x8005B2F8);index=rrj_u16(rrj_at(m,object+172,2));if(index>=rrj_read32(m,global+48))return 0;
    player=0x800CD898+1132*index;rrj_write32(m,player+548,rrj_read32(m,player+548)&0xf5ffff7f);
    if(material_byte(m,global+4)==44&&material_byte(m,global+57)==2){
        flags=rrj_read32(m,player+552);value=rrj_read32(m,player+776);rrj_write32(m,player+552,flags|16);
        if(rrj_s32(value)<5){rrj_write32(m,player+776,5);rrj_write32(m,player+792,0);}
        *(uint8_t *)rrj_at(m,rrj_read32(m,0x8005B2F8)+10,1)=2;
    }else rrj_write32(m,player+548,(rrj_read32(m,player+548)|6)&0xff807fff);
    (void)sub_80086AF8(m,player);(void)sub_80018440(m,rrj_u16(rrj_at(m,object+172,2)),0,reverb);
    return sub_800235B0(m,rrj_u16(rrj_at(m,object+172,2)),object);
}


/* 80093FE4: inactive actor cleanup, including actual activation when required. */
void sub_80093FE4(RRJMemory *m,uint32_t object,RRJReverbCall reverb)
{
    uint32_t global,secondary,flags,animation;
    uint32_t state;
    if(!object || rrj_u16(rrj_at(m,object+320,2)))return;
    global=rrj_read32(m,0x8005B2F8);
    if(rrj_u16(rrj_at(m,object+172,2))<rrj_read32(m,global+48) || (uint32_t)(material_byte(m,global+57)-1u)>=2u){
        (void)sub_8002090C(m,object);
        secondary=rrj_read32(m,object+852);
        if(rrj_read32(m,secondary+604)>=3u && rrj_u16(rrj_at(m,secondary+320,2)))
            rrj_write32(m,object+568,rrj_read32(m,object+568)|0x100u);
        else{
            rrj_put16(rrj_at(m,secondary+320,2),0);
            (void)sub_800903F4(m,object,1,reverb);
        }
    }
    flags=rrj_read32(m,object+568);animation=rrj_read32(m,object+540);
    rrj_write32(m,object+568,flags|0x08000000u);
    if(animation){
        rrj_write32(m,animation+36,0);
        rrj_write32(m,0x800CE178,rrj_read32(m,0x800CE178)-1u);
        rrj_write32(m,object+540,0);
    }
    global=rrj_read32(m,0x8005B2F8);
    if(rrj_u16(rrj_at(m,object+172,2))>=rrj_read32(m,global+48) && (material_byte(m,rrj_read32(m,object+1084)+1)&15u)==2u){
        state=material_byte(m,object+928);
        if(state&16u){
            *(uint8_t *)rrj_at(m,object+928,1)=(uint8_t)(state&0xEFu);
            rrj_write32(m,object+324,rrj_read32(m,0x8005B244)<<12);
            (void)sub_8009DAF8(m,0xFFFFFFFFu);
            if(rrj_read32(m,object+36)&0x08000000u){
                (void)sub_8002847C(m,object);
                (void)sub_8002820C(m,object);
            }
        }
        *(uint8_t *)rrj_at(m,object+928,1)=(uint8_t)(material_byte(m,object+928)&0xDFu);
    }
}

/* 800A0708: activate and place the actor at its directional track offset. */
uint32_t sub_800A0708(RRJMemory *m,uint32_t object,RRJReverbCall reverb)
{
    uint32_t global,value,record,offset=0,direction,piece,scale,x,y,z;
    (void)sub_800903F4(m,object,1,reverb);
    global=rrj_read32(m,0x8005B2F8);
    value=rrj_read32(m,0x80053138+4u*rrj_read32(m,global+60));
    record=rrj_read32(m,object+372);
    rrj_write32(m,object+576,value);rrj_write32(m,object+480,value);rrj_write32(m,object+924,value);
    if(record){
        direction=rrj_read32(m,object+364);offset=rrj_read32(m,record+80);
        if(rrj_s32(direction)>=0)offset=rrj_read32(m,record+208);
    }
    piece=rrj_read32(m,object+340);direction=rrj_read32(m,object+364);
    scale=offset-0x8000u+((0u-direction)&0x10000u);
    rrj_write32(m,object+344,scale);
    (void)sub_8002EAD8(m,object+184,piece+2,scale,object+184);
    x=rrj_read32(m,object+184);y=rrj_read32(m,object+188);z=rrj_read32(m,object+192);
    rrj_write32(m,object+504,x);rrj_write32(m,object+508,y);rrj_write32(m,object+512,z);
    return x;
}


/* 8003A468: rebuild track position using the loaded segment search. */
uint32_t sub_8003A468(RRJMemory *m,uint32_t position,uint32_t output)
{
    uint32_t segment=sub_80039DFC(m,0,position),route=0,count,i,word,base,subrecord,piece,delta,limit;
    if(!segment)return 0;
    if(!material_half(m,segment+16))route=rrj_read32(m,segment+44);
    else{
        count=material_half(m,segment+18);
        if(rrj_s32(count)>0){
            word=rrj_read32(m,position);base=rrj_read32(m,segment+44);
            for(i=0;rrj_s32(i)<rrj_s32(count);++i){
                route=base+32*i;
                if(material_half(m,route+2)==(word>>16) && rrj_read32(m,route+12)==(word&65535u))break;
            }
        }
        if(material_half(m,route+2))return 0;
    }
    subrecord=rrj_read32(m,segment+48)+28u*material_half(m,route+20);
    piece=rrj_read32(m,segment+52)+52u*material_half(m,subrecord+8);
    rrj_write32(m,output,segment);rrj_write32(m,output+4,route);rrj_write32(m,output+8,subrecord);rrj_write32(m,output+12,piece);
    rrj_write32(m,output+20,0);rrj_write32(m,output+16,0);rrj_write32(m,output+24,0);rrj_write32(m,output+28,0);
    delta=rrj_read32(m,position+8)-rrj_read32(m,route+24);
    if(rrj_s32(delta)<0)delta=0;
    limit=rrj_read32(m,route+28);if(rrj_s32(limit)<rrj_s32(delta))delta=limit;
    (void)sub_80037524(m,output,delta);return 1;
}


static uint32_t surface_magnitude(uint32_t value)
{uint32_t sign=pause_asr(value,31);return (sign+value)^sign;}
/* 8003E67C: classify lateral position against surface bands. */
uint32_t sub_8003E67C(RRJMemory *m,uint32_t surface,uint32_t position,uint32_t side,uint32_t output)
{
    uint32_t block=surface+(side==2?136u:8u),main_width=rrj_read32(m,block+8),width,value;
    uint32_t count=material_half(m,block+6),outer=rrj_read32(m,block+72);
    position=surface_magnitude(position);
    if(rrj_s32(count)>0 && rrj_s32(position)<rrj_s32(surface_magnitude(outer))){
        value=rrj_u16(rrj_at(m,block+82,2));rrj_put16(rrj_at(m,output,2),5);rrj_put16(rrj_at(m,output+2,2),value);return 0;
    }
    width=main_width;if(rrj_s32(material_half(m,block+2))>0)width=rrj_read32(m,block+24);
    if(rrj_s32(position)<rrj_s32(surface_magnitude(width))){
        rrj_put16(rrj_at(m,output+2,2),1);rrj_put16(rrj_at(m,output,2),4);return 0;
    }
    if(rrj_s32(position)<rrj_s32(surface_magnitude(main_width))){
        value=rrj_u16(rrj_at(m,block+18,2));rrj_put16(rrj_at(m,output,2),1);rrj_put16(rrj_at(m,output+2,2),value);return 0;
    }
    rrj_put16(rrj_at(m,output+2,2),0);rrj_put16(rrj_at(m,output,2),0);return 1;
}

/* 8003EF34: select/reuse a surface interval and update its lateral bands. */
uint32_t sub_8003EF34(RRJMemory *m,uint32_t record,uint32_t position,uint32_t output)
{
    uint32_t segment=rrj_read32(m,record),route=rrj_read32(m,record+4),subrecord=rrj_read32(m,record+8);
    uint32_t kind=material_half(m,route+2),piece=rrj_read32(m,record+12),count=0,interval,surface=0,changed=0,flags=0;
    uint32_t progress,last,lateral,start,end,index,base,i,p,value;
    if(!kind){count=material_half(m,subrecord+16);if(rrj_s32(count)<0)count=0;}
    interval=rrj_read32(m,output+4);
    index=material_half(m,subrecord+8)+material_half(m,subrecord+10);
    progress=rrj_read32(m,position+8);last=rrj_read32(m,segment+52)+52u*index-52u;
    kind=material_half(m,piece);lateral=rrj_read32(m,record+16);
    if((!kind && rrj_s32(progress)<rrj_s32(rrj_read32(m,piece+40))) ||
       (piece==last && rrj_s32(rrj_read32(m,piece+40)+rrj_read32(m,piece+32))<rrj_s32(progress))){
        start=rrj_read32(m,piece+40);end=start+rrj_read32(m,piece+32);progress=rrj_read32(m,position+8);
        if(rrj_s32(end)<rrj_s32(progress))progress=end;
        if(rrj_s32(start)>=rrj_s32(progress))progress=start;
    }
    if(!count || !rrj_read32(m,output))interval=0;
    if(interval){
        start=rrj_read32(m,interval+8);value=rrj_read32(m,position+8);
        if(rrj_s32(value)<rrj_s32(start) || rrj_s32(rrj_read32(m,interval+12))<rrj_s32(value) || rrj_s32(material_half(m,interval+4))<0)interval=0;
        else surface=rrj_read32(m,output);
    }
    if(rrj_s32(count)>0 && !interval){
        progress=pause_asr(progress,16);index=material_half(m,subrecord+18);base=rrj_read32(m,segment+56);
        for(i=0;rrj_s32(i)<rrj_s32(count);++i){
            p=base+16u*(index+i);
            if(rrj_s32(progress)>=rrj_s32(material_half(m,p+10)) && rrj_s32(material_half(m,p+14))>=rrj_s32(progress)){
                interval=p;value=material_half(m,p+4);surface=0;
                if(rrj_s32(value)>=0)surface=rrj_read32(m,segment+60)+264u*value;
                break;
            }
        }
        changed=1;
    }
    if(surface){
        rrj_write32(m,output+52,rrj_read32(m,surface+4));rrj_write32(m,output+40,rrj_read32(m,surface+144));
        rrj_put16(rrj_at(m,output+48,2),rrj_u16(rrj_at(m,surface+140,2)));
        rrj_write32(m,output+28,rrj_read32(m,surface+16));rrj_put16(rrj_at(m,output+36,2),rrj_u16(rrj_at(m,surface+12,2)));
        flags=sub_8003E67C(m,surface,lateral,rrj_s32(lateral)<0?1u:2u,output+20);
    }
    value=rrj_read32(m,output+16);
    rrj_write32(m,output,surface);rrj_write32(m,output+4,interval);rrj_write32(m,output+8,0);rrj_write32(m,output+12,0);
    rrj_write32(m,output+16,(value&0xfffffff0u)|flags);return changed;
}


/* 800B6AAC falls through800B6B04: each product is shifted before summation. */
static uint32_t junction_plane_distance(RRJMemory *m,uint32_t point,const uint16_t normal[3],uint32_t origin)
{
    uint32_t i,result=0;
    for(i=0;i<3;++i){
        uint32_t component=(uint32_t)(int32_t)(int16_t)normal[i]<<4;
        uint32_t delta=rrj_read32(m,point+4*i)-rrj_read32(m,origin+4*i);
        int64_t product=(int64_t)rrj_s32(component)*(int64_t)rrj_s32(delta);
        result+=(uint32_t)((uint64_t)product>>16);
    }
    return result;
}
uint32_t sub_800B6AAC(RRJMemory *m,uint32_t point,uint32_t normal,uint32_t origin)
{
    uint16_t values[3];uint32_t i;
    for(i=0;i<3;++i)values[i]=rrj_u16(rrj_at(m,normal+2*i,2));
    return junction_plane_distance(m,point,values,origin);
}

uint32_t rrj_find_junction_planes_local(RRJMemory *m,uint32_t point,uint32_t header,uint32_t *first,uint32_t *next,uint32_t hint)
{
    uint32_t count,current=0,following=0,result=0xffffffffu,i,entry,planes[4];uint16_t normal[3];
    count=material_half(m,header+2);
    if(rrj_s32(hint)>=0 && rrj_s32(hint)<rrj_s32(count)){
        result=hint;current=hint;if(rrj_s32(hint)<rrj_s32(count-1u))following=hint+1u;
    }else{
        for(i=0;rrj_s32(i)<rrj_s32(count);++i){
            /* Four entries fit the104-byte junction record. Larger input would
               overwrite the original local stack; it is outside this port's domain. */
            if(i>=4)abort();
            entry=header+8+24*i;
            normal[0]=(uint16_t)(0u-rrj_u16(rrj_at(m,entry+10,2)));
            normal[1]=rrj_u16(rrj_at(m,entry+8,2));normal[2]=rrj_u16(rrj_at(m,entry+6,2));
            planes[i]=junction_plane_distance(m,point,normal,entry+12);
            count=material_half(m,header+2);
        }
        count=material_half(m,header+2);
        for(current=0;rrj_s32(current)<rrj_s32(count);++current){
            following=rrj_s32(current)<rrj_s32(count-1u)?current+1u:0;
            ++result;
            if(rrj_s32(planes[current])<=0 && rrj_s32(planes[following])>0)break;
        }
    }
    if(rrj_s32(result)>=0){if(first)*first=header+24*current+8;if(next)*next=header+24*following+8;}
    return result;
}
uint32_t sub_8003EB58(RRJMemory *m,uint32_t point,uint32_t header,uint32_t first,uint32_t next,uint32_t hint)
{
    uint32_t a=0,b=0,result=rrj_find_junction_planes_local(m,point,header,&a,&b,hint);
    if(rrj_s32(result)>=0){if(first)rrj_write32(m,first,a);if(next)rrj_write32(m,next,b);}
    return result;
}


/* 8003F204: junction contact descriptor selected by plane index or pair. */
void sub_8003F204(RRJMemory *m,uint32_t object,uint32_t hint)
{
    uint32_t header,first=0,next=0,index,segment,count,entry,i,a,b,found=0;
    if(!object)return;
    header=sub_80039AFC(m,rrj_read32(m,rrj_read32(m,object+328)));
    if(!header)return;
    index=rrj_find_junction_planes_local(m,object+184,header,&first,&next,hint);
    if(rrj_s32(index)<0)return;
    segment=rrj_read32(m,object+328);count=material_half(m,segment+78);
    if(rrj_s32(count)>=4)count=3;
    entry=rrj_read32(m,segment+92);
    if(entry && rrj_s32(count)>0){
        for(i=0;rrj_s32(i)<rrj_s32(count);++i,entry+=20){
            if(material_byte(m,entry+2)!=1)continue;
            a=material_byte(m,entry+8);
            if(a==255){if(index==material_byte(m,entry+3))found=1;}
            else{
                b=material_half(m,first+4);
                if(a==b && material_byte(m,entry+9)==material_half(m,next+4))found=1;
                else if(material_byte(m,entry+9)==b && a==material_half(m,next+4))found=1;
            }
            if(found)break;
        }
    }
    if(!found){rrj_write32(m,object+496,0);return;}
    rrj_write32(m,object+496,object+500);
    *(uint8_t *)rrj_at(m,object+502,1)=(uint8_t)material_byte(m,entry);
    rrj_put16(rrj_at(m,object+500,2),rrj_u16(rrj_at(m,entry+4,2)));
    *(uint8_t *)rrj_at(m,object+503,1)=(uint8_t)material_byte(m,entry+6);
}
uint32_t sub_8003F1F0(RRJMemory *m,uint32_t object)
{uint32_t result=rrj_read32(m,object+340)+44u;rrj_write32(m,object+492,result);return result;}
/* Both contact update branches are used for effects, with no caller return use. */
void sub_8003DF54(RRJMemory *m,uint32_t object,uint32_t reset,uint32_t hint)
{
    uint32_t previous;
    if(reset==1)(void)sub_8001E100(m,object+492,0,12);
    if(rrj_u16(rrj_at(m,object+362,2))==1){
        previous=rrj_read32(m,object+496);rrj_write32(m,object+492,0);
        if(previous==0xffffffffu)rrj_write32(m,object+496,0);
        sub_8003F204(m,object,hint);
    }else{
        previous=rrj_read32(m,object+492);rrj_write32(m,object+496,0);
        if(previous==0xffffffffu)rrj_write32(m,object+492,0);
        (void)sub_8003F1F0(m,object);
    }
}


/* 8003BD2C: lookup a directional junction link and its signed offset. */
static uint32_t junction_link_lookup(RRJMemory *m,uint32_t record,uint32_t first,uint32_t next,uint32_t *offset,uint32_t *found,unsigned *written)
{
    uint32_t metadata,i=0,manager,entry,link,index,table,value;
    *written=0;if(!record)return 0;
    metadata=rrj_read32(m,record+24);if(!metadata)return 0;
    while(rrj_s32(i)<rrj_s32(material_half(m,metadata+8))){
        index=material_half(m,metadata+6)+i;manager=rrj_read32(m,0x8005B240);
        entry=rrj_read32(m,manager+52)+12u*index;
        if(material_half(m,entry+8)==first && material_half(m,entry+10)==next){
            link=rrj_read32(m,manager+56)+12u*material_half(m,entry+2);
            index=material_half(m,rrj_read32(m,record+4)+20)+material_half(m,link+4);
            table=rrj_read32(m,rrj_read32(m,record)+48);value=material_half(m,link+8);
            *offset=value;*found=entry;*written=1;return table+28u*index;
        }
        metadata=rrj_read32(m,record+24);++i;
    }
    return 0;
}
uint32_t rrj_find_junction_link_local(RRJMemory *m,uint32_t record,uint32_t first,uint32_t next,uint32_t *offset,uint32_t *entry)
{unsigned written;return junction_link_lookup(m,record,first,next,offset,entry,&written);}
uint32_t sub_8003BD2C(RRJMemory *m,uint32_t record,uint32_t first,uint32_t next,uint32_t offset,uint32_t entry)
{
    uint32_t a=0,b=0,result;unsigned written;
    result=junction_link_lookup(m,record,first,next,&a,&b,&written);
    if(written){rrj_write32(m,offset,a);rrj_write32(m,entry,b);}return result;
}
/* 8003E61C: signed fixed-point blend, truncating each product separately. */
uint32_t sub_8003E61C(uint32_t weight,uint32_t first,uint32_t next)
{
    int64_t a=(int64_t)rrj_s32(65536u-weight)*(int64_t)rrj_s32(first);
    int64_t b=(int64_t)rrj_s32(weight)*(int64_t)rrj_s32(next);
    return (uint32_t)((uint64_t)a>>16)+(uint32_t)((uint64_t)b>>16);
}


/* 8003E45C: interpolate two surface bands; actual eight-argument ABI. */
uint32_t sub_8003E45C(RRJMemory *m,uint32_t first,uint32_t next,uint32_t weight,uint32_t position,uint32_t side_a,uint32_t side_b,uint32_t reference,uint32_t output)
{
    uint32_t a=first+(side_a==2?136u:8u),b=next+(side_b==2?136u:8u),main_width,inner,x,y,ca,cb,value;
    if(rrj_s32(position^reference)<0)position=0;position=surface_magnitude(position);
    main_width=sub_8003E61C(weight,surface_magnitude(rrj_read32(m,a+8)),surface_magnitude(rrj_read32(m,b+8)));
    inner=main_width;x=surface_magnitude(rrj_read32(m,a+8));y=surface_magnitude(rrj_read32(m,b+8));
    ca=material_half(m,a+2);
    if(rrj_s32(ca)>0){
        x=surface_magnitude(rrj_read32(m,a+24));cb=material_half(m,b+2);
        if(cb)y=surface_magnitude(rrj_read32(m,b+24));inner=sub_8003E61C(weight,x,y);
    }else{
        cb=material_half(m,b+2);
        if(rrj_s32(cb)>0){
            if(ca)x=surface_magnitude(rrj_read32(m,a+24));
            y=surface_magnitude(rrj_read32(m,b+24));inner=sub_8003E61C(weight,x,y);
        }
    }
    if(rrj_s32(position)<rrj_s32(surface_magnitude(inner))){
        rrj_put16(rrj_at(m,output+2,2),1);rrj_write32(m,output+4,0);rrj_put16(rrj_at(m,output,2),4);return 0;
    }
    if(rrj_s32(position)<rrj_s32(surface_magnitude(main_width))){
        rrj_write32(m,output+4,inner);value=rrj_u16(rrj_at(m,a+18,2));
        rrj_put16(rrj_at(m,output,2),1);rrj_put16(rrj_at(m,output+2,2),value);return 0;
    }
    rrj_write32(m,output+4,main_width);rrj_put16(rrj_at(m,output+2,2),0);rrj_put16(rrj_at(m,output,2),0);return 1;
}
/* 8003ED14: compose an interpolated junction surface contact (nine args). */
uint32_t sub_8003ED14(RRJMemory *m,uint32_t first,uint32_t interval,uint32_t orientation_a,uint32_t orientation_b,uint32_t next,uint32_t position,uint32_t weight,uint32_t output,uint32_t reference)
{
    uint32_t side_a=rrj_s32(orientation_a)>0?1u:2u,side_b=rrj_s32(orientation_b)>0?2u:1u;
    uint32_t a=first+(side_a==2?136u:8u),b=next+(side_b==2?136u:8u),width,material,flags,value,old;
    width=sub_8003E61C(weight,surface_magnitude(rrj_read32(m,a+8)),surface_magnitude(rrj_read32(m,b+8)));
    material=rrj_u16(rrj_at(m,a+4,2));
    flags=sub_8003E45C(m,first,next,weight,position,side_a,side_b,reference,output+20);
    value=sub_8003E61C(weight,rrj_read32(m,first+4),rrj_read32(m,next+4));
    rrj_write32(m,output+28,0);rrj_put16(rrj_at(m,output+36,2),0);rrj_write32(m,output+40,width);
    old=rrj_read32(m,output+16);
    rrj_write32(m,output+52,value);rrj_write32(m,output,first);rrj_write32(m,output+4,interval);
    rrj_write32(m,output+8,next);rrj_write32(m,output+12,interval);
    rrj_write32(m,output+16,(old&0xfffffff0u)|flags);rrj_put16(rrj_at(m,output+48,2),material);return 0;
}


/* 80036800: optional lateral/longitudinal projections; callers ignore v0. */
void sub_80036800(RRJMemory *m,uint32_t point,uint32_t piece,uint32_t lateral,uint32_t longitudinal)
{
    if(lateral)rrj_write32(m,lateral,sub_800B6AAC(m,point,piece+2,piece+20));
    if(longitudinal)rrj_write32(m,longitudinal,sub_800B6AAC(m,point,piece+14,piece+20));
}
void rrj_project_track_local(RRJMemory *m,uint32_t point,uint32_t piece,uint32_t *lateral,uint32_t *longitudinal)
{
    if(lateral)*lateral=sub_800B6AAC(m,point,piece+2,piece+20);
    if(longitudinal)*longitudinal=sub_800B6AAC(m,point,piece+14,piece+20);
}


/* 8003697C: distance from a segment start or its opposite endpoint. */
uint32_t sub_8003697C(RRJMemory *m,uint32_t previous,uint32_t direction,uint32_t piece,uint32_t point)
{
    uint32_t origin[3],axis[3],i,length,result=0;
    for(i=0;i<3;++i){origin[i]=rrj_read32(m,piece+20+4*i);axis[i]=material_half(m,piece+14+2*i)<<4;}
    if(rrj_s32(direction)<=0 && previous!=direction){
        length=rrj_read32(m,piece+32);
        for(i=0;i<3;++i){
            int64_t product=(int64_t)rrj_s32(axis[i])*(int64_t)rrj_s32(length);
            origin[i]+=(uint32_t)((uint64_t)product>>16);
        }
    }
    for(i=0;i<3;++i){
        uint32_t delta=rrj_read32(m,point+4*i)-origin[i];
        int64_t product=(int64_t)rrj_s32(axis[i])*(int64_t)rrj_s32(delta);
        result+=(uint32_t)((uint64_t)product>>16);
    }
    return result;
}

/* 8001FC58: original 32-bit LCG, including seed writeback. */
uint32_t sub_8001FC58(RRJMemory *m)
{
    uint32_t value=rrj_read32(m,0x8005B4A8u)*1664525u+1013904223u;
    rrj_write32(m,0x8005B4A8u,value);return value;
}
/* 80039C38: signed link index into the manager's 12-byte records. */
uint32_t sub_80039C38(RRJMemory *m,uint32_t link)
{
    uint32_t index,manager;
    if(!link)return 0;
    index=material_half(m,link+2);
    if(rrj_s32(index)<0)return 0;
    manager=rrj_read32(m,0x8005B240u);
    if(rrj_s32(index)>=rrj_s32(material_half(m,manager+64)))return 0;
    return rrj_read32(m,manager+56)+12u*index;
}
/* 8003F580: membership in a route's signed-count road list. */
uint32_t sub_8003F580(RRJMemory *m,uint32_t record,uint32_t road)
{
    uint32_t count,i;
    if(!record || rrj_s32(road)<0)return 0;
    count=rrj_read32(m,record+16);
    for(i=0;rrj_s32(i)<rrj_s32(count);++i){
        if(rrj_read32(m,record+84)==road)return 1;
        record+=4;
    }
    return 0;
}
/* 8003F5D0: find the first 120-byte route containing a road. */
uint32_t sub_8003F5D0(RRJMemory *m,uint32_t road)
{
    uint32_t i=0,offset=0,record;
    if(rrj_s32(material_half(m,0x800D6182u)+1u)<=0)return 0;
    do{
        record=rrj_read32(m,0x800D6194u)+offset;
        if(sub_8003F580(m,record,road))return record;
        ++i;offset+=120;
    }while(rrj_s32(i)<rrj_s32(material_half(m,0x800D6182u)+1u));
    return 0;
}

/* 8003C840/8003C948: resolve the loaded segment beyond either endpoint. */
static uint32_t adjacent_track_segment(RRJMemory *m,uint32_t segment,uint32_t link_id,unsigned backward)
{
    uint32_t record=sub_80039A08(m,rrj_read32(m,segment)),target=0xffffffffu,header,link;
    if(!record)return 0;
    if(!material_half(m,record+4)){
        header=sub_80039AA0(m,rrj_read32(m,segment));
        if(header)target=material_half(m,header+(backward?4u:2u));
    }else{
        header=sub_80039AFC(m,rrj_read32(m,segment));
        if(header){
            if(!rrj_read32(m,record+28))target=material_half(m,header+6);
            else{
                link=sub_8003A37C(m,header,link_id);
                if(link){
                    unsigned positive=rrj_s32(material_half(m,link+2))>0;
                    target=material_half(m,((positive != backward)?link:header+6));
                }
            }
        }
    }
    record=sub_80039A08(m,target);
    return record?rrj_read32(m,record+12):0;
}
uint32_t sub_8003C840(RRJMemory *m,uint32_t actor,uint32_t segment,uint32_t link_id)
{
    (void)actor;return adjacent_track_segment(m,segment,link_id,0);
}
uint32_t sub_8003C948(RRJMemory *m,uint32_t actor,uint32_t segment,uint32_t link_id)
{
    (void)actor;return adjacent_track_segment(m,segment,link_id,1);
}

/* 8003F408: find an explicitly connected route; table count is exclusive. */
uint32_t sub_8003F408(RRJMemory *m,uint32_t record,uint32_t id)
{
    uint32_t result=0,count,i,n,j,p;
    if(material_half(m,0x800D6182u)==0xffffffffu)return 0;
    if(!record || id==0xffffffffu)return 0;
    if(rrj_read32(m,record)==id)return record;
    count=rrj_read32(m,record+16);
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,record+=4){
        if(rrj_read32(m,record+100)!=id)continue;
        n=material_half(m,0x800D6182u);
        if(rrj_s32(n)<=0)continue;
        p=rrj_read32(m,0x800D6194u);
        for(j=0;rrj_s32(j)<rrj_s32(n);++j,p+=120){
            if(rrj_read32(m,p)==id){result=p;break;}
        }
    }
    return result;
}

/* 8003CCA0: choose a candidate road for a player or pursuing actor. */
static uint32_t actor_route_choice_ref(RRJMemory *m,uint32_t actor,RRJTrackWords candidates,uint32_t count)
{
    uint32_t desired=0xffffffffu,result=count-2u,identity,target,route,index=0,i,player,road;
    if(rrj_u16(rrj_at(m,actor+172,2))>=rrj_read32(m,rrj_read32(m,0x8005B2F8u)+48)
       && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2
       && rrj_s32(rrj_read32(m,actor+480))<131){
        index=material_byte(m,actor+946);if(index&128u)index|=0xffffff00u;
        identity=rrj_u16(rrj_at(m,actor+958+8u*(index-1u),2));
        if(identity!=224 && !(identity>>5)){
            target=rrj_read32(m,0x8005B3A0u)+1096u*identity;
            if(target)desired=rrj_u16(rrj_at(m,target+360,2));
        }
    }
    if(material_half(m,0x800D6182u)==0xffffffffu){
        return count==2?sub_8001FC58(m)%count:result;
    }
    route=sub_8003F408(m,rrj_read32(m,actor+428),rrj_read32(m,track_read(m,candidates,0)+8));
    if(!route)return result;
    index=0;
    if(rrj_s32(rrj_read32(m,route+16))>=2){
        index=sub_8001FC58(m);if(count)index%=count;
    }
    if(rrj_s32(count)<=0)return result;
    target=route+4u*index;
    player=rrj_u16(rrj_at(m,actor+172,2))<rrj_read32(m,rrj_read32(m,0x8005B2F8u)+48);
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,candidates=track_offset(candidates,32)){
        if(!player && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2 && desired!=0xffffffffu){
            road=material_half(m,track_read(m,candidates,28)+10);
            if(road==desired)return i;
        }else{
            road=material_half(m,track_read(m,candidates,28)+10);
            if(road==rrj_read32(m,target+84))return i;
        }
    }
    return result;
}

uint32_t sub_8003CCA0(RRJMemory *m,uint32_t actor,uint32_t candidates,uint32_t count)
{
    return actor_route_choice_ref(m,actor,track_ram(candidates),count);
}
uint32_t rrj_actor_route_choice_local(RRJMemory *m,uint32_t actor,uint32_t candidates[24],uint32_t count)
{
    return actor_route_choice_ref(m,actor,track_local(candidates),count);
}

static unsigned excluded_route_road(uint32_t road)
{
    return road==11 || road==12 || road==20 || road==26;
}
/* 80039048: select and commit a candidate track record (six arguments). */
static void track_copy_record(RRJMemory *m,RRJTrackWords output,RRJTrackWords source)
{
    uint32_t i;
    if(!output.local && !source.local){(void)sub_8001E0B4(m,output.address,source.address,32);return;}
    for(i=0;i<8;++i)track_write(m,output,4*i,track_read(m,source,4*i));
}
static uint32_t commit_track_ref(RRJMemory *m,uint32_t identity_ptr,RRJTrackWords candidates,RRJTrackWords directions,uint32_t count,RRJTrackWords output,RRJTrackWords direction_out,RRJTrackWords previous,RRJTrackWords previous_direction)
{
    uint32_t kind=6,actor=0,identity=0,index=0xffffffffu,value,old,traffic,desired=0xffffffffu,i,p,road;
    RRJTrackWords cursor,selected;
    if(identity_ptr){
        identity=rrj_u16(rrj_at(m,identity_ptr,2));kind=identity>>5;
        if(!kind)actor=rrj_read32(m,0x8005B3A0u)+1096u*identity;
    }
    if(!track_read(m,candidates,28)){
        if(actor && rrj_read32(m,actor+356)){
            *(uint8_t *)rrj_at(m,actor+947,1)=255;rrj_write32(m,actor+948,0);rrj_write32(m,actor+952,0);
        }
        if(track_read(m,output,8)==track_read(m,candidates,8))track_write(m,output,12,track_read(m,candidates,12));
        else track_copy_record(m,output,candidates);
        value=track_read(m,directions,0);if(value==1 || value==0xffffffffu)track_write(m,direction_out,0,value);
        return track_read(m,output,12);
    }
    if(rrj_s32(count)<2){
        if(track_read(m,output,8)==track_read(m,candidates,8))track_write(m,output,12,track_read(m,candidates,12));
        else{
            track_copy_record(m,output,candidates);track_write(m,direction_out,0,track_read(m,directions,0));
        }
        return track_read(m,output,12);
    }
    if(kind==3){
        traffic=0x800CF660u+512u*(identity&31u);
        if(sub_8003F3B4(m,rrj_read32(m,track_read(m,candidates,4)+12))){
            if((material_byte(m,rrj_read32(m,0x8005B2F8u)+4)&1u) && !rrj_read32(m,traffic+180)){
                value=material_byte(m,traffic+511);
                if(rrj_s32(value)<rrj_s32(rrj_read32(m,0x8005B1F8u))){
                    p=rrj_read32(m,0x8005B3A0u)+1096u*value;
                    if(p){value=rrj_read32(m,p+360);if(!(value>>16))desired=value;}
                }
            }
            for(i=0,cursor=candidates;rrj_s32(i)<rrj_s32(count);++i,cursor=track_offset(cursor,32)){
                value=material_byte(m,rrj_read32(m,0x8005B2F8u)+4);
                road=material_half(m,track_read(m,cursor,28)+10);
                if(value&16u){
                    if(sub_8003F5D0(m,road) && !excluded_route_road(road)){index=i;break;}
                }else if((desired!=0xffffffffu && road==desired) || sub_8003F5D0(m,road)){index=i;break;}
            }
        }
        if(index==0xffffffffu){
            if(material_byte(m,rrj_read32(m,0x8005B2F8u)+4)&16u){
                for(i=0,cursor=candidates;rrj_s32(i)<rrj_s32(count);++i,cursor=track_offset(cursor,32)){
                    road=material_half(m,track_read(m,cursor,28)+10);
                    if(!excluded_route_road(road)){index=i;break;}
                }
            }else index=sub_8001FC58(m)%count;
        }
    }else if(kind)index=sub_8001FC58(m)%count;
    else if(actor){
        if(rrj_u16(rrj_at(m,actor+172,2))<rrj_read32(m,rrj_read32(m,0x8005B2F8u)+48)
           || (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)!=2){
            old=rrj_read32(m,actor+948);
            if(old){
                if(material_half(m,old+2)==material_half(m,track_read(m,candidates,24)+2)){
                    index=material_byte(m,actor+947);if(index&128u)index|=0xffffff00u;
                }else{
                    *(uint8_t *)rrj_at(m,actor+947,1)=255;rrj_write32(m,actor+948,0);rrj_write32(m,actor+952,0);
                }
            }
        }
        if(index==0xffffffffu)index=actor_route_choice_ref(m,actor,candidates,count);
    }
    if(candidates.local){
        if(index==0xffffffffu)selected=previous;
        else{if(index>=3)abort();selected=track_offset(candidates,32u*index);}
    }else selected=track_offset(candidates,32u*index);
    track_copy_record(m,output,selected);
    if(directions.local && index==0xffffffffu && !previous_direction.local)abort(); /* Original caller reads an undefined stack word. */
    value=directions.local && index==0xffffffffu?track_read(m,previous_direction,0):track_read(m,directions,4u*index);
    track_write(m,direction_out,0,value);
    if(actor && !rrj_read32(m,actor+948)){
        *(uint8_t *)rrj_at(m,actor+947,1)=(uint8_t)index;
        rrj_write32(m,actor+948,track_read(m,output,24));rrj_write32(m,actor+952,track_read(m,output,28));
    }
    return track_read(m,output,12);
}

/* 80039BB0: gather matching links, preserving the original inclusive limit. */
uint32_t sub_80039048(RRJMemory *m,uint32_t identity_ptr,uint32_t candidates,uint32_t directions,uint32_t count,uint32_t output,uint32_t direction_out)
{
    return commit_track_ref(m,identity_ptr,track_ram(candidates),track_ram(directions),count,track_ram(output),track_ram(direction_out),track_ram(0),track_ram(0));
}
uint32_t rrj_commit_track_local(RRJMemory *m,uint32_t identity_ptr,uint32_t candidates[24],uint32_t directions[3],uint32_t count,uint32_t output[8],uint32_t *direction_out,uint32_t previous[8],uint32_t *previous_direction)
{
    return commit_track_ref(m,identity_ptr,track_local(candidates),track_local(directions),count,track_local(output),track_local(direction_out),track_local(previous),track_local(previous_direction));
}

static uint32_t collect_route_links(RRJMemory *m,uint32_t metadata,uint32_t id,uint32_t output,uint32_t limit,uint32_t *local,uint32_t capacity)
{
    uint32_t matches=0,i=0,manager,entry,index;
    if(!metadata || rrj_s32(material_half(m,metadata+8))<=0)return 0;
    manager=rrj_read32(m,0x8005B240u);
    do{
        index=material_half(m,metadata+6)+i;
        entry=rrj_read32(m,manager+52)+12u*index;
        if(material_half(m,entry+8)==id){
            if(rrj_s32(limit)<rrj_s32(matches))return 0xffffffffu;
            if(local){if(matches>=capacity)abort();local[matches]=entry;}
            else{rrj_write32(m,output,entry);output+=4;}
            ++matches;
        }
        ++i;
    }while(rrj_s32(i)<rrj_s32(material_half(m,metadata+8)));
    return matches;
}
uint32_t sub_80039BB0(RRJMemory *m,uint32_t metadata,uint32_t id,uint32_t output,uint32_t limit)
{
    return collect_route_links(m,metadata,id,output,limit,NULL,0);
}
uint32_t rrj_collect_route_links_local(RRJMemory *m,uint32_t metadata,uint32_t id,uint32_t *output,uint32_t capacity,uint32_t limit)
{
    return collect_route_links(m,metadata,id,0,limit,output,capacity);
}

/* 80037A30/80037FBC: enumerate the next track pieces in either direction. */
static uint32_t traverse_track_candidates(RRJMemory *m,uint32_t actor,RRJTrackWords record,RRJTrackWords output,RRJTrackWords directions,uint32_t limit,unsigned backward)
{
    uint32_t segment=track_read(m,record,0),route=track_read(m,record,4),sub=track_read(m,record,8);
    uint32_t last=track_read(m,record,28),saved=track_read(m,record,12),piece=0,state=0,count=0;
    uint32_t old,header,link=0xffffffffu,lookup,metadata,links[4],n,i,index,base,value,positive;
    RRJTrackWords p;
    if(backward?material_half(m,saved)!=0:rrj_s32(material_half(m,saved))<rrj_s32(material_half(m,sub+10)-1u)){
        piece=backward?saved-52:saved+52;route=0;state=1;goto tail;
    }
    old=segment;
    if(!material_half(m,segment+16)){
        segment=backward?sub_8003C948(m,actor,segment,0xffffffffu):sub_8003C840(m,actor,segment,0xffffffffu);
        if(!segment)goto tail;
        route=!material_half(m,segment+16)?rrj_read32(m,segment+44):sub_80039C90(m,segment,rrj_read32(m,old+8));
        if(route)state=1;goto tail;
    }
    header=sub_80039AFC(m,rrj_read32(m,segment));if(!header)goto tail;
    if(rrj_read32(m,segment+12)==1){
        link=sub_8003A37C(m,header,rrj_read32(m,route+12));if(!link)goto tail;
        value=rrj_read32(m,route+12);
        segment=backward?sub_8003C948(m,actor,segment,value):sub_8003C840(m,actor,segment,value);
        if(!segment)goto tail;
        route=!material_half(m,segment+16)?rrj_read32(m,segment+44):sub_80039C90(m,segment,material_half(m,link+4));
        if(route)state=1;goto tail;
    }
    if(material_half(m,route+2)){
        if(!backward && !last)goto tail;
        if(backward)route=0;
        value=0xffffffffu;
        if(last){
            lookup=sub_80039C38(m,last);if(!backward && !lookup)goto tail;
            positive=rrj_s32(material_half(m,last+6))>0;
            index=rrj_s32(material_half(m,lookup+8))>0;
            value=material_half(m,last+((positive==index)?(backward?8u:10u):(backward?10u:8u)));
        }
        route=0;link=sub_8003A37C(m,header,value);
        if(link)route=sub_80039C90(m,segment,material_half(m,link+4));
        if(route){
            base=rrj_read32(m,segment+52);sub=rrj_read32(m,segment+48)+28u*material_half(m,route+20);
            index=material_half(m,sub+8);value=material_half(m,link+2);
            if(backward?rrj_s32(value)<=0:rrj_s32(value)<0)index+=material_half(m,sub+10)-1u;
            piece=base+52u*index;state=2;track_write(m,directions,0,material_half(m,link+2));
        }
        goto tail;
    }
    link=sub_8003A37C(m,header,rrj_read32(m,route+12));if(!link)goto tail;
    positive=rrj_s32(material_half(m,link+2))>0;
    if(positive!=backward){
        value=rrj_read32(m,route+12);route=0;
        segment=backward?sub_8003C948(m,actor,segment,value):sub_8003C840(m,actor,segment,value);
        if(segment)route=sub_80039C90(m,segment,material_half(m,link+4));
        if(route)state=1;goto tail;
    }
    metadata=sub_80039B60(m,rrj_read32(m,segment));if(!metadata)goto tail;
    n=rrj_collect_route_links_local(m,metadata,material_half(m,link+4),links,4,limit);
    if(rrj_s32(n)<=0 || rrj_s32(n)>=rrj_s32(limit))goto tail;
    for(i=0;i<n;++i){
        last=links[i];lookup=sub_80039C38(m,last);if(!lookup)continue;
        route=rrj_read32(m,segment+44)+32u*material_half(m,lookup+2);
        sub=rrj_read32(m,segment+48)+28u*(material_half(m,route+20)+material_half(m,lookup+4));
        positive=rrj_s32(material_half(m,last+6))>0;
        index=material_half(m,sub+8);value=material_half(m,lookup+8);base=rrj_read32(m,segment+52);
        if(positive!=(uint32_t)(rrj_s32(value)>0))index+=material_half(m,sub+10)-1u;
        piece=base+52u*index;value=material_half(m,lookup+8);
        track_write(m,directions,0,positive?value:0u-value);directions=track_offset(directions,4);
        p=track_offset(output,32u*count);track_write(m,p,20,track_read(m,record,20));value=track_read(m,record,16);
        track_write(m,p,0,segment);track_write(m,p,4,route);track_write(m,p,8,sub);track_write(m,p,12,piece);
        track_write(m,p,28,last);track_write(m,p,24,metadata);track_write(m,p,16,value);++count;state=1;
    }
tail:
    if(!state){
        p=track_offset(output,32u*count);track_write(m,p,8,track_read(m,record,8));track_write(m,p,12,track_read(m,record,12));return count+1;
    }
    if(count)return count;
    if(!route){
        value=track_read(m,record,8);
        if(sub==value || piece==saved){track_write(m,output,8,value);track_write(m,output,12,piece);return 1;}
    }
    if(state!=2){
        base=rrj_read32(m,segment+52);sub=rrj_read32(m,segment+48)+28u*material_half(m,route+20);
        index=material_half(m,sub+8);if(backward)index+=material_half(m,sub+10)-1u;
        track_write(m,directions,0,backward?0xffffffffu:1);piece=base+52u*index;
    }
    track_write(m,output,20,track_read(m,record,20));value=track_read(m,record,16);
    track_write(m,output,0,segment);track_write(m,output,4,route);track_write(m,output,8,sub);track_write(m,output,12,piece);
    track_write(m,output,24,0);track_write(m,output,28,0);track_write(m,output,16,value);return 1;
}
uint32_t sub_80037A30(RRJMemory *m,uint32_t actor,uint32_t record,uint32_t output,uint32_t directions,uint32_t limit)
{
    return traverse_track_candidates(m,actor,track_ram(record),track_ram(output),track_ram(directions),limit,0);
}
uint32_t sub_80037FBC(RRJMemory *m,uint32_t actor,uint32_t record,uint32_t output,uint32_t directions,uint32_t limit)
{
    return traverse_track_candidates(m,actor,track_ram(record),track_ram(output),track_ram(directions),limit,1);
}

uint32_t rrj_traverse_track_local(RRJMemory *m,uint32_t actor,uint32_t record[8],uint32_t candidates[24],uint32_t directions[3],uint32_t limit,unsigned backward)
{
    return traverse_track_candidates(m,actor,track_local(record),track_local(candidates),track_local(directions),limit,backward);
}

static uint32_t commit_followed_piece(RRJMemory *m,uint32_t actor,RRJTrackWords record,uint32_t temporary[8])
{
    uint32_t flags,selected;
    if(track_read(m,record,8)==temporary[2])track_write(m,record,12,temporary[3]);
    else track_copy_record(m,record,track_local(temporary));
    flags=rrj_read32(m,actor+216);selected=track_read(m,record,12);
    if((flags&128u) && material_half(m,selected+50))
        rrj_write32(m,rrj_read32(m,0x8005B3A0u)+1096u*rrj_u16(rrj_at(m,actor,2))+828,selected);
    return selected;
}
/* 80036B14: follow a position across pieces and junctions. */
static uint32_t follow_track_ref(RRJMemory *m,uint32_t actor,RRJTrackWords record,uint32_t point)
{
    uint32_t temporary[8]={0},candidates[24]={0},directions[3]={0};
    uint32_t selected,candidate,distance,previous,direction,last_distance=0,n,index,difference;
    temporary[2]=track_read(m,record,8);temporary[3]=track_read(m,record,12);selected=track_read(m,record,12);
    distance=sub_800B6AAC(m,point,selected+14,selected+20);
    if(!distance)goto done;
    direction=rrj_s32(distance)>0?1u:0xffffffffu;previous=direction;directions[0]=direction;candidate=selected;
    index=material_half(m,temporary[3]);
    if(index!=0 && index!=material_half(m,temporary[2]+10)-1u){candidate+=52u*previous;temporary[3]=candidate;}
    else{
        if(!temporary[0]){track_copy_record(m,track_local(temporary),record);temporary[3]=candidate;}
        n=rrj_traverse_track_local(m,actor,temporary,candidates,directions,3,rrj_s32(direction)<=0);
        candidate=rrj_commit_track_local(m,actor,candidates,directions,n,temporary,&direction,temporary,NULL);
    }
    distance=sub_8003697C(m,previous,direction,candidate,point);
    while((rrj_s32(direction)>0 && rrj_s32(distance)>=0) || (rrj_s32(direction)<0 && rrj_s32(distance)<0)){
        selected=commit_followed_piece(m,actor,record,temporary);previous=direction;
        index=material_half(m,temporary[3]);
        if(index!=0 && index!=material_half(m,temporary[2]+10)-1u){candidate+=52u*previous;temporary[3]=candidate;}
        else{
            if(!temporary[0]){track_copy_record(m,track_local(temporary),record);temporary[3]=candidate;}
            if(rrj_track_exit_local(m,temporary,direction)){
                if(rrj_u16(rrj_at(m,actor,2))<rrj_read32(m,rrj_read32(m,0x8005B2F8u)+48))rrj_write32(m,0x8005B580u,1);
                break;
            }
            n=rrj_traverse_track_local(m,actor,temporary,candidates,directions,3,rrj_s32(direction)<=0);
            candidate=rrj_commit_track_local(m,actor,candidates,directions,n,temporary,&direction,temporary,NULL);
        }
        distance=sub_8003697C(m,previous,direction,candidate,point);
        difference=surface_magnitude(distance-last_distance);last_distance=distance;
        if(rrj_s32(difference)<131)break;
    }
    if(rrj_s32(previous)<0 && rrj_s32(direction)<0 && rrj_s32(distance)>0
       && rrj_s32(distance)<rrj_s32(rrj_read32(m,temporary[3]+32)))
        selected=commit_followed_piece(m,actor,record,temporary);
done:
    rrj_write32(m,actor+216,rrj_read32(m,actor+216)&0xffffff7fu);return selected;
}

uint32_t sub_80036B14(RRJMemory *m,uint32_t actor,uint32_t record,uint32_t point)
{
    return follow_track_ref(m,actor,track_ram(record),point);
}
uint32_t rrj_follow_track_local(RRJMemory *m,uint32_t actor,uint32_t record[8],uint32_t point)
{
    return follow_track_ref(m,actor,track_local(record),point);
}

/* 8003E754: junction surface attachment, actual five-argument ABI. */
static uint32_t surface_attach_ref(RRJMemory *m,uint32_t actor,uint32_t header,uint32_t output,RRJTrackWords track_output,uint32_t hint)
{
    uint32_t root=rrj_read32(m,actor+328),first=0,next=0,reference=1,link=0;
    uint32_t record[8],vector[3]={0},sub,piece,length,lateral,longitudinal,index,interval,pair;
    uint32_t a,b,side_a,side_b,surface,next_surface,progress,delta,clamped,ratio,result,value,i;
    uint16_t normal[3];
    if(!header || !rrj_read32(m,root+64))goto missing;
    if(rrj_s32(rrj_find_junction_planes_local(m,actor+184,header,&first,&next,hint))<0)goto missing;
    sub=rrj_find_junction_link_local(m,actor+328,material_half(m,first+4),material_half(m,next+4),&reference,&link);
    length=rrj_read32(m,sub+12);
    for(i=0;i<8;++i)record[i]=rrj_read32(m,actor+328+4*i);
    record[2]=sub;record[3]=rrj_read32(m,root+52)+52u*material_half(m,sub+8);
    piece=rrj_follow_track_local(m,actor+172,record,actor+184);
    rrj_project_track_local(m,actor+184,piece,&lateral,&longitudinal);
    if(material_half(m,header+2)==2)lateral=surface_magnitude(lateral);
    record[4]=lateral;record[5]=longitudinal;
    for(i=0;i<3;++i)normal[i]=rrj_u16(rrj_at(m,piece+14+2*i,2));
    /* Prior direction changes only magnitude: all downstream tests use its sign.
       This caller consumes only progress, independent of the original stale word. */
    (void)rrj_update_track_direction_local(m,normal,record,vector);
    index=material_half(m,sub+18);if(rrj_s32(index)<0)goto missing;
    interval=rrj_read32(m,root+56)+16u*index;pair=rrj_read32(m,root+64)+20u*index;
    a=material_half(m,pair+4);if(a==0xffffffffu)goto missing;
    b=material_half(m,pair+6);if(b==0xffffffffu)goto missing;
    if(material_half(m,first+4)==material_half(m,pair+10) && material_half(m,next+4)==material_half(m,pair+12)){
        side_a=material_half(m,first+2);side_b=material_half(m,next+2);
        value=rrj_read32(m,root+60);surface=value+264u*a;next_surface=value+264u*b;
    }else{
        b=material_half(m,pair+6);value=rrj_read32(m,root+60);a=material_half(m,pair+4);
        surface=value+264u*b;next_surface=value+264u*a;
        side_a=material_half(m,next+2);side_b=material_half(m,first+2);
    }
    progress=vector[2];delta=length-progress;
    clamped=(~pause_asr(progress,31)&progress)+(pause_asr(delta,31)&delta);
    ratio=sub_80010028(surface_magnitude(clamped),surface_magnitude(length));
    if((rrj_s32(clamped)>0)!=(rrj_s32(length)>0))ratio=0u-ratio;
    if(!surface)goto missing;
    result=sub_8003ED14(m,surface,interval,side_a,side_b,next_surface,lateral,ratio,output,reference);
    if(track_output.local || track_output.address){
        if(material_half(m,record[1]+2)==1){
            track_write(m,track_output,0,record[0]);value=record[1];
            track_write(m,track_output,8,sub);track_write(m,track_output,12,piece);
            track_write(m,track_output,16,lateral);track_write(m,track_output,20,longitudinal);
            track_write(m,track_output,4,value);track_write(m,track_output,28,link);track_write(m,track_output,24,record[6]);
        }else track_write(m,track_output,12,0);
    }
    return result&255u;
missing:
    value=rrj_read32(m,output+16);rrj_write32(m,output,0);rrj_write32(m,output+16,value&0xfffffff0u);
    if(track_output.local || track_output.address)track_write(m,track_output,12,0);return 1;
}

/* 8003EE68: junction-only contact selection; fourth argument is plane hint. */
static uint32_t select_surface_ref(RRJMemory *m,uint32_t actor,uint32_t output,RRJTrackWords track_output,uint32_t hint)
{
    uint32_t kind=material_half(m,rrj_read32(m,actor+332)+2),root=rrj_read32(m,actor+328);
    uint32_t header,result=0,selected=0,value;
    if(kind==1){
        header=sub_80039AFC(m,rrj_read32(m,root));
        if(header){result=surface_attach_ref(m,actor,header,output,track_output,hint);selected=rrj_read32(m,output);}
    }
    if(!selected){
        value=rrj_read32(m,output+16);rrj_write32(m,output,0);rrj_write32(m,output+16,value&0xfffffff0u);
        if(track_output.local || track_output.address)track_write(m,track_output,12,0);
    }
    return result&255u;
}

/* 8003DE28: update actor contact and retain the previous contact flag. */
static uint32_t update_contact_ref(RRJMemory *m,uint32_t actor,uint32_t reset,RRJTrackWords track_output,uint32_t hint)
{
    uint32_t previous=0,interval,result;
    if(rrj_read32(m,actor+372))previous=rrj_read32(m,actor+388)&1u;
    if(reset==1){
        rrj_write32(m,actor+372,0);rrj_write32(m,actor+376,0);
        rrj_write32(m,actor+380,0);rrj_write32(m,actor+384,0);
    }
    if(rrj_u16(rrj_at(m,actor+362,2))==1){
        interval=rrj_read32(m,actor+376);
        if(interval && rrj_s32(material_half(m,interval+4))>=0)rrj_write32(m,actor+376,0);
        result=select_surface_ref(m,actor,actor+372,track_output,hint);
    }else{
        interval=rrj_read32(m,actor+376);
        if(interval && rrj_s32(material_half(m,interval+4))<0)rrj_write32(m,actor+376,0);
        result=sub_8003EF34(m,actor+328,actor+360,actor+372);
        if(track_output.local || track_output.address)track_write(m,track_output,12,0);
    }
    if(rrj_read32(m,actor+372) && ((rrj_read32(m,actor+388)&1u) || previous))result=1;
    return result&255u;
}

/* 800952AC: actor state transition, effects-only return contract. */
void sub_800952AC(RRJMemory *m,uint32_t actor,uint32_t requested,RRJReverbCall reverb)
{
    uint32_t active,linked,piece;uint8_t event[8]={0};
    if(!actor)return;
    active=rrj_u16(rrj_at(m,actor+320,2));if((active&1u)==(requested&1u))return;
    if(active){
        if(!sub_8003A468(m,actor+360,actor+328)){rrj_put16(rrj_at(m,actor+320,2),0);return;}
        rrj_write32(m,actor+600,0xffff0000u);
        (void)sub_8003DE28(m,actor,1,0,0xffffffffu);sub_8003DF54(m,actor,1,0xffffffffu);
        piece=rrj_read32(m,actor+340);
        (void)sub_8002EAD8(m,piece+20,piece+14,rrj_read32(m,actor+348),actor+184);
        piece=rrj_read32(m,actor+340);
        (void)sub_8002EAD8(m,actor+184,piece+2,rrj_read32(m,actor+344),actor+184);
        rrj_write32(m,actor+508,0);rrj_write32(m,actor+552,0);rrj_put16(rrj_at(m,actor+544,2),224);rrj_write32(m,actor+604,0);
        (void)sub_800C4550(m,73,actor,17);
    }else{
        linked=rrj_read32(m,actor+596);
        if(material_half(m,linked+320)){
            (void)sub_800C3104(m,actor,0);(void)sub_800BCD10(m,rrj_read32(m,actor+596));
            linked=rrj_read32(m,actor+596);rrj_put16(event,4);rrj_put16(event+2,rrj_u16(rrj_at(m,linked+172,2)));
            /* Mode zero overwrites both queued tail halves before their use. */
            (void)rrj_enqueue_actor_event_local(m,event,0,rrj_read32(m,actor+596));
            rrj_put16(event,18);(void)rrj_enqueue_actor_event_local(m,event,0,rrj_read32(m,actor+596));
        }else (void)sub_800903F4(m,linked,1,reverb);
    }
}

/* 800951B8: choose actor activity then apply its transition. */
void sub_800951B8(RRJMemory *m,uint32_t actor,uint32_t force_inactive,RRJReverbCall reverb)
{
    uint32_t previous=material_half(m,actor+320),tag,count,value,linked;
    if(!previous)(void)sub_80099D48(m,actor);
    else if(rrj_read32(m,actor+552)&0x04000000u){
        tag=rrj_u16(rrj_at(m,actor+172,2));count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
        if(((tag>>5)!=1 || rrj_s32(tag&31u)>=rrj_s32(count)) && !(material_byte(m,actor+572)&32u)){
            force_inactive=1;linked=rrj_read32(m,actor+596);value=rrj_read32(m,actor+552);
            linked=rrj_read32(m,linked+360);rrj_write32(m,actor+552,value&0xfbffffffu);rrj_write32(m,actor+360,linked+1u);
        }
    }
    if(force_inactive)rrj_put16(rrj_at(m,actor+320,2),0);
    else rrj_put16(rrj_at(m,actor+320,2),sub_80039F68(m,actor+172));
    sub_800952AC(m,actor,previous,reverb);
}

/* 8003701C: follow actor position and update its road coordinates. */
uint32_t sub_8003701C(RRJMemory *m,uint32_t actor)
{
    uint32_t sub=rrj_read32(m,actor+336),old_piece=rrj_read32(m,actor+340),old_id=material_half(m,sub+6);
    uint32_t piece=sub_80036B14(m,actor+172,actor+328,actor+184),prior,mask,value,flags;
    sub=rrj_read32(m,actor+336);prior=rrj_read32(m,actor+364);
    mask=material_half(m,sub+6)==old_id?0xffffffffu:0;
    value=rrj_s32(prior)>0?5u+(mask&(prior-5u)):0xfffffffbu+(mask&(prior+5u));
    rrj_write32(m,actor+364,value);
    sub_80036800(m,actor+184,piece,actor+344,actor+348);
    (void)sub_8003662C(m,actor+450,actor+328,actor+360);
    flags=rrj_read32(m,actor+388);flags=piece==old_piece?flags&0xffffffbfu:flags|64u;
    rrj_write32(m,actor+388,flags);return flags;
}

uint32_t sub_8003E754(RRJMemory *m,uint32_t actor,uint32_t header,uint32_t output,uint32_t track_output,uint32_t hint)
{return surface_attach_ref(m,actor,header,output,track_ram(track_output),hint);}
uint32_t sub_8003EE68(RRJMemory *m,uint32_t actor,uint32_t output,uint32_t track_output,uint32_t hint)
{return select_surface_ref(m,actor,output,track_ram(track_output),hint);}
uint32_t sub_8003DE28(RRJMemory *m,uint32_t actor,uint32_t reset,uint32_t track_output,uint32_t hint)
{return update_contact_ref(m,actor,reset,track_ram(track_output),hint);}
uint32_t rrj_update_contact_local(RRJMemory *m,uint32_t actor,uint32_t reset,uint32_t track_output[8],uint32_t hint)
{return update_contact_ref(m,actor,reset,track_local(track_output),hint);}
/* 8003DDB0: optional junction-plane hint for the actor position. */
uint32_t sub_8003DDB0(RRJMemory *m,uint32_t actor)
{
    uint32_t header;
    if(!actor || rrj_u16(rrj_at(m,actor+362,2))!=1)return 0xffffffffu;
    header=sub_80039AFC(m,rrj_read32(m,rrj_read32(m,actor+328)));
    if(!header)return 0xffffffffu;
    return rrj_find_junction_planes_local(m,actor+184,header,NULL,NULL,0xffffffffu);
}
/* 8003DFF4: refresh contact, optionally adopt its junction track record. */
uint32_t sub_8003DFF4(RRJMemory *m,uint32_t actor)
{
    uint32_t hint=sub_8003DDB0(m,actor),record[8]={0},result,flags,value,i;
    result=rrj_update_contact_local(m,actor,0,record,hint)&255u;
    if((rrj_u16(rrj_at(m,actor+172,2))>>5)<2){
        flags=rrj_read32(m,actor+388);
        if(((flags&32u) || (rrj_read32(m,actor+372) && (flags&1u)))
           && rrj_u16(rrj_at(m,actor+362,2))==1
           && material_half(m,rrj_read32(m,actor+332)+2)==1
           && record[3] && rrj_read32(m,actor+336)!=record[2]
           && record[1] && material_half(m,record[1]+2)==1){
            for(i=0;i<8;++i)rrj_write32(m,actor+328+4*i,record[i]);
            (void)sub_8003662C(m,actor+450,actor+328,actor+360);
        }
    }
    if(rrj_read32(m,actor+388)&65u)result=1;
    sub_8003DF54(m,actor,0,hint);
    value=(0u-result)&64u;rrj_write32(m,actor+388,rrj_read32(m,actor+388)|value);return value;
}

/* 8003A5F4: choose an endpoint junction and return its road distance. */
uint32_t rrj_nearest_junction_local(RRJMemory *m,uint32_t position,uint32_t *output,uint32_t mode)
{
    uint32_t word=rrj_read32(m,position),id=0xffffffffu,distance=0,road,progress,junction;
    if((word>>16)==1)id=word&65535u;
    else{
        road=sub_800245DC(m,word&65535u);
        if(road){
            word=rrj_read32(m,road+4);id=rrj_read32(m,road+12);progress=rrj_read32(m,position+8);
            distance=((word>>6)<<16)-progress;
            if((!mode && rrj_s32(progress)<rrj_s32(distance))
               || (rrj_s32(rrj_read32(m,position+4))<0 && mode==1)){
                distance=rrj_read32(m,position+8);id=rrj_read32(m,road+8);
            }
        }
    }
    if(id!=0xffffffffu){
        junction=sub_800245F4(m,id);
        if(junction){id=0xffffffffu;if(rrj_read32(m,junction+4)>=3)id=rrj_read32(m,junction);}
    }
    *output=id;return distance;
}
uint32_t sub_8003A5F4(RRJMemory *m,uint32_t position,uint32_t output,uint32_t mode)
{
    uint32_t id,result=rrj_nearest_junction_local(m,position,&id,mode);rrj_write32(m,output,id);return result;
}
/* 8003F4D8: first loaded route containing the requested member. */
uint32_t sub_8003F4D8(RRJMemory *m,uint32_t id)
{
    uint32_t i=0,record;
    if(rrj_s32(material_half(m,0x800D6182))<=0)return 0;
    do{
        record=rrj_read32(m,0x800D6194)+120u*i;
        if(sub_8003B4B0(m,record,id))return record;
        ++i;
    }while(rrj_s32(i)<rrj_s32(material_half(m,0x800D6182)));
    return 0;
}

/* 8003B024: assign a route and update the player's membership mask. */
void sub_8003B024(RRJMemory *m,uint32_t input,uint32_t player,uint32_t kind,uint32_t other)
{
    uint32_t table,id,flags,found=0,linked,count;
    if(!input)return;
    if(player){
        count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
        if(rrj_u16(rrj_at(m,player+172,2))<count
           || (material_byte(m,rrj_read32(m,player+1084)+1)&15u)!=2){
            count=material_half(m,0x800D6182);table=rrj_read32(m,0x800D6194);
            rrj_write32(m,input+256,table+120u*count);
        }else if(other)rrj_write32(m,input+256,rrj_read32(m,other+428));
        else{
            found=sub_8003F4D8(m,rrj_u16(rrj_at(m,input+188,2)));
            if(found)rrj_write32(m,input+256,found);
        }
        table=rrj_read32(m,player+428);id=rrj_u16(rrj_at(m,player+172,2));
        flags=rrj_u16(rrj_at(m,table+118,2));
        rrj_put16(rrj_at(m,table+118,2),flags|(1u<<(id&31u)));return;
    }
    if(kind==1){
        id=rrj_u16(rrj_at(m,input,2))&31u;linked=rrj_read32(m,0x8005B3A4)+628u*id;
        table=linked?rrj_read32(m,rrj_read32(m,linked+596)+428):0;
        rrj_write32(m,input+256,table);return;
    }
    if(other)found=sub_8003B4B0(m,rrj_read32(m,other+428),rrj_u16(rrj_at(m,input+188,2)));
    if(!found){
        found=sub_8003F4D8(m,rrj_u16(rrj_at(m,input+188,2)));
        if(found){rrj_write32(m,input+256,found);return;}
        if(!other)return;
    }
    rrj_write32(m,input+256,rrj_read32(m,other+428));
}

/* 8003B1C4: refresh the route after entering a road or junction.
 * Architectural v0 is unused by the sole wrapper and its callers. */
void sub_8003B1C4(RRJMemory *m,uint32_t input,uint32_t kind,uint32_t player)
{
    uint32_t word=rrj_read32(m,input+188),table,found,id,i,count,flags,mask;
    if((word>>16)==1){
        if(kind==1 || (!kind && player)){
            table=rrj_read32(m,input+256);
            if(!table || rrj_s32(rrj_read32(m,table+16))<=0)return;
            id=word&65535u;
            if(rrj_read32(m,table)==id){
                if(player){
                    mask=1u<<(rrj_u16(rrj_at(m,player+172,2))&31u);
                    flags=rrj_u16(rrj_at(m,table+118,2));rrj_put16(rrj_at(m,table+118,2),flags|mask);
                }
                return;
            }
            i=0;
            do{
                found=rrj_read32(m,table+100+4u*i);
                if(found!=0xffffffffu && found==id){
                    found=sub_8003F3B4(m,found);
                    if(found)rrj_write32(m,input+256,found);
                    if(player){
                        if(found){
                            table=rrj_read32(m,input+256);id=rrj_u16(rrj_at(m,player+172,2));
                        }else{
                            id=rrj_u16(rrj_at(m,player+172,2));table=rrj_read32(m,input+256);
                        }
                        mask=1u<<(id&31u);flags=rrj_u16(rrj_at(m,table+118,2));
                        rrj_put16(rrj_at(m,table+118,2),found?flags|mask:flags&~mask);
                    }
                    return;
                }
                count=rrj_read32(m,table+16);++i;
            }while(rrj_s32(i)<rrj_s32(count));
            if(player){
                id=rrj_u16(rrj_at(m,player+172,2));table=rrj_read32(m,input+256);
                flags=rrj_u16(rrj_at(m,table+118,2));rrj_put16(rrj_at(m,table+118,2),flags&~(1u<<(id&31u)));
            }
            return;
        }
        found=sub_8003F3B4(m,rrj_u16(rrj_at(m,input+188,2)));
        if(found)rrj_write32(m,input+256,found);return;
    }
    if(sub_8003B4B0(m,rrj_read32(m,input+256),word&65535u))return;
    if(kind>=2 || (player && !kind
       && (material_byte(m,rrj_read32(m,player+1084)+1)&15u)!=2
       && rrj_u16(rrj_at(m,player+172,2))>=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48))){
        found=sub_8003F4D8(m,rrj_u16(rrj_at(m,input+188,2)));
        if(found)rrj_write32(m,input+256,found);return;
    }
    table=rrj_read32(m,input+256);
    if(!table || !player || rrj_s32(rrj_read32(m,table+16))<=0)return;
    if(rrj_s32(rrj_nearest_junction_local(m,player+360,&id,0))>0x320000)return;
    if(rrj_s32(rrj_read32(m,table+16))<=0)return;
    i=0;
    do{
        found=rrj_read32(m,table+100+4u*i);
        if(found!=0xffffffffu && found==id){
            found=sub_8003F3B4(m,found);
            if(found && sub_8003F580(m,found,rrj_u16(rrj_at(m,player+360,2))))rrj_write32(m,input+256,found);
            return;
        }
        count=rrj_read32(m,table+16);++i;
    }while(rrj_s32(i)<rrj_s32(count));
}
/* 8003AF9C: select initial assignment versus route refresh. */
void sub_8003AF9C(RRJMemory *m,uint32_t input,uint32_t refresh,uint32_t other)
{
    uint32_t tag,kind,player=0;
    if(!input)return;
    tag=rrj_u16(rrj_at(m,input,2));kind=tag>>5;
    if(!kind)player=rrj_read32(m,0x8005B3A0)+1096u*tag;
    if(rrj_read32(m,input+256) && refresh)sub_8003B1C4(m,input,kind,player);
    else sub_8003B024(m,input,player,kind,other);
}

/* 8003B61C: distance along the assigned route, with junction projection. */
uint32_t sub_8003B61C(RRJMemory *m,uint32_t input)
{
    uint32_t count=material_half(m,0x800D6182),table=rrj_read32(m,input+256);
    uint32_t word,record,link,distance,progress,direction=0,marker,id,value,sign,piece,offset,local[8]={0};
    unsigned written;
    if(count==0xffffffffu)return 0;
    if(!table)return rrj_read32(m,input+196);
    word=rrj_read32(m,input+188);
    if((word>>16)==1){
        record=sub_8003F408(m,table,word&65535u);
        if(!record)goto fallback;
        count=rrj_read32(m,record+16);distance=rrj_read32(m,record+4);
        if(rrj_s32(count)>0){
            link=track_link_match(m,input+156,rrj_read32(m,record+8),rrj_read32(m,record+84),&direction,&written);
            if(link){
                if(link==rrj_read32(m,input+164))progress=rrj_read32(m,input+196);
                else{
                    local[0]=rrj_read32(m,input+156);local[1]=rrj_read32(m,input+160);local[2]=link;
                    local[3]=rrj_read32(m,rrj_read32(m,input+156)+52)+52u*material_half(m,link+8);
                    piece=rrj_follow_track_local(m,input,local,input+12);
                    offset=sub_800B6AAC(m,input+12,piece+14,piece+20);
                    progress=rrj_read32(m,piece+40)+offset;
                }
                if(rrj_s32(direction)>0)distance=distance-rrj_read32(m,record+8)+pause_asr(rrj_read32(m,link+12)-progress,4);
                else distance=distance-rrj_read32(m,record+8)+pause_asr(progress,4);
            }
        }
    }else{
        marker=rrj_read32(m,0x800D6188);id=rrj_read32(m,marker+12);
        if((id==rrj_read32(m,table) || id==0xffffffffu) && !(word>>16) && (word&65535u)==rrj_read32(m,marker)){
            direction=rrj_read32(m,marker+8);
            if((rrj_s32(direction)>0 && rrj_s32(rrj_read32(m,input+196))>=rrj_s32(rrj_read32(m,marker+4)))
               || (rrj_s32(direction)<0 && rrj_s32(rrj_read32(m,marker+4))>=rrj_s32(rrj_read32(m,input+196))))return 0;
            marker=rrj_read32(m,0x800D6188);value=rrj_read32(m,marker+4)-rrj_read32(m,input+196);
            sign=pause_asr(value,31);distance=pause_asr((sign+value)^sign,4);
        }else{
            record=sub_8003B4B0(m,table,rrj_u16(rrj_at(m,input+188,2)));
            if(!record)goto fallback;
            if(rrj_s32(rrj_read32(m,record+4))>0){
                progress=rrj_read32(m,record+8)-rrj_read32(m,input+196);id=material_half(m,record+14);
            }else{progress=rrj_read32(m,input+196);id=material_half(m,record+12);}
            distance=pause_asr(progress,4);
            if(id!=0xffffffffu){record=sub_8003F408(m,table,id);if(record)distance+=rrj_read32(m,record+4);}
        }
    }
    return rrj_s32(distance)<0?0:distance;
 fallback:
    distance=pause_asr(rrj_read32(m,input+196),4);return rrj_s32(distance)<0?0:distance;
}

/* 8003DCB8: opposite signs of projected zone boundaries. */
uint32_t sub_8003DCB8(RRJMemory *m,uint32_t actor,uint32_t zone)
{
    uint32_t nx=material_half(m,rrj_read32(m,actor+340)+14)<<4;
    uint32_t nz=material_half(m,rrj_read32(m,actor+340)+18)<<4;
    uint32_t x=rrj_read32(m,actor+184),z,ax,az,bx,bz;
    int64_t product=(int64_t)rrj_s32(nx)*rrj_s32(x-rrj_read32(m,zone+20));
    ax=(uint32_t)((uint64_t)product>>16);z=rrj_read32(m,actor+192);
    product=(int64_t)rrj_s32(nz)*rrj_s32(z-rrj_read32(m,zone+28));az=(uint32_t)((uint64_t)product>>16);
    product=(int64_t)rrj_s32(nx)*rrj_s32(x-rrj_read32(m,zone+8));bx=(uint32_t)((uint64_t)product>>16);
    product=(int64_t)rrj_s32(nz)*rrj_s32(z-rrj_read32(m,zone+16));bz=(uint32_t)((uint64_t)product>>16);
    return ((ax+az)^(bx+bz))>>31;
}

/* 8003A9D8: road-zone flags, counter and propagation to linked actors. */
uint32_t sub_8003A9D8(RRJMemory *m,uint32_t actor)
{
    uint32_t thresholds[6],i,j,flags,previous,descriptor,side=0,count,zone,kind,tag,speed,index,limit,node;
    for(i=0;i<6;++i)thresholds[i]=rrj_read32(m,0x80010DFCu+4*i);
    flags=rrj_read32(m,actor+36);descriptor=rrj_read32(m,actor+496);previous=(flags>>5)&3u;
    flags&=0xfffffc9fu;rrj_write32(m,actor+36,flags);
    if(descriptor)count=1;
    else{
        side=rrj_read32(m,actor+492);if(!side)return flags;
        if(rrj_s32(rrj_read32(m,actor+344))>=0)side+=2;
        count=material_byte(m,side+1);
    }
    if(count){
        if(!descriptor && side)descriptor=rrj_read32(m,rrj_read32(m,actor+328)+100)+4u*material_byte(m,side);
        for(i=0;i<count;++i,descriptor+=4){
            zone=rrj_read32(m,rrj_read32(m,actor+328)+96)+40u*material_half(m,descriptor);
            for(j=0;j<material_byte(m,descriptor+3);++j,zone+=40){
                if(sub_8003DCB8(m,actor,zone)){
                    kind=rrj_u16(rrj_at(m,zone+2,2))&15u;
                    switch(kind){
                    case 1:case 2:
                        flags=rrj_read32(m,actor+36);flags=(flags&0xfffffdffu)|(rrj_u16(rrj_at(m,zone+2,2))&512u);
                        rrj_write32(m,actor+36,flags);break;
                    case 3:rrj_write32(m,actor+36,rrj_read32(m,actor+36)|768u);break;
                    case 5:rrj_write32(m,actor+36,(rrj_read32(m,actor+36)&0xffffff9fu)|800u);break;
                    case 4:case 6:case 7:case 8:
                        tag=rrj_u16(rrj_at(m,actor+172,2));
                        if((tag>>5)!=4 || (tag&31u)<30){
                            speed=rrj_read32(m,actor+480);
                            if(pause_asr(speed,16)){
                                index=pause_asr(speed,20);index=rrj_s32(index)<0?0:(index>5?5:index);limit=thresholds[index];
                                flags=rrj_read32(m,actor+36);
                                if(((flags>>1)&15u)<limit)rrj_write32(m,actor+36,(flags&0xffffff9fu)|(previous?64u:0));
                                else if(previous){
                                    rrj_write32(m,actor+36,flags&0xffffffe1u);
                                    if(sub_8001FC58(m)&limit){
                                        rrj_write32(m,actor+36,rrj_read32(m,actor+36)&0xffffff9fu);
                                        index=sub_8001FC58(m)%limit;
                                        rrj_write32(m,actor+36,(rrj_read32(m,actor+36)&0xffffffe1u)|((index&15u)<<1));
                                    }
                                }else rrj_write32(m,actor+36,(flags&0xffffffe1u&0xffffff9fu)|64u);
                                flags=rrj_read32(m,actor+36);rrj_write32(m,actor+36,(flags&0xffffffe1u)|(((((flags>>1)&15u)+1u)&15u)<<1));
                            }
                        }
                        flags=rrj_read32(m,actor+36)|512u;rrj_write32(m,actor+36,flags);
                        kind=rrj_u16(rrj_at(m,zone+2,2))&15u;
                        rrj_write32(m,actor+36,(flags&0xfffffeffu)|((kind!=6 && kind!=8)?256u:0));break;
                    default:break;
                    }
                }
                if(rrj_read32(m,actor+36)&96u)break;
            }
            if(rrj_read32(m,actor+36)&96u)break;
        }
    }
    for(i=0;i<2;++i){
        node=rrj_read32(m,actor+56+8*i);
        while(node){
            flags=rrj_read32(m,node+36);flags=(flags&0xffffff9fu)|(rrj_read32(m,actor+36)&96u);
            rrj_write32(m,node+36,flags);node=rrj_read32(m,node+56+8*i);
        }
    }
    return 0;
}

/* 8001FCB0: signed approximate vector length, including wrapped abs edges. */
uint32_t sub_8001FCB0(uint32_t x,uint32_t y,uint32_t z)
{
    uint32_t sign,temp,sum;
    sign=pause_asr(x,31);x=(sign+x)^sign;
    sign=pause_asr(y,31);y=(sign+y)^sign;
    sign=pause_asr(z,31);z=(sign+z)^sign;
    if(rrj_s32(x)<rrj_s32(y)){temp=x;x=y;y=temp;}
    if(rrj_s32(x)<rrj_s32(z)){temp=x;x=z;z=temp;}
    sum=y+z;return x-pause_asr(x,4)+pause_asr(sum,2)+pause_asr(sum,3);
}

/* 8003BFE8: the original reads a pre-existing stack direction word.
 * Keep that input explicit until its caller contract is proved. */
void rrj_correct_road_with_prior(RRJMemory *m,uint32_t actor,uint32_t prior_direction)
{
    uint32_t root,record[8]={0},position[3]={0},projected[3],nearest,distance,best,header,i,entry,word,route,sub,piece,progress;
    uint32_t found,table,tag,index,offset,flags,k;uint16_t normal[3];
    if(rrj_u16(rrj_at(m,actor+362,2)) || !rrj_read32(m,actor+372) || !(rrj_read32(m,actor+388)&1u))return;
    root=rrj_read32(m,actor+328);record[0]=root;position[1]=prior_direction;
    distance=rrj_nearest_junction_local(m,actor+360,&nearest,0);
    if(nearest==0xffffffffu || rrj_s32(distance)>0x31ffff)return;
    piece=rrj_read32(m,actor+340);rrj_project_vector_local(m,piece+20,piece+14,rrj_read32(m,actor+348),projected);
    best=sub_8001FCB0(pause_asr(rrj_read32(m,actor+184)-projected[0],16),pause_asr(rrj_read32(m,actor+188)-projected[1],16),pause_asr(rrj_read32(m,actor+192)-projected[2],16));
    header=0;if(material_half(m,root+16)==1 && !rrj_read32(m,root+12))header=sub_80039AFC(m,rrj_read32(m,root));
    if(!header || rrj_s32(material_half(m,header+2))<=0)return;
    i=0;
    do{
        entry=header+8+24u*i;word=rrj_read32(m,actor+360);
        if(!(word>>16) && (word&65535u)==material_half(m,entry+4))goto next;
        route=sub_80039C90(m,root,material_half(m,entry+4));if(!route)return;
        sub=rrj_read32(m,root+48)+28u*material_half(m,route+20);
        if(rrj_s32(material_half(m,entry+2))>0){piece=rrj_read32(m,root+52)+52u*material_half(m,sub+8);progress=0;}
        else{piece=rrj_read32(m,root+52)+52u*(material_half(m,sub+8)+material_half(m,sub+10))-52u;progress=rrj_read32(m,piece+32);}
        record[1]=route;record[2]=sub;record[3]=piece;record[5]=progress;
        piece=rrj_follow_track_local(m,actor+172,record,actor+184);
        record[4]=sub_800B6AAC(m,actor+184,piece+2,piece+20);record[5]=sub_800B6AAC(m,actor+184,piece+14,piece+20);
        for(k=0;k<3;++k)normal[k]=rrj_u16(rrj_at(m,actor+450+2*k,2));
        rrj_update_track_direction_local(m,normal,record,position);
        rrj_project_vector_local(m,piece+20,piece+14,record[5],projected);
        distance=sub_8001FCB0(pause_asr(rrj_read32(m,actor+184)-projected[0],16),pause_asr(rrj_read32(m,actor+188)-projected[1],16),pause_asr(rrj_read32(m,actor+192)-projected[2],16));
        if(rrj_s32(distance)>=rrj_s32(best))goto next;
        word=rrj_read32(m,actor+360);if((word>>16) || (position[0]>>16) || word==position[0])goto next;
        for(k=0;k<8;++k)rrj_write32(m,actor+328+4*k,record[k]);
        for(k=0;k<3;++k)rrj_write32(m,actor+360+4*k,position[k]);
        (void)sub_8003DE28(m,actor,1,0,0xffffffffu);sub_8003DF54(m,actor,1,0xffffffffu);
        found=sub_8003F3B4(m,nearest);if(found)rrj_write32(m,actor+428,found);
        table=rrj_read32(m,actor+428);
        if(table){
            tag=rrj_u16(rrj_at(m,actor+172,2));
            if((tag>>5)<2){
                index=tag&31u;offset=rrj_read32(m,0x8005B1F8);if(rrj_s32(index)>=rrj_s32(offset))index-=offset;
                flags=rrj_u16(rrj_at(m,table+118,2));word=1u<<(index&31u);
                rrj_put16(rrj_at(m,table+118,2),found?flags|word:flags&~word);
            }
        }
        sub_8003AF9C(m,actor+172,1,0);rrj_write32(m,actor+324,sub_8003B61C(m,actor+172));return;
 next:++i;
    }while(rrj_s32(i)<rrj_s32(material_half(m,header+2)));
}

static void correct_actor_track(RRJMemory *m,uint32_t actor,uint32_t prior_direction,unsigned canonical)
{
    uint32_t old_id=material_half(m,rrj_read32(m,actor+336)+6),route,root,header,entry,selected=0,i,k;
    uint32_t best=0xffff0000u,delta[3],saved[3]={0},ax,az,sign,minimum,score,dot,sub,piece,value,mask,old_word;
    if(!rrj_u16(rrj_at(m,actor+362,2))){
        old_word=rrj_read32(m,actor+360);rrj_correct_road_with_prior(m,actor,prior_direction);
        /* A changed road must have a distinct sub-id for the caller to erase
         * the original uninitialized scratch magnitude. Keep this explicit. */
        if(canonical && old_word!=rrj_read32(m,actor+360)
           && old_id==material_half(m,rrj_read32(m,actor+336)+6))abort();
    }else{
        route=rrj_read32(m,actor+332);root=rrj_read32(m,actor+328);
        if(material_half(m,route+2)!=1 || material_half(m,route))goto tail;
        header=sub_80039AFC(m,rrj_read32(m,root));if(!header)return;
        for(i=0;rrj_s32(i)<rrj_s32(material_half(m,header+2));++i){
            entry=header+8+24u*i;
            for(k=0;k<3;++k)delta[k]=rrj_read32(m,actor+184+4*k)-rrj_read32(m,entry+12+4*k);
            sign=pause_asr(delta[0],31);ax=(delta[0]+sign)^sign;
            sign=pause_asr(delta[2],31);az=(delta[2]+sign)^sign;
            if(rrj_s32(ax)>0x5a8000 || rrj_s32(az)>0x5a8000)score=0x7fff0000u;
            else{minimum=rrj_s32(ax)<rrj_s32(az)?ax:az;score=ax+az-pause_asr(minimum+(minimum>>31),1);}
            if(best==0xffff0000u || rrj_s32(score)<rrj_s32(best)){
                selected=entry;best=score;for(k=0;k<3;++k)saved[k]=delta[k];
            }
        }
        if(!selected)return;
        dot=0;
        for(k=0;k<3;++k){
            int64_t product=(int64_t)rrj_s32(saved[k])*rrj_s32(material_half(m,selected+6+2*k)<<4);
            dot+=(uint32_t)((uint64_t)product>>16);
        }
        if(rrj_s32(dot)<-131){
            route=sub_80039C90(m,root,material_half(m,selected+4));if(!route)return;
            sub=rrj_read32(m,root+48)+28u*material_half(m,route+20);i=material_half(m,sub+8);
            if(rrj_s32(material_half(m,selected+2))<=0)i+=material_half(m,sub+10)-1u;
            piece=rrj_read32(m,root+52)+52u*i;
            sub_80036800(m,actor+184,piece,actor+344,actor+348);
            rrj_write32(m,actor+332,route);rrj_write32(m,actor+336,sub);rrj_write32(m,actor+340,piece);
            rrj_write32(m,actor+352,0);rrj_write32(m,actor+356,0);
        }
    }
 tail:
    mask=old_id==material_half(m,rrj_read32(m,actor+336)+6)?0xffffffffu:0;
    value=rrj_read32(m,actor+364);
    value=rrj_s32(value)>0?5u+(mask&(value-5u)):0xfffffffbu+(mask&(value+5u));rrj_write32(m,actor+364,value);
}
void rrj_correct_actor_track_with_prior(RRJMemory *m,uint32_t actor,uint32_t prior_direction)
{correct_actor_track(m,actor,prior_direction,0);}
void sub_800396A8(RRJMemory *m,uint32_t actor)
{correct_actor_track(m,actor,0,1);}

/* 80037104: projected track record and attached actor placement. */
uint32_t sub_80037104(RRJMemory *m,uint32_t actor)
{
    uint32_t record[8]={0},i,flag,scale,origin,linked,result,flags;
    for(i=0;i<4;++i)record[i]=rrj_read32(m,actor+328+4*i);
    flag=(rrj_read32(m,actor+568)&0x600u)!=0;
    scale=rrj_read32(m,actor+308);origin=actor+(flag?184u:504u);
    if(!flag)scale<<=1;
    (void)sub_8002EAD8(m,origin,actor+528,scale,actor+244);
    (void)rrj_follow_track_local(m,actor+172,record,actor+244);
    result=record[3];linked=rrj_read32(m,actor+856);rrj_write32(m,actor+256,result);
    if(!linked)return result;
    result=rrj_read32(m,actor+1088);if(!result)return 0;
    if(flag)origin=actor+184;
    else{
        (void)sub_8002EAD8(m,actor+504,actor+528,rrj_read32(m,actor+308),linked+504);
        origin=linked+504;
    }
    scale=rrj_read32(m,actor+304)+rrj_read32(m,linked+304);
    (void)sub_8002EAD8(m,origin,actor+516,scale,linked+184);
    if(rrj_read32(m,actor+560)&0x08000000u)flags=rrj_read32(m,linked+388)&~32u;
    else{sub_800396A8(m,linked);flags=rrj_read32(m,linked+388)|32u;}
    rrj_write32(m,linked+388,flags);return sub_8003701C(m,linked);
}

/* 80037450: ordered actor track/contact/route/surface update. */
uint32_t sub_80037450(RRJMemory *m,uint32_t actor)
{
    uint32_t distance;
    (void)sub_8003701C(m,actor);
    if(!(rrj_u16(rrj_at(m,actor+172,2))>>5))(void)sub_80037104(m,actor);
    (void)sub_8003DFF4(m,actor);sub_8003AF9C(m,actor+172,1,0);
    distance=sub_8003B61C(m,actor+172);rrj_write32(m,actor+324,distance);
    (void)sub_8003A9D8(m,actor);return (rrj_read32(m,actor+388)>>6)&1u;
}

/* 800374D4: optional correction, then the complete actor track update. */
uint32_t sub_800374D4(RRJMemory *m,uint32_t actor,uint32_t correct)
{
    uint32_t flags;
    if(correct){sub_800396A8(m,actor);flags=rrj_read32(m,actor+388)|32u;}
    else flags=rrj_read32(m,actor+388)&~32u;
    rrj_write32(m,actor+388,flags);return sub_80037450(m,actor);
}

/* 8003FA40: rotation MVMVA(sf=1,lm=0), one input column at a time.
 * The rotation is captured before stores; later columns may alias output. */
static uint32_t multiply_rotation(RRJMemory *m,uint32_t left,uint32_t right,const uint16_t *local_right,const uint16_t *local_left,uint32_t output)
{
    int32_t rotation[9],vector[3];uint32_t i,j,k;int64_t sum,value;
    for(i=0;i<9;++i)rotation[i]=local_left?(int32_t)(int16_t)local_left[i]:rrj_s32(material_half(m,left+2*i));
    for(j=0;j<3;++j){
        for(k=0;k<3;++k)vector[k]=local_right?(int32_t)(int16_t)local_right[3*k+j]:rrj_s32(material_half(m,right+6*k+2*j));
        for(i=0;i<3;++i){
            sum=0;for(k=0;k<3;++k)sum+=(int64_t)rotation[3*i+k]*vector[k];
            value=sum/4096-(sum<0 && sum%4096!=0);
            if(value>32767)value=32767;if(value< -32768)value= -32768;
            rrj_put16(rrj_at(m,output+6*i+2*j,2),(uint32_t)value);
        }
    }
    return output+2;
}

/* 8003FB34: axis/angle matrix, original table and intermediate truncation. */
static uint32_t axis_rotation(RRJMemory *m,uint32_t axis,uint32_t angle,uint32_t output,uint16_t *local_output)
{
    uint32_t v[3],square[3],sine[3],cross[3],cosine,sin_value,complement,product,diagonal[3],values[9],i,j,k;
    cosine=material_half(m,0x8005624c+4*(angle&4095u)+2)<<4;
    sin_value=material_half(m,0x8005624c+4*(angle&4095u))<<4;complement=65536u-cosine;
    for(i=0;i<3;++i)v[i]=material_half(m,axis+2*i);
    for(i=0;i<3;++i){
        square[i]=(uint32_t)sub_8001FC90(rrj_s32(v[i]<<4),rrj_s32(v[i]<<4));
        sine[i]=(uint32_t)sub_8001FC90(rrj_s32(v[i]<<4),rrj_s32(sin_value));
        j=(i+1)%3;k=(i+2)%3;product=v[j]*v[k];
        cross[i]=(uint32_t)sub_8001FC90(rrj_s32(pause_asr(product<<4,16)<<4),rrj_s32(complement));
        diagonal[i]=square[i]+(uint32_t)sub_8001FC90(rrj_s32(cosine),rrj_s32(65536u-square[i]));
    }
    values[0]=diagonal[0];values[1]=cross[2]+sine[2];values[2]=cross[1]-sine[1];
    values[3]=cross[2]-sine[2];values[4]=diagonal[1];values[5]=cross[0]+sine[0];
    values[6]=cross[1]+sine[1];values[7]=cross[0]-sine[0];values[8]=diagonal[2];
    for(i=0;i<9;++i){
        if(local_output)local_output[i]=(uint16_t)pause_asr(values[i],4);
        else rrj_put16(rrj_at(m,output+2*i,2),pause_asr(values[i],4));
    }
    return pause_asr(values[8],4);
}

/* 80028034: attach the table-selected effect set using actual slot/list code. */
uint32_t sub_80028034(RRJMemory *m,uint32_t actor)
{
    uint32_t variant=0,count,table,value,delta,id,slot,flags,stamp,i,kind,byte_value;
    kind=(rrj_u16(rrj_at(m,rrj_read32(m,actor)+14,2))&120u)>>3;
    if(kind==2){
        value=rrj_read32(m,actor+180);if(value<18)return 1;
        delta=20u-value;variant=value+(pause_asr(value-18u,31)&(18u-value))+(pause_asr(delta,31)&delta)-18u;
        count=6;table=0x800536dc;
    }else{count=4;table=0x800536e8;}
    if((rrj_read32(m,actor+36)>>27)&1u)return 1;
    for(i=0;i<count;++i,table+=2){
        id=sub_80027178(m);if(id==0xffffffffu)continue;
        slot=0x800d39b0+112u*id;flags=(rrj_read32(m,slot)&0xfffffc3fu)|0x140u;
        rrj_write32(m,slot,flags);flags=rrj_read32(m,slot);stamp=rrj_read32(m,rrj_read32(m,0x8005b2f8)+16);
        rrj_write32(m,slot+36,0);rrj_write32(m,slot+44,0);rrj_write32(m,slot+40,0);
        *(uint8_t *)rrj_at(m,slot+61,1)=0;rrj_write32(m,slot+48,stamp);
        byte_value=material_byte(m,table);*(uint8_t *)rrj_at(m,slot+96,1)=5;
        flags=(flags&0xffc03fffu)|(((byte_value+count*variant)&255u)<<14);rrj_write32(m,slot,flags);
        byte_value=material_byte(m,table+1);flags=(flags&0xffffc3ffu)|0x800u;
        rrj_write32(m,slot,flags);*(uint8_t *)rrj_at(m,slot+97,1)=(uint8_t)byte_value;
        (void)sub_800271CC(m,actor,id);
    }
    flags=rrj_read32(m,actor+36)|0x38000000u;rrj_write32(m,actor+36,flags);return flags;
}

/* 8008BA18: eight world-space actor corners with class-specific offsets. */
uint32_t sub_8008BA18(RRJMemory *m,uint32_t actor)
{
    uint32_t tag=rrj_u16(rrj_at(m,actor+172,2)),height=rrj_read32(m,actor+312),width=rrj_read32(m,actor+304);
    uint32_t depth,product,a[3],b[3],c[3],points[8][3],i,k,shift_all=0,shift_top=0;
    if(!(tag>>5) && rrj_read32(m,actor+180)<18u && rrj_read32(m,rrj_read32(m,actor+852)+604)>=2u){
        product=5u*height;height=pause_asr(product+(rrj_s32(product)<0?7u:0u),3);
    }
    depth=rrj_read32(m,actor+308);
    (void)rrj_scale_short_local(m,width,actor+432,a);
    (void)rrj_scale_short_local(m,height,actor+438,b);
    (void)rrj_scale_short_local(m,depth,actor+444,c);
    for(k=0;k<3;++k){
        points[0][k]=0u-a[k]-c[k];points[1][k]=a[k]-c[k];
        points[2][k]=a[k]+c[k];points[3][k]=c[k]-a[k];
        points[4][k]=0u-a[k]-b[k]-c[k];points[5][k]=a[k]-b[k]-c[k];
        points[6][k]=a[k]-b[k]+c[k];points[7][k]=0u-a[k]-b[k]+c[k];
    }
    tag=rrj_u16(rrj_at(m,actor+172,2));
    if((tag>>5)==3){shift_top=1;(void)rrj_scale_short_local(m,0x2000,actor+438,b);}
    else if((tag>>5)==4 && (tag&31u)>=30u){shift_all=1;(void)rrj_scale_long_local(m,0x8000,b);}
    for(i=0;i<8;++i){
        if(shift_all || (shift_top && i<4))for(k=0;k<3;++k)points[i][k]+=b[k];
        for(k=0;k<3;++k)rrj_write32(m,actor+196+12*i+4*k,points[i][k]+rrj_read32(m,actor+184+4*k));
    }
    return 0;
}

uint32_t sub_8003FA40(RRJMemory *m,uint32_t left,uint32_t right,uint32_t output)
{return multiply_rotation(m,left,right,NULL,NULL,output);}
uint32_t rrj_multiply_rotation_local(RRJMemory *m,uint32_t left,const uint16_t right[9],uint32_t output)
{return multiply_rotation(m,left,0,right,NULL,output);}
uint32_t sub_8003FB34(RRJMemory *m,uint32_t axis,uint32_t angle,uint32_t output)
{return axis_rotation(m,axis,angle,output,NULL);}
uint32_t rrj_axis_rotation_local(RRJMemory *m,uint32_t axis,uint32_t angle,uint16_t output[9])
{return axis_rotation(m,axis,angle,0,output);}

/* 8009432C: complete placement/activation flow; actual callees throughout. */
uint32_t sub_8009432C(RRJMemory *m,uint32_t actor)
{
    uint32_t angle=(uint32_t)sub_8001FC90(0x3c0000,11),flags=0,lateral=0,player=0;
    uint32_t global,descriptor,secondary,tag,count,rank,value,other,side,piece,link,type,i,k,x,y,z,t,u,speed;
    uint32_t distances[2];uint16_t matrix[9],axes[9];uint8_t event[8]={0};
    if(!actor)return angle;
    global=rrj_read32(m,0x8005b2f8);
    if(material_byte(m,global+4)==44 && !material_byte(m,global+57))rrj_put16(rrj_at(m,actor+320,2),0);
    if(!material_half(m,actor+320))return 0x80060000u;
    global=rrj_read32(m,0x8005b2f8);tag=rrj_u16(rrj_at(m,actor+172,2));
    if((material_byte(m,global+4)&1u) && tag>=rrj_read32(m,global+48)
       && material_byte(m,rrj_read32(m,actor+1084)+39)==254){
        flags=61;angle=(uint32_t)sub_8001FC90(0xa0000,11);rrj_write32(m,actor+560,rrj_read32(m,actor+560)|0x20000000u);
    }else{
        global=rrj_read32(m,0x8005b2f8);value=material_byte(m,global+57);
        if(value==1){
            flags=rrj_read32(m,rrj_read32(m,rrj_read32(m,0x8005b38c)+1084)+40)?67u:123u;
            lateral=rrj_read32(m,actor+344);angle=rrj_read32(m,actor+292);rrj_write32(m,actor+924,0);
        }else if(value>=3 && rrj_u16(rrj_at(m,actor+172,2))>=rrj_read32(m,global+48)
          && ((material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==(material_byte(m,rrj_read32(m,rrj_read32(m,0x8005b38c)+1084)+1)&15u))){
            flags=85;descriptor=rrj_read32(m,actor+1084);*(uint8_t *)rrj_at(m,descriptor,1)=(uint8_t)(material_byte(m,descriptor)|16u);
            angle=(uint32_t)sub_8001FC90(0x9b0000,11);
        }else{
            secondary=rrj_read32(m,actor+852);flags=1;
            if(rrj_read32(m,secondary+604)>=3 && material_half(m,secondary+320))flags=161;
            count=rrj_read32(m,rrj_read32(m,0x8005b2f8)+48);
            if(rrj_u16(rrj_at(m,actor+172,2))>=count){
                if(count==2){x=rrj_read32(m,0x8005b268);y=rrj_read32(m,0x8005b26c);player=rrj_read32(m,0x8005b268+4u*(rrj_s32(rrj_read32(m,y+324))<rrj_s32(rrj_read32(m,x+324))));}
                else player=rrj_read32(m,0x8005b38c);
                if(rrj_s32(rrj_read32(m,0x8005b1f8))>=(int32_t)material_byte(m,rrj_read32(m,player+1084)+39)){
                    descriptor=rrj_read32(m,actor+1084);
                    if(rrj_read32(m,descriptor+40)){
                        count=rrj_read32(m,rrj_read32(m,0x8005b2f8)+48);rank=material_byte(m,descriptor+39);
                        if(rrj_s32(pause_asr(8,(count-1u)&31u))<(int32_t)rank)flags=0;
                        else{
                            value=rrj_read32(m,rrj_read32(m,0x800d6188)+4);descriptor=rrj_read32(m,actor+1084);rrj_write32(m,actor+368,value);
                            rank=material_byte(m,descriptor+39);flags|=60u;rrj_write32(m,actor+560,rrj_read32(m,actor+560)|0x20000000u);
                            rrj_write32(m,actor+368,value+196608u*(rank>>1)*(1u-2u*(rank&1u)));
                        }
                    }
                }
            }
        }
    }
    if(!flags || !sub_8003A468(m,actor+360,actor+328)){
        global=rrj_read32(m,0x8005b2f8);tag=rrj_u16(rrj_at(m,actor+172,2));rrj_put16(rrj_at(m,actor+320,2),0);
        if(tag<rrj_read32(m,global+48))return 1;goto ai_tail;
    }
    link=sub_8003B4B0(m,rrj_read32(m,actor+428),rrj_read32(m,rrj_read32(m,actor+332)+12));
    (void)sub_8003DE28(m,actor,1,0,0xffffffffu);
    if(flags&4u){
        global=rrj_read32(m,0x8005b2f8);
        if(material_byte(m,global+57)>=3){side=((rrj_u16(rrj_at(m,actor+172,2))>>1)-1u)&1u;if(!side)angle=0u-angle;}
        else side=rrj_read32(m,global+48)==2 && rrj_read32(m,rrj_read32(m,0x800d6188))==32;
        value=2u*rrj_read32(m,actor+308);value=side?rrj_read32(m,actor+412)-value:rrj_read32(m,actor+400)+value;
        rrj_write32(m,actor+344,value);rrj_write32(m,actor+924,0);
    }
    value=(material_byte(m,actor+946)^128u)-128u;type=rrj_u16(rrj_at(m,actor+956+8u*(value-1u),2));
    if(flags&2u)rrj_write32(m,actor+344,lateral);
    else{
        global=rrj_read32(m,0x8005b2f8);tag=rrj_u16(rrj_at(m,actor+172,2));
        if(!(flags&4u) && !(tag>=rrj_read32(m,global+48) && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2) && type!=1)rrj_write32(m,actor+344,0);
        else if(rrj_u16(rrj_at(m,actor+172,2))>=rrj_read32(m,rrj_read32(m,0x8005b2f8)+48)
             && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2 && type==1)
            rrj_write32(m,actor+344,rrj_read32(m,actor+(rrj_s32(rrj_read32(m,actor+364))>0?416u:404u)));
    }
    piece=rrj_read32(m,actor+340);
    (void)sub_8002EAD8(m,piece+20,piece+14,rrj_read32(m,actor+348),actor+184);
    (void)sub_8002EAD8(m,actor+184,piece+2,rrj_read32(m,actor+344),actor+184);
    x=rrj_read32(m,actor+184);y=rrj_read32(m,actor+188);z=rrj_read32(m,actor+192);
    rrj_write32(m,actor+504,x);rrj_write32(m,actor+508,y);rrj_write32(m,actor+512,z);
    rrj_write32(m,actor+468,x);rrj_write32(m,actor+472,y);rrj_write32(m,actor+476,z);
    (void)sub_80037450(m,actor);sub_8003DF54(m,actor,1,0xffffffffu);
    for(i=0;i<5;++i)rrj_put16(rrj_at(m,actor+432+2*i,2),rrj_u16(rrj_at(m,piece+2+2*i,2)));
    x=rrj_u16(rrj_at(m,actor+438,2));y=rrj_u16(rrj_at(m,piece+12,2));rrj_put16(rrj_at(m,actor+438,2),0u-x);
    x=rrj_u16(rrj_at(m,actor+440,2));rrj_put16(rrj_at(m,actor+442,2),y);rrj_put16(rrj_at(m,actor+442,2),0u-y);rrj_put16(rrj_at(m,actor+440,2),0u-x);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+444+2*i,2),rrj_u16(rrj_at(m,piece+14+2*i,2)));
    if(link){rrj_write32(m,actor+364,rrj_read32(m,link+4));if(rrj_s32(rrj_read32(m,link+4))<0){
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+444+2*i,2),0u-rrj_u16(rrj_at(m,actor+444+2*i,2)));
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+432+2*i,2),0u-rrj_u16(rrj_at(m,actor+432+2*i,2)));
    }}else rrj_write32(m,actor+364,1);
    global=rrj_read32(m,0x8005b2f8);count=rrj_read32(m,global+48);
    if(rrj_u16(rrj_at(m,actor+172,2))>=count && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2){
        if(count==2){
            for(i=0;i<2;++i){
                other=rrj_read32(m,0x8005b268+4*i);x=material_half(m,actor+186)-material_half(m,other+186);y=material_half(m,actor+194)-material_half(m,other+194);
                t=pause_asr(x,31);x=(x+t)^t;t=pause_asr(y,31);y=(y+t)^t;if(rrj_s32(x)<rrj_s32(y)){t=x;x=y;y=t;}
                t=y+pause_asr(y,1);distances[i]=x-pause_asr(x,5)-pause_asr(x,7)+pause_asr(t,2)+pause_asr(t,6);
            }
            player=rrj_read32(m,0x8005b268+4u*(rrj_s32(distances[1])<rrj_s32(distances[0])));
        }else player=rrj_read32(m,0x8005b38c);
        if(rrj_read32(m,player+360)==rrj_read32(m,actor+360) && rrj_s32(rrj_read32(m,actor+364)^rrj_read32(m,player+364))<0){
            for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+444+2*i,2),0u-rrj_u16(rrj_at(m,actor+444+2*i,2)));
            for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+432+2*i,2),0u-rrj_u16(rrj_at(m,actor+432+2*i,2)));
            rrj_write32(m,actor+364,0u-rrj_read32(m,actor+364));
        }
    }
    if(flags&16u){(void)rrj_axis_rotation_local(m,piece+8,angle,matrix);(void)rrj_multiply_rotation_local(m,actor+432,matrix,actor+432);}
    for(i=0;i<9;++i)axes[i]=rrj_u16(rrj_at(m,actor+432+2*i,2));
    for(i=0;i<9;++i)rrj_put16(rrj_at(m,actor+516+2*i,2),axes[i]);
    (void)sub_8002090C(m,actor);
    if(flags&136u){
        value=rrj_read32(m,actor+668);
        if(flags&8u){rrj_write32(m,actor+636,28595);rrj_write32(m,actor+652,value+28595u);}
        else{rrj_write32(m,actor+652,102943);rrj_write32(m,actor+636,102943u-value);}
        (void)rrj_axis_rotation_local(m,actor+444,pause_asr(652u*rrj_read32(m,actor+652),16),matrix);
        (void)rrj_multiply_rotation_local(m,actor+432,matrix,actor+432);rrj_write32(m,actor+568,rrj_read32(m,actor+568)|256u);
    }
    if(flags&32u)(void)sub_80068D20(m,actor,rrj_read32(m,actor+852),0);
    (void)sub_8008BA18(m,actor);value=sub_80020018(m,material_half(m,actor+444)<<4,material_half(m,actor+448)<<4);rrj_write32(m,actor+292,value);
    x=material_half(m,0x8005624c+4*(value&4095u)+2);value=rrj_read32(m,actor+292);y=0x8005624c+4*(value&4095u);z=rrj_read32(m,actor+308);
    rrj_write32(m,actor+296,x<<4);rrj_write32(m,actor+300,material_half(m,y)<<4);(void)sub_8002EAD8(m,actor+184,actor+528,2u*z,actor+880);
    for(i=0;i<3;++i){axes[i]=rrj_u16(rrj_at(m,actor+444+2*i,2));axes[i+3]=rrj_u16(rrj_at(m,actor+432+2*i,2));}
    speed=rrj_read32(m,actor+924);rrj_put16(rrj_at(m,actor+450,2),axes[0]);rrj_write32(m,actor+480,speed);rrj_write32(m,actor+576,speed);
    rrj_put16(rrj_at(m,actor+452,2),axes[1]);rrj_put16(rrj_at(m,actor+454,2),axes[2]);for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+814+2*i,2),axes[i+3]);
    *(uint8_t *)rrj_at(m,actor+849,1)=(uint8_t)(rrj_s32(speed)>655360?material_byte(m,rrj_read32(m,actor+556)+444)-1u:0u);
    if((flags&64u) && rrj_u16(rrj_at(m,actor+956,2))==1){value=rrj_read32(m,actor+564);rrj_put16(rrj_at(m,actor+960,2),1);rrj_write32(m,actor+564,value&~512u);}
    if(!(flags&32u)){
        secondary=rrj_read32(m,actor+852);(void)sub_8001E0B4(m,secondary+328,actor+328,32);
        for(k=0;k<2;++k)for(i=0;i<3;++i)rrj_write32(m,secondary+(k?468u:184u)+4*i,rrj_read32(m,actor+184+4*i));
        value=rrj_u16(rrj_at(m,actor+320,2));rrj_write32(m,secondary+552,0);rrj_put16(rrj_at(m,secondary+320,2),value);
        if(!(flags&8u))(void)sub_8007EC30(m,actor);
        (void)sub_80012838(m,actor,secondary,2,0);speed=rrj_read32(m,actor+924);*(uint8_t *)rrj_at(m,actor+72,1)=0;
        rrj_write32(m,actor+480,speed);rrj_write32(m,actor+576,speed);value=(uint32_t)sub_8001FC90(rrj_s32(speed),rrj_s32(speed));speed=rrj_read32(m,actor+480);rrj_write32(m,actor+580,value);
        *(uint8_t *)rrj_at(m,actor+849,1)=(uint8_t)(rrj_s32(speed)>655360?material_byte(m,rrj_read32(m,actor+556)+444)-1u:0u);
        for(k=0;k<3;++k)for(i=0;i<3;++i){value=444u-6u*k+2u*i;rrj_put16(rrj_at(m,secondary+value,2),rrj_u16(rrj_at(m,actor+value,2)));}
        global=rrj_read32(m,0x8005b2f8);
        if(rrj_u16(rrj_at(m,actor+172,2))>=rrj_read32(m,global+48) && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2 && !rrj_read32(m,secondary+540))rrj_write32(m,secondary+540,sub_80012884(m,0x800ce170,secondary));
        (void)sub_80012858(m,rrj_read32(m,secondary+540),rrj_read32(m,0x800ce190));
        global=rrj_read32(m,0x8005b2f8);value=11;u=1;
        if(rrj_u16(rrj_at(m,actor+172,2))>=rrj_read32(m,global+48) && (material_byte(m,rrj_read32(m,actor+1084)+1)&15u)==2 && !(material_byte(m,actor+928)&16u)){value=4;u=0;}
        rrj_write32(m,secondary+604,u);(void)sub_800C4550(m,value,secondary,1);
    }
    rrj_write32(m,actor+568,rrj_read32(m,actor+568)|0x08000000u);
 ai_tail:
    if(rrj_u16(rrj_at(m,actor+172,2))<rrj_read32(m,rrj_read32(m,0x8005b2f8)+48))return 1;
    value=material_byte(m,rrj_read32(m,actor+1084)+1)&15u;if(value!=2)return value;
    if(material_half(m,actor+320) && (material_byte(m,actor+928)&16u)){(void)sub_800BCD10(m,actor);type=4;}
    else{
        (void)sub_8009DAF8(m,1);*(uint8_t *)rrj_at(m,actor+928,1)=(uint8_t)((material_byte(m,actor+928)|16u)&223u);
        sub_8003AF9C(m,actor+172,0,player);rrj_write32(m,rrj_read32(m,actor+1084)+40,0);(void)sub_800BCD10(m,actor);type=material_half(m,actor+320)?1u:4u;
    }
    rrj_put16(event,type);rrj_put16(event+2,224);(void)rrj_enqueue_actor_event_local(m,event,1,actor);return sub_80028034(m,actor);
}

/* Motion transition 800723FC: branch priority 40,20,80,100. Unsigned
 * intermediates retain the MIPS wrapping sign masks at integer extremes. */
uint32_t sub_800723FC(RRJMemory *m,uint32_t actor)
{
    uint32_t flags=rrj_read32(m,actor+0x238),secondary=rrj_read32(m,actor+0x354);
    uint32_t angle,rate,target,scale,value,sign,dot,linked;
    rrj_write32(m,actor+0x3A4,0);rrj_write32(m,actor+0x2E8,0);
    rrj_write32(m,actor+0x234,rrj_read32(m,actor+0x234)&0xC0018200u);
    if(flags&0x40){
        angle=rrj_read32(m,actor+0x28C);
        rrj_write32(m,actor+0x2D4,0);rrj_write32(m,actor+0x2AC,0);rrj_write32(m,actor+0x298,angle);
        target=rrj_s32(angle)>0?0x1921Fu:0u-0x1921Fu;
        if(rrj_read32(m,actor+0x238)&0x18000){
            rrj_write32(m,actor+0x248,0);scale=0x1AAAA;
            rate=(uint32_t)*(uint8_t *)rrj_at(m,0x800531F1,1)<<10;
            if(rate<0x3333)rate=0x3333;
            sign=(rrj_read32(m,actor+0x238)>>15)&1;
            if((sign && rrj_s32(angle)>0)||(!sign && rrj_s32(angle)<0)){
                if(rrj_s32(angle)<0)rate=0u-rate;
            }else if(!angle)rate=((0u-sign)&(2u*rate))-rate;
            else if(rrj_s32(angle)>0)rate=0u-rate;
            rrj_write32(m,actor+0x290,rate);
            rrj_write32(m,actor+0x294,rrj_s32(rate)<0?0xFFFF0000u:0x10000u);
            rrj_write32(m,actor+0x2A8,0);rrj_write32(m,actor+0x2B0,target);
        }else{
            scale=0x35555;
            value=(uint32_t)sub_8001FC90(rrj_s32(2u*(target-angle)),0xB1C71);
            rrj_write32(m,actor+0x294,value);rrj_write32(m,actor+0x290,0);
            if(rrj_read32(m,actor+0x238)&0x20000){
                rrj_write32(m,actor+0x2B0,target);
                value=(uint32_t)sub_8001FC90(rrj_s32(target-rrj_read32(m,actor+0x2A4)),0x35555);
                rrj_write32(m,actor+0x2A8,value);
            }
        }
        value=(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,actor+0x1E8)),rrj_s32(scale));
        rrj_write32(m,actor+0x248,0u-value);
        value=(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,actor+0x268)),rrj_s32(scale));
        rrj_write32(m,actor+0x26C,0u-value);
    }else if(flags&0x20){
        angle=rrj_read32(m,actor+0x2A4);target=0x1921F;
        if(rrj_read32(m,actor+0x358) && rrj_s32(angle)>0)target+=angle-0x138C3;
        if(rrj_s32(angle)<0 || (!angle && rrj_s32(rrj_read32(m,actor+0x28C))<=0))target=0u-target;
        rrj_write32(m,actor+0x2B0,target);rate=rrj_read32(m,actor+0x2A8);
        if(rrj_s32(rate^target)<0)rate=0u-rate;
        sign=rrj_s32(target)>0;
        if(!rate)rate=(sign<<18)-0x20000;
        rate*=4;
        if(rrj_s32(rate)<0){if(rrj_s32(rate)>-0x20000)rate=0xFFFE0000;}
        else if(rate<0x20000)rate=0x20000;
        value=angle+(pause_asr(angle+0x1921F,31)&(0u-0x1921F-angle));
        value+=pause_asr(0x1921F-angle,31)&(0x1921F-angle);
        rrj_write32(m,actor+0x2A4,value);
        dot=pause_asr(value,31);dot=(value+dot)^dot;
        if(rrj_s32(dot)>0x1921E)rate=0;
        rrj_write32(m,actor+0x2A8,rate);rrj_write32(m,actor+0x2AC,0);
        value=rrj_read32(m,actor+0x27C);
        if(!value || (rrj_read32(m,actor+0x358) && rrj_s32(value)>0))rrj_write32(m,actor+0x27C,sign?0x41u:0u-0x41u);
        rrj_write32(m,actor+0x238,rrj_read32(m,actor+0x238)&0xFFFDFFFFu);
    }else if(flags&0x80){
        value=rrj_read32(m,actor+0x27C);sign=pause_asr(value,31);value=(value+sign)^sign;
        rrj_write32(m,actor+0x2D4,rrj_s32(value)>0x8000?0xCCCu:0u);
        rrj_write32(m,actor+0x2AC,0);rrj_write32(m,actor+0x2B0,0);
        rrj_write32(m,actor+0x2A8,0u-rrj_read32(m,actor+0x2A4));
        if(rrj_read32(m,secondary+0x25C)<2){
            rrj_put16(rrj_at(m,secondary+0x1C8,2),rrj_u16(rrj_at(m,actor+0x1C2,2)));
            rrj_put16(rrj_at(m,secondary+0x1CA,2),rrj_u16(rrj_at(m,actor+0x1C4,2)));
            value=rrj_u16(rrj_at(m,actor+0x1C6,2));
            rrj_write32(m,secondary+0x228,rrj_read32(m,secondary+0x228)|0x208000);
            rrj_put16(rrj_at(m,secondary+0x1CC,2),(uint16_t)value);
        }
    }else if(flags&0x100){
        rrj_write32(m,actor+0x2AC,0);dot=sub_8002E698(m,actor+0x210,rrj_read32(m,actor+0x154)+2);
        if(rrj_read32(m,actor+0x238)&0x400000)dot=0u-dot;
        rrj_write32(m,actor+0x2A8,rrj_s32(dot)>0?0xFFFFC000u:0x4000u);
        angle=rrj_read32(m,actor+0x28C);
        rrj_write32(m,actor+0x268,0);rrj_write32(m,actor+0x26C,0);rrj_write32(m,actor+0x270,0);
        linked=rrj_read32(m,actor+0x358);
        rrj_write32(m,actor+0x28C,rrj_s32(angle)>0?0x1921Fu:0u-0x1921Fu);
        rrj_write32(m,actor+0x290,0);rrj_write32(m,actor+0x294,0);rrj_write32(m,actor+0x280,0);
        rrj_write32(m,actor+0x1E8,0);rrj_write32(m,actor+0x248,0);if(linked)rrj_write32(m,linked+0x1E8,0);
        value=rrj_read32(m,actor+0x1E0);rrj_write32(m,actor+0x2C0,0);rrj_write32(m,actor+0x25C,0);rrj_write32(m,actor+0x2D4,0x30000);
        (void)sub_8002EE50(m,value,actor+0x1C2,actor+0x1C8);
        rrj_write32(m,actor+0x238,rrj_read32(m,actor+0x238)&0xEFFFFFFFu);
    }
    value=rrj_read32(m,actor+0x238)&0xFFFFF7FFu;rrj_write32(m,actor+0x238,value);return value;
}


uint32_t rrj_multiply_rotation_local_left(RRJMemory *m,const uint16_t left[9],uint32_t right,uint32_t output)
{return multiply_rotation(m,0,right,NULL,left,output);}

/* 8004D2A4/P0043: negative angles use the positive-index table with sine
 * negation. Preserve low-word multiplication and negate BEFORE sra 12. */
static void euler_sincos(RRJMemory *m,uint16_t raw,uint32_t *sine,uint32_t *cosine)
{
    int32_t angle=(int32_t)(int16_t)raw;
    uint32_t index=(angle<0?0u-(uint32_t)angle:(uint32_t)angle)&4095u;
    *sine=material_half(m,0x8005624C+4*index);
    *cosine=material_half(m,0x8005624E+4*index);
    if(angle<0)*sine=0u-*sine;
}
static void euler_store(RRJMemory *m,uint32_t output,uint16_t *local,unsigned index,uint32_t value)
{
    if(local)local[index]=(uint16_t)value;
    else rrj_put16(rrj_at(m,output+2*index,2),value);
}
static void euler_rotation(RRJMemory *m,const uint16_t angles[3],uint32_t output,uint16_t *local)
{
    uint32_t sx,cx,sy,cy,sz,cz,czn,szn;
    euler_sincos(m,angles[0],&sx,&cx);euler_sincos(m,angles[1],&sy,&cy);
    euler_store(m,output,local,2,sy);
    euler_store(m,output,local,5,pause_asr(0u-cy*sx,12));
    euler_store(m,output,local,8,pause_asr(cy*cx,12));
    euler_sincos(m,angles[2],&sz,&cz);
    euler_store(m,output,local,0,pause_asr(cz*cy,12));
    euler_store(m,output,local,1,pause_asr(0u-sz*cy,12));
    czn=pause_asr(cz*(0u-sy),12);
    euler_store(m,output,local,3,pause_asr(sz*cx,12)-pause_asr(czn*sx,12));
    euler_store(m,output,local,6,pause_asr(sz*sx,12)+pause_asr(czn*cx,12));
    szn=pause_asr(sz*(0u-sy),12);
    euler_store(m,output,local,4,pause_asr(cz*cx,12)+pause_asr(szn*sx,12));
    euler_store(m,output,local,7,pause_asr(cz*sx,12)-pause_asr(szn*cx,12));
}
uint32_t sub_8004D2A4(RRJMemory *m,uint32_t angles,uint32_t output)
{
    uint16_t captured[3];unsigned i;
    for(i=0;i<3;++i)captured[i]=rrj_u16(rrj_at(m,angles+2*i,2));
    euler_rotation(m,captured,output,NULL);return output;
}
void rrj_euler_rotation_local(RRJMemory *m,const uint16_t angles[3],uint16_t output[9])
{
    uint16_t captured[3];unsigned i;for(i=0;i<3;++i)captured[i]=angles[i];
    euler_rotation(m,captured,0,output);
}

/* GTE OP sf=1,lm=0 result consumed by 80071D24. All sources are captured
 * before writes. Persistent GTE registers are audited separately at callers. */
static void motion_outer_product(RRJMemory *,uint32_t,uint32_t,uint32_t);
static void motion_outer_product_ref(RRJMemory *m,uint32_t left,const uint16_t *local_left,uint32_t right,uint32_t output)
{
    int32_t a[3],b[3];int64_t v;unsigned i,j,k;
    for(i=0;i<3;++i){a[i]=local_left?(int16_t)local_left[i]:rrj_s32(material_half(m,left+2*i));b[i]=rrj_s32(material_half(m,right+2*i));}
    for(i=0;i<3;++i){
        j=(i+1)%3;k=(i+2)%3;v=(int64_t)a[j]*b[k]-(int64_t)a[k]*b[j];
        v=v/4096-(v<0 && v%4096!=0);if(v>32767)v=32767;if(v< -32768)v= -32768;
        rrj_put16(rrj_at(m,output+2*i,2),(uint32_t)v);
    }
}
uint32_t sub_80071D24(RRJMemory *m,uint32_t actor)
{
    static const uint16_t clear[]={0x290,0x304,0x26C,0x270,0x280,0x2AC,0x248,0x2E8,0x300,0x260,0x2A8};
    uint32_t value=rrj_read32(m,actor+0x1E0),a,b,c,sign,n,d,np,dp,dot,secondary;unsigned i;
    uint16_t angles[3],matrix[9],copied[6];
    for(i=0;i<sizeof(clear)/sizeof(clear[0]);++i)rrj_write32(m,actor+clear[i],0);
    (void)sub_8002EE50(m,value,actor+0x1C2,actor+0x1C8);
    if(rrj_read32(m,actor+0x238)&0x400){
        a=sub_8001FF3C(m,0u-(material_half(m,actor+0x1BE)<<4));
        if(rrj_u16(rrj_at(m,0x8005624E+4*(a&4095u),2))){
            b=sub_80020018(m,material_half(m,actor+0x1BC)<<4,material_half(m,actor+0x1C0)<<4);
            c=sub_80020018(m,material_half(m,actor+0x1B2)<<4,material_half(m,actor+0x1B8)<<4);
        }else{
            b=sub_80020018(m,material_half(m,actor+0x210)<<4,material_half(m,actor+0x214)<<4);
            c=pause_asr(652u*rrj_read32(m,actor+0x27C),16);
        }
        sign=pause_asr(c,31);value=(c+sign)^sign;
        if(rrj_s32(value)>0x400){
            a=(rrj_s32(a)<0?0xFFFFF800u:0x800u)-a;
            b+=rrj_s32(b)>0?0xFFFFF800u:0x800u;c+=rrj_s32(c)>0?0xFFFFF800u:0x800u;
        }
        rrj_write32(m,actor+0x268,pause_asr(25736u*a,8));
        rrj_write32(m,actor+0x2B4,pause_asr(25736u*b,8));rrj_write32(m,actor+0x27C,pause_asr(25736u*c,8));
        value=material_half(m,actor+0x1C4)<<4;sign=pause_asr(value,31);value=(value+sign)^sign;d=0x1E0000;
        if(rrj_s32(value)<=0xB4FC)d=(uint32_t)sub_8001FC90(rrj_s32(value),0x279A9D)+0x20000;
        value=rrj_read32(m,actor+0x1CC);n=(uint32_t)sub_8001FC90(rrj_s32(value),rrj_s32(value));np=rrj_s32(n)>0;dp=rrj_s32(d)>0;
        value=sub_80010028(np?n:0u-n,dp?d:0u-d);if(np!=dp)value=0u-value;
        if(rrj_s32(value)<=0x9D086)value=0x9D087;rrj_write32(m,actor+0x1E4,value);
    }else{
        value=rrj_read32(m,actor+0x268);rrj_write32(m,actor+0x25C,0);rrj_write32(m,actor+0x2F4,0);
        angles[0]=(uint16_t)(0u-pause_asr(652u*value,16));angles[1]=0;
        angles[2]=(uint16_t)(0u-pause_asr(652u*rrj_read32(m,actor+0x28C),16));
        rrj_euler_rotation_local(m,angles,matrix);(void)rrj_multiply_rotation_local_left(m,matrix,actor+0x204,actor+0x1B0);
        dot=sub_8002E698(m,actor+0x1BC,actor+0x31C);sign=pause_asr(dot,31);value=(dot+sign)^sign;
        if(rrj_s32(value)>0xF0A3){
            for(i=0;i<3;++i)copied[i]=rrj_u16(rrj_at(m,actor+0x1B6+2*i,2));
            for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+0x322+2*i,2),copied[i]);
        }else{motion_outer_product(m,actor+0x1BC,actor+0x31C,actor+0x322);(void)sub_8002E468(m,actor+0x322);}
        motion_outer_product(m,actor+0x31C,actor+0x322,actor+0x328);
        value=sub_8001FF3C(m,dot);
        for(i=0;i<3;++i){copied[i]=rrj_u16(rrj_at(m,actor+0x328+2*i,2));copied[i+3]=rrj_u16(rrj_at(m,actor+0x322+2*i,2));}
        rrj_write32(m,actor+0x2B4,pause_asr(25736u*value,8));rrj_write32(m,actor+0x268,0);
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+0x26C+2*i,2),copied[i]);
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+0x274+2*i,2),copied[i+3]);
        value=sub_8001FF3C(m,sub_8002E698(m,actor+0x1B6,actor+0x322));
        rrj_write32(m,actor+0x2C4,pause_asr(25736u*(0x400u-value),8));
        secondary=rrj_read32(m,actor+0x354);value=rrj_read32(m,secondary+0x228);
        if(rrj_read32(m,secondary+0x25C)<2)value|=0x18000;rrj_write32(m,secondary+0x228,value);
        if(*(uint8_t *)rrj_at(m,rrj_read32(m,actor+0x354)+0x23C,1)&16){
            secondary=rrj_read32(m,rrj_read32(m,actor+0x358)+0x354);value=rrj_read32(m,secondary+0x228);
            if(rrj_read32(m,secondary+0x25C)<2)value|=0x18000;rrj_write32(m,secondary+0x228,value);
        }
        rrj_write32(m,actor+0x2D4,0);
    }
    (void)sub_8002EAD8(m,actor+0xB8,actor+0x1B6,0u-0x570A,actor+0x310);
    if(rrj_read32(m,actor+0x358) && rrj_read32(m,actor+0x440))
        (void)sub_8002EAD8(m,actor+0x310,actor+0x1B0,2u*rrj_read32(m,actor+0x130),actor+0x310);
    rrj_write32(m,actor+0x238,rrj_read32(m,actor+0x238)&0xEFC7F7FFu);(void)sub_8002076C(m,actor);
    value=rrj_read32(m,actor+0x234);secondary=rrj_read32(m,actor+0x354);rrj_write32(m,actor+0x234,value&0xE7CE0383u);
    value=rrj_read32(m,secondary+0x228)&0xFFFFDFFFu;rrj_write32(m,secondary+0x228,value);return value;
}

/* Actual movement producer 8007F0BC, including shared scratchpad append.
 * Components and products retain their individual fixed-point truncations. */
uint32_t sub_8007F0BC(RRJMemory *m,uint32_t actor,uint32_t delta)
{
    uint32_t flags=rrj_read32(m,actor+0x238),speed=rrj_read32(m,actor+0x240),old=rrj_read32(m,actor+0x1E0);
    uint32_t road_local,inhibit,special,normal,rotation_delta,rate,value,limit,road,sum,index,cosine,sine;unsigned i,j;
    if(!(flags&0x600?old:speed) && !(rrj_read32(m,actor+0x234)&0x8000) && !(flags&15) && !rrj_read32(m,actor+0x1E8) && !rrj_read32(m,actor+0x2A4))return 0;
    if(flags&0x600){
        if(rrj_read32(m,actor+0x238)&0x800){
            (void)sub_80071D24(m,actor);rrj_write32(m,actor+0x238,rrj_read32(m,actor+0x238)|0x08000000);
            rrj_write32(m,actor+0x234,rrj_read32(m,actor+0x234)|0x00800000);
        }
        (void)sub_8002E570(m,actor+0x310,actor+0x1C8,delta,actor+0x310);
        (void)sub_8002EAD8(m,actor+0x310,actor+0x1B6,0x570A,actor+0xB8);
        if(rrj_read32(m,actor+0x358) && rrj_read32(m,actor+0x440))
            (void)sub_8002EAD8(m,actor+0xB8,actor+0x1B0,0u-2u*rrj_read32(m,actor+0x130),actor+0xB8);
    }else{
        for(i=0;i<3;++i)rrj_write32(m,actor+0x31C+4*i,rrj_read32(m,actor+0xB8+4*i)-rrj_read32(m,actor+0x1F8+4*i));
        (void)sub_8002EE50(m,speed,actor+0x1C2,actor+0x1C8);
        flags=rrj_read32(m,actor+0x238);
        if((flags&0x800) && (flags&0x1E0)){(void)sub_800723FC(m,actor);rrj_write32(m,actor+0x238,rrj_read32(m,actor+0x238)|0x08000000);}
        flags=rrj_read32(m,actor+0x234);road_local=(flags>>18)&1;
        if((rrj_read32(m,actor+0x238)&15) || (flags&8)){
            (void)sub_8002E570(m,actor+0x310,actor+0x1C8,delta,actor+0x310);
            (void)sub_8002EAD8(m,actor+0x310,actor+0x210,0u-rrj_read32(m,actor+0x134),actor+0x1F8);
        }else if(road_local){
            for(j=0;j<2;++j){
                road=rrj_read32(m,actor+0x340);sum=0;
                for(i=0;i<3;++i)sum+=(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,actor+0x1C8+4*i)),rrj_s32(material_half(m,road+0x104+12*j+2*i)<<4));
                value=(uint32_t)sub_8001FC90(rrj_s32(sum),rrj_s32(delta));
                rrj_write32(m,actor+0x308+4*j,rrj_read32(m,actor+0x308+4*j)+value);
            }
        }else (void)sub_8002E570(m,actor+0x1F8,actor+0x1C8,delta,actor+0x1F8);
        flags=rrj_read32(m,actor+0x238);inhibit=(flags>>14)&1;
        for(i=0;i<3;++i)rrj_write32(m,actor+0xB8+4*i,rrj_read32(m,actor+0x1F8+4*i)+rrj_read32(m,actor+0x31C+4*i));
        special=((flags&31)||(rrj_read32(m,actor+0x234)&0x3400)) && !inhibit;
        normal=(!(rrj_read32(m,actor+0x238)&0x600) && rrj_read32(m,actor+0x240)) || inhibit || road_local || rrj_read32(m,actor+0x1E8) || rrj_read32(m,actor+0x2A4);
        rotation_delta=delta;
        if((rrj_read32(m,actor+0x238)&15) && !inhibit){
            value=rrj_read32(m,actor+0x2D4)+delta;limit=rrj_read32(m,actor+0x2D8);rrj_write32(m,actor+0x2D4,value);
            if(rrj_s32(limit)<rrj_s32(value))rotation_delta=delta+limit-value;
        }
        if(special){
            rate=rrj_read32(m,actor+0x2E8);if(!rate)rate=rrj_read32(m,actor+0x1E8);
            if(rate){
                value=(uint32_t)sub_8001FC90(rrj_s32(rate),rrj_s32(rotation_delta));index=((163u*value)>>14)&4095;
                cosine=material_half(m,0x8005624E+4*index)<<4;sine=material_half(m,0x8005624C+4*index)<<4;
                (void)sub_8002EB78(m,actor+0x210,actor+0x204,actor+0x210,cosine,sine);
            }
        }else if(normal){
            if(rrj_s32(rrj_read32(m,actor+0x24C))<=0 && rrj_s32(rrj_read32(m,actor+0x240))<0x3334 && !(rrj_read32(m,actor+0x230)&0x300))rrj_write32(m,actor+0x240,0);
            if(rrj_read32(m,actor+0x1E8) || rrj_read32(m,actor+0x2A4) || road_local){
                rate=rrj_read32(m,actor+0x1E8)+rrj_read32(m,actor+0x2E8);
                value=(uint32_t)sub_8001FC90(rrj_s32(rate),rrj_s32(delta))+rrj_read32(m,actor+0x2A4);index=((163u*value)>>14)&4095;
                cosine=material_half(m,0x8005624E+4*index)<<4;sine=material_half(m,0x8005624C+4*index)<<4;
                (void)sub_8002EB78(m,actor+0x1C2,actor+0x32E,actor+0x210,cosine,sine);
            }
        }
        (void)sub_8002EAD8(m,actor+0x1F8,actor+0x210,0u-rrj_read32(m,actor+0x134),actor+0x1F8);
    }
    value=rrj_read32(m,0x1F800000);rrj_write32(m,value,actor);
    value=rrj_read32(m,0x1F800000)+4;rrj_write32(m,0x1F800000,value);return value;
}

/* 800B6E08: strict convex polygon containment on two projected axes. */
static uint32_t polygon_contains(RRJMemory *m,RRJTrackWords point,RRJTrackWords vertices,uint32_t count,uint32_t axis)
{
    uint32_t x=axis+1,y,prior=count-1,i,current=0,previous,px,py,dx,dy,first,second;
    x=rrj_s32(x)<3?x:0;y=x+1;y=rrj_s32(y)<3?y:0;x<<=2;y<<=2;
    if(rrj_s32(count)<=0)return 1;
    for(i=0;rrj_s32(i)<rrj_s32(count);++i,current+=12){
        previous=12u*prior;
        py=track_read(m,vertices,previous+y);px=track_read(m,vertices,previous+x);
        dy=track_read(m,vertices,current+y)-py;dx=track_read(m,vertices,current+x)-px;
        first=(uint32_t)sub_8001FC90(rrj_s32(dy),rrj_s32(track_read(m,point,x)-px));
        second=(uint32_t)sub_8001FC90(rrj_s32(dx),rrj_s32(track_read(m,point,y)-track_read(m,vertices,previous+y)));
        if(rrj_s32(first-second)>=0)return 0;
        prior=i;
    }
    return 1;
}
uint32_t sub_800B6E08(RRJMemory *m,uint32_t point,uint32_t vertices,uint32_t count,uint32_t axis)
{return polygon_contains(m,track_ram(point),track_ram(vertices),count,axis);}
uint32_t rrj_polygon_contains_local(RRJMemory *m,uint32_t point[3],uint32_t *vertices,uint32_t count,uint32_t axis)
{return polygon_contains(m,track_local(point),track_local(vertices),count,axis);}
/* 800B6844: wrapped fixed-point Newell sum, rescale, normalize, narrow. */
static uint32_t polygon_normal(RRJMemory *m,RRJTrackWords vertices,uint32_t count,uint16_t output[3])
{
    uint32_t vector[3]={0,0,0},i=0,j,a,b,x,y,sign,large;
    if(rrj_s32(count-1)>0)do{
        a=12u*i;b=a+12;
        for(j=0;j<3;++j){
            x=((j+1)%3)*4;y=((j+2)%3)*4;
            vector[j]+=(uint32_t)sub_8001FC90(rrj_s32(track_read(m,vertices,a+x)-track_read(m,vertices,b+x)),rrj_s32(track_read(m,vertices,a+y)+track_read(m,vertices,b+y)));
        }
        ++i;
    }while(rrj_s32(i)<rrj_s32(count-1));
    a=12u*i;
    for(j=0;j<3;++j){
        x=((j+1)%3)*4;y=((j+2)%3)*4;
        vector[j]+=(uint32_t)sub_8001FC90(rrj_s32(track_read(m,vertices,a+x)-track_read(m,vertices,x)),rrj_s32(track_read(m,vertices,a+y)+track_read(m,vertices,y)));
    }
    for(;;){
        large=0;
        for(j=0;j<3;++j){sign=pause_asr(vector[j],31);if(rrj_s32((sign+vector[j])^sign)>0x5A8000)large=1;}
        if(!large)break;
        for(j=0;j<3;++j)vector[j]=pause_asr(vector[j],1);
    }
    (void)rrj_normalize_vector32(m,vector);
    for(j=0;j<3;++j)output[j]=(uint16_t)pause_asr(vector[j],4);
    return pause_asr(vector[2],4);
}
uint32_t sub_800B6844(RRJMemory *m,uint32_t vertices,uint32_t count,uint32_t output)
{
    uint16_t normal[3];uint32_t i,result=polygon_normal(m,track_ram(vertices),count,normal);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,output+2*i,2),normal[i]);return result;
}
uint32_t rrj_polygon_normal_local(RRJMemory *m,uint32_t *vertices,uint32_t count,uint16_t output[3])
{return polygon_normal(m,track_local(vertices),count,output);}

static uint32_t surface_div(uint32_t v,uint32_t bits)
{return pause_asr(v+(rrj_s32(v)<0?((1u<<bits)-1u):0),bits);}
static uint32_t surface_abs(uint32_t v)
{uint32_t sign=pause_asr(v,31);return (v+sign)^sign;}
static uint32_t surface_geometry(RRJMemory *m,uint32_t position,uint32_t direction,RRJTrackWords geometry,RRJTrackWords vertex_output,RRJTrackWords origin,RRJTrackWords selection)
{
    uint32_t bank=(track_read(m,selection,0)&0x80000000u)?12u:0u,end=bank+12;
    uint32_t slot=track_read(m,selection,0)&63u,attempt,handle,header,indices,group,tries,count,i,j;
    uint32_t point[3],vertices[18],minimum[2],maximum[2],start,index,v,best,chosen,x,z,tmp,distance,edge[3],normal[3],dot,cross,entry,offset,base,upper;
    for(attempt=bank;attempt<end;++attempt,++slot){
        if(slot<bank || slot>=end)slot=bank;
        handle=0x800D87EC+112u*slot;
        if(rrj_read32(m,handle-4)==0xffffffffu)continue;
        header=rrj_read32(m,handle);
        track_write(m,vertex_output,0,rrj_read32(m,header+52)+4);
        header=rrj_read32(m,handle);indices=rrj_read32(m,header+44);
        track_write(m,origin,0,rrj_read32(m,header+8)<<10);
        track_write(m,origin,4,rrj_read32(m,rrj_read32(m,handle)+12)<<10);
        track_write(m,origin,8,rrj_read32(m,rrj_read32(m,handle)+16)<<10);
        for(i=0;i<3;++i)point[i]=rrj_read32(m,position+4*i)-track_read(m,origin,4*i);
        group=(track_read(m,selection,0)>>6)&7u;header=rrj_read32(m,handle);
        for(tries=0;tries<rrj_u16(rrj_at(m,header+6,2));++tries,++group){
            if(group>=rrj_u16(rrj_at(m,header+6,2)))group=0;
            start=material_half(m,indices+2*group+2);
            count=material_half(m,indices+2*group+4)-start;
            minimum[0]=minimum[1]=0x3fff0000;maximum[0]=maximum[1]=0xc0010000;
            /* Original storage is six vertices; valid hulls contain edge pairs. */
            if(rrj_s32(count)>6 || (rrj_s32(count)>0 && (count&1u)))abort();
            for(i=0;rrj_s32(i)<rrj_s32(count);++i){
                index=material_half(m,indices+2*(material_half(m,indices+2*group+2)+i));
                for(j=0;j<3;++j)vertices[3*(count-1-i)+j]=material_half(m,track_read(m,vertex_output,0)+8*index+2*j)<<10;
                for(j=0;j<2;++j){v=vertices[3*(count-1-i)+2*j];if(rrj_s32(v)<rrj_s32(minimum[j]))minimum[j]=v;if(rrj_s32(v)>rrj_s32(maximum[j]))maximum[j]=v;}
            }
            if(rrj_s32(point[0])>=rrj_s32(minimum[0]) && rrj_s32(point[0])<=rrj_s32(maximum[0]) &&
               rrj_s32(point[2])>=rrj_s32(minimum[1]) && rrj_s32(point[2])<=rrj_s32(maximum[1]) &&
               rrj_polygon_contains_local(m,point,vertices,count,1)){
                track_write(m,selection,0,(track_read(m,selection,0)&0xfffff800u)|slot|(group<<6));
                chosen=0;best=0x3fff0000;
                for(i=0;i<count;i+=2){
                    x=surface_abs(point[0]-surface_div(vertices[3*i]+vertices[3*(i+1)],1));
                    z=surface_abs(point[2]-surface_div(vertices[3*i+2]+vertices[3*(i+1)+2],1));
                    if(rrj_s32(x)<rrj_s32(z)){tmp=x;x=z;z=tmp;}
                    tmp=z+pause_asr(z,1);distance=x-pause_asr(x,5)-pause_asr(x,7)+pause_asr(tmp,2)+pause_asr(tmp,6);
                    if(rrj_s32(distance)<rrj_s32(best)){chosen=surface_div(count-i-1,1);best=distance;}
                }
                track_write(m,selection,0,track_read(m,selection,0)|(chosen<<9));
                index=count-2*chosen;
                for(i=0;i<3;++i){edge[i]=surface_div(vertices[3*(index-2)+i]-vertices[3*(index-1)+i],2);normal[i]=material_half(m,direction+2*i)<<4;}
                dot=0;for(i=0;i<3;++i)dot+=(uint32_t)sub_8001FC90(rrj_s32(edge[i]),rrj_s32(normal[i]));
                cross=(uint32_t)sub_8001FC90(rrj_s32(edge[2]),rrj_s32(normal[0]))-(uint32_t)sub_8001FC90(rrj_s32(edge[0]),rrj_s32(normal[2]));
                v=track_read(m,selection,0);if(rrj_s32(cross)<0)v|=0x2000;
                track_write(m,selection,0,v|(pause_asr(dot,31)&0x1000));
                upper=0;
                if(track_read(m,selection,0)&0x800u){
                    header=rrj_read32(m,handle);base=rrj_read32(m,header+60);
                    if(base){
                        offset=rrj_u16(rrj_at(m,header+4,2))+rrj_u16(rrj_at(m,header+6,2));
                        entry=rrj_read32(m,header+40)+12u*(offset+group);
                        track_write(m,geometry,0,base+rrj_read32(m,entry));upper=1;
                    }
                }
                if(!upper){
                    if(!rrj_read32(m,rrj_read32(m,handle)+56))goto missing;
                    track_write(m,selection,0,track_read(m,selection,0)&0xfffff7ffu);
                    header=rrj_read32(m,handle);offset=rrj_u16(rrj_at(m,header+4,2));
                    entry=rrj_read32(m,header+40)+12u*(offset+group);
                    v=rrj_read32(m,entry);base=rrj_read32(m,header+56);
                    track_write(m,geometry,0,base+v);
                }
                header=rrj_read32(m,handle);offset=rrj_u16(rrj_at(m,header+4,2));
                if(upper)offset+=rrj_u16(rrj_at(m,header+6,2));
                entry=rrj_read32(m,header+40)+12u*(offset+group);
                return rrj_read32(m,entry+4)|(rrj_read32(m,entry+8)<<16);
            }
            header=rrj_read32(m,handle);
        }
    }
 missing:track_write(m,selection,0,0);return 0;
}
uint32_t sub_800A8498(RRJMemory *m,uint32_t position,uint32_t direction,uint32_t geometry,uint32_t vertices,uint32_t origin,uint32_t selection)
{return surface_geometry(m,position,direction,track_ram(geometry),track_ram(vertices),track_ram(origin),track_ram(selection));}
uint32_t rrj_surface_geometry_local(RRJMemory *m,uint32_t position,uint32_t direction,uint32_t *geometry,uint32_t *vertices,uint32_t origin[3],uint32_t *selection)
{return surface_geometry(m,position,direction,track_local(geometry),track_local(vertices),track_local(origin),track_local(selection));}

static uint32_t surface_range_byte(const uint8_t ranges[6],uint32_t index)
{if(index>=6)abort();return ranges[index];}
static uint32_t surface_hit(RRJMemory *m,uint32_t actor,uint32_t position,RRJTrackWords output,uint32_t normal_address,uint16_t *normal_local,uint32_t prior)
{
    uint32_t geometry=0,vertex_base=0,origin[3],selection=prior&0x7ffu,distance=rrj_read32(m,actor+44),reply;
    uint32_t point[3],vertices[12],table,offset,same,i,j,n,value,category,index,last,finish,step,group_step,limit,iterations=0,status;
    uint32_t bounds_min[2],bounds_max[2],quad,face,face_index,small,point_index,normal_start=0;
    uint8_t ranges[6];uint16_t normal[3];
    if(!position)position=actor+184;
    if(rrj_read32(m,rrj_read32(m,0x8005B2F8)+48)>=2 && rrj_s32(rrj_read32(m,actor+48))<rrj_s32(distance)){
        distance=rrj_read32(m,actor+48);selection|=0x80000000u;
    }
    if(rrj_s32(distance)<1280)selection|=0x800;
    reply=rrj_surface_geometry_local(m,position,actor+450,&geometry,&vertex_base,origin,&selection);
    selection&=0x7fffffffu;if(!reply)return 0xffffffffu;
    for(i=0;i<3;++i)point[i]=rrj_read32(m,position+4*i)-origin[i];
    same=((selection^prior)&0xfffu)==0;
    offset=same?((prior>>14)&1023u):0xffffffffu;
    table=rrj_read32(m,rrj_read32(m,0x800D87EC+112u*(selection&63u))+48);
    if(offset>=512){
        n=material_byte(m,table+((selection>>11)&1u));offset=2;
        for(i=0;i<n;++i){
            value=material_byte(m,table+offset);
            if((value>>7)==((selection>>11)&1u) && ((value>>4)&7u)==((selection>>6)&7u) && ((value>>2)&3u)==((selection>>9)&3u))break;
            offset+=2+material_byte(m,table+offset+1);
        }
        if(i==n)return 0xffffffffu;
    }
    table+=offset;n=material_byte(m,table+1);
    if(!n)return (offset<<14)|(selection&0xfffu)|0xff000000u;
    index=(prior>>24)&127u;category=(prior>>12)&3u;
    ranges[0]=2;ranges[1]=(uint8_t)(n+1);ranges[2]=(uint8_t)(n+4);
    value=(uint32_t)ranges[2]+material_byte(m,table+ranges[2]-1);
    ranges[3]=(uint8_t)(value-1);ranges[4]=(uint8_t)(value+2);
    ranges[5]=(uint8_t)(ranges[4]+(material_byte(m,table+ranges[4]-1)&(0u-((selection>>11)&1u)))-1);
    step=1-((selection>>11)&2u);group_step=1-((selection>>12)&2u);
    if(category==3 || ranges[2*category+1]<ranges[2*category])category=0;
    if(!same || index==127 || index<2)index=surface_range_byte(ranges,2*category+(step>>31));
    last=surface_range_byte(ranges,2*category+(rrj_s32(step)<0?step+1:step));
    finish=group_step==1?((selection>>11)&1u)+1:0;
    while(finish && ranges[2*finish+1]<ranges[2*finish])--finish;
    limit=(uint32_t)ranges[1]-ranges[0]+ranges[3]-ranges[2]+((0u-((selection>>11)&1u))&((uint32_t)ranges[5]-ranges[4]+1))+2;
    do{
        bounds_min[0]=bounds_min[1]=0x3fff0000;bounds_max[0]=bounds_max[1]=0xc0010000;
        face_index=material_byte(m,table+index);small=reply&65535u;quad=face_index>=small;
        face_index-=quad?small:0;
        face=geometry+14+20u*(quad?small:face_index)+(quad?2u*(12u*face_index+1):0);
        n=3+quad;
        for(i=0;i<n;++i){
            for(j=0;j<3;++j)vertices[3*i+j]=material_half(m,vertex_base+8u*rrj_u16(rrj_at(m,face+2*i,2))+2*j)<<10;
            for(j=0;j<2;++j){value=vertices[3*i+2*j];if(rrj_s32(value)<rrj_s32(bounds_min[j]))bounds_min[j]=value;if(rrj_s32(value)>rrj_s32(bounds_max[j]))bounds_max[j]=value;}
        }
        if(rrj_s32(point[0])>=rrj_s32(bounds_min[0]) && rrj_s32(point[0])<=rrj_s32(bounds_max[0]) &&
           rrj_s32(point[2])>=rrj_s32(bounds_min[1]) && rrj_s32(point[2])<=rrj_s32(bounds_max[1]) &&
           rrj_polygon_contains_local(m,point,vertices,n,1)){
            normal_start=0;point_index=0;
            if(quad){
                point_index=3;
                if(rrj_polygon_contains_local(m,point,vertices+3,3,1))normal_start=3;
                else for(i=0;i<3;++i)vertices[6+i]=vertices[9+i];
            }
            (void)rrj_polygon_normal_local(m,vertices+normal_start,3,normal);
            /* Original stores the normal, then negates its halfwords before point writes. */
            for(i=0;i<3;++i){if(normal_local)normal_local[i]=normal[i];else rrj_put16(rrj_at(m,normal_address+2*i,2),normal[i]);}
            {uint16_t nx=normal_local?normal_local[0]:rrj_u16(rrj_at(m,normal_address,2));
             uint16_t nz=normal_local?normal_local[2]:rrj_u16(rrj_at(m,normal_address+4,2));
             uint16_t ny;
             if(normal_local)normal_local[0]=(uint16_t)(0u-nx);else rrj_put16(rrj_at(m,normal_address,2),(uint16_t)(0u-nx));
             ny=normal_local?normal_local[1]:rrj_u16(rrj_at(m,normal_address+2,2));
             if(normal_local){normal_local[2]=(uint16_t)(0u-nz);normal_local[1]=(uint16_t)(0u-ny);}
             else{rrj_put16(rrj_at(m,normal_address+4,2),(uint16_t)(0u-nz));rrj_put16(rrj_at(m,normal_address+2,2),(uint16_t)(0u-ny));}}
            for(i=0;i<3;++i)track_write(m,output,4*i,vertices[point_index+i]+origin[i]);
            *(uint8_t *)rrj_at(m,actor+534,1)=(uint8_t)(rrj_u16(rrj_at(m,face-(quad?14:12),2))>>12);
            status=2;
        }else{
            status=0;
            if(index==last){
                if(category==finish)status=1;
                else{category+=group_step;value=2*category+(step>>31);index=surface_range_byte(ranges,value);last=surface_range_byte(ranges,value+step);}
            }else index+=step;
        }
        ++iterations;
        if(rrj_s32(limit)<rrj_s32(iterations))break;
    }while(!status);
    if(status==2)return (index<<24)|(offset<<14)|(category<<12)|(selection&0xfffu);
    return ((0u-status)&(((offset<<14)|(selection&0xfffu)|0xff000000u)+1))-1;
}
uint32_t sub_800A7BF8(RRJMemory *m,uint32_t actor,uint32_t position,uint32_t output,uint32_t normal,uint32_t prior)
{return surface_hit(m,actor,position,track_ram(output),normal,NULL,prior);}
uint32_t rrj_surface_hit_local(RRJMemory *m,uint32_t actor,uint32_t position,uint32_t output[3],uint16_t normal[3],uint32_t prior)
{return surface_hit(m,actor,position,track_local(output),0,normal,prior);}

/* 80075628: contact-triggered mode transition and direction adjustment. */
uint32_t sub_80075628(RRJMemory *m,uint32_t actor,uint32_t normal,uint32_t distance)
{
    uint32_t queried,trigger=0,speed,dot,value,modified,angles[2],owner,sign,target,offset,x,z,y,sum,numerator,ratio,scale;
    if((rrj_read32(m,actor+564)&0x61D800u) || (rrj_read32(m,actor+568)&0x7FFu))return 0;
    queried=rrj_read32(m,actor+388)&1u;
    if((rrj_s32(distance)>32768 && rrj_s32(rrj_read32(m,actor+480))>732430) || material_byte(m,actor+534)==4){trigger=1;queried=1;}
    else if(queried){
        dot=sub_8002E698(m,normal,actor+450);
        if(rrj_s32(dot)<=0){
            value=(uint32_t)sub_8001FC90(rrj_s32(dot),rrj_s32(material_half(m,normal+2)<<4));
            modified=(material_half(m,actor+452)-pause_asr(value<<12,16))<<4;
            /* Inline MIPS uses the same table/interpolation as 8001FF3C. */
            angles[0]=sub_8001FF3C(m,material_half(m,actor+452)<<4);
            angles[1]=sub_8001FF3C(m,modified);
            trigger=rrj_s32(angles[1]-angles[0])>=rrj_s32(rrj_read32(m,rrj_read32(m,actor+556)+440));
        }
    }else if(rrj_read32(m,actor+828)){
        dot=sub_8002E698(m,rrj_read32(m,actor+340)+14,actor+450);
        owner=material_half(m,rrj_read32(m,actor+828)+50);
        if(rrj_s32(owner^rrj_read32(m,actor+364))>=0){
            sign=pause_asr(owner,31);value=(owner+sign)^sign;
            if(rrj_s32(value)<rrj_s32(material_half(m,actor+482))){sign=pause_asr(dot,31);trigger=rrj_s32((dot+sign)^sign)>46333;}
        }
    }
    if(!trigger)return 0;
    rrj_write32(m,actor+568,rrj_read32(m,actor+568)|0xC00u);
    if(queried)return 1;
    speed=rrj_read32(m,actor+480);target=material_half(m,actor+452)<<4;
    if(rrj_s32(speed)<=3515664){
        offset=rrj_s32(speed)>2343776?pause_asr((uint32_t)sub_8001FC90(80629,rrj_s32(3515665-speed)),16):22;
        value=((offset+1024-sub_8001FF3C(m,target))&4095)*4;
        target=material_half(m,0x8005624C+(value|2u))<<4;
    }
    target>>=4;speed=rrj_read32(m,actor+576);
    if(rrj_s32(speed)>29032446){
        value=(0u-sub_80010028(29032446,speed))>>4;
        if(rrj_s32(value<<16)<rrj_s32(target<<16))value=target;
        target=value;
    }
    y=pause_asr(target<<16,16);
    if(y==material_half(m,actor+452))return 1;
    x=material_half(m,actor+450)<<4;z=material_half(m,actor+454)<<4;
    rrj_put16(rrj_at(m,actor+452,2),(uint16_t)target);y<<=4;
    sum=(uint32_t)sub_8001FC90(rrj_s32(x),rrj_s32(x))+(uint32_t)sub_8001FC90(rrj_s32(z),rrj_s32(z));
    if(rrj_s32(sum)<6){x=0;z=655;sum=6;}
    numerator=65536-(uint32_t)sub_8001FC90(rrj_s32(y),rrj_s32(y));
    ratio=rrj_s32(numerator)>0?sub_80010028(numerator,sum):0u-sub_80010028(0u-numerator,sum);
    scale=sub_8004CF74(m,ratio)<<2;
    rrj_put16(rrj_at(m,actor+450,2),(uint16_t)pause_asr((uint32_t)sub_8001FC90(rrj_s32(scale),rrj_s32(x)),4));
    rrj_put16(rrj_at(m,actor+454,2),(uint16_t)pause_asr((uint32_t)sub_8001FC90(rrj_s32(scale),rrj_s32(z)),4));
    return 1;
}
static void motion_outer_product(RRJMemory *m,uint32_t left,uint32_t right,uint32_t output)
{motion_outer_product_ref(m,left,NULL,right,output);}
/* 80075B08: contact deflection with signed16-truncated products. */
uint32_t sub_80075B08(RRJMemory *m,uint32_t actor,uint32_t normal)
{
    uint32_t x,z,value;
    if((rrj_read32(m,actor+568)&0x60Fu) || (rrj_read32(m,actor+564)&0x400u))return 0;
    if(rrj_s32(material_half(m,normal+2))>=2633)return 0;
    x=material_half(m,actor+450)*material_half(m,normal);
    z=material_half(m,actor+454)*material_half(m,normal+4);
    value=pause_asr(x<<4,16)+pause_asr(z<<4,16);
    if(rrj_s32(value)<=0)return 0;
    rrj_put16(rrj_at(m,actor+820,2),(uint16_t)(0u-rrj_u16(rrj_at(m,normal,2))));
    rrj_put16(rrj_at(m,actor+822,2),0);
    rrj_put16(rrj_at(m,actor+824,2),(uint16_t)(0u-rrj_u16(rrj_at(m,normal+4,2))));
    (void)sub_8002E468(m,actor+820);
    rrj_write32(m,actor+564,rrj_read32(m,actor+564)|0x400000u);
    return 1;
}
/* 8002ECB8: sum two scaled short vectors, preserving output aliases. */
int64_t sub_8002ECB8(RRJMemory *m,uint32_t left,uint32_t right,uint32_t output,uint32_t scale_left,uint32_t scale_right)
{
    uint32_t i,first;int64_t result=0;
    for(i=0;i<3;++i){
        first=(uint32_t)sub_8001FC90(rrj_s32(scale_left),rrj_s32(material_half(m,left+2*i)<<4));
        result=sub_8001FC90(rrj_s32(scale_right),rrj_s32(material_half(m,right+2*i)<<4));
        rrj_write32(m,output+4*i,first+(uint32_t)result);
    }
    return result; /* v1:v0 from the last multiply, not the output sum. */
}
/* 8007504C: project actor motion onto road or queried geometry; update child. */
uint32_t sub_8007504C(RRJMemory *m,uint32_t actor,uint32_t position)
{
    uint32_t first_point[3],second_point[3],difference[3],queried=rrj_read32(m,actor+388)&1u;
    uint32_t saved_material=0,result,reply,normal_address,flags,i,dot=0,linked,road,other_road;
    uint16_t normal_local[3],captured[3];int local_normal=0;
    RRJTrackWords plane;
    if(queried){
        plane=track_local(first_point);normal_address=actor+268;
        reply=surface_hit(m,actor,position,plane,normal_address,NULL,rrj_read32(m,actor+536));
        flags=rrj_read32(m,actor+388);
        if(reply==rrj_read32(m,actor+536))flags&=~64u;
        else{rrj_write32(m,actor+536,reply);flags|=64u;}
        rrj_write32(m,actor+388,flags);saved_material=material_byte(m,actor+534);
    }else{
        road=rrj_read32(m,actor+340);reply=0;rrj_write32(m,actor+536,0);
        plane=track_ram(road+20);normal_address=road+8;
        *(uint8_t *)rrj_at(m,actor+534,1)=(uint8_t)(rrj_read32(m,actor+372)?material_byte(m,actor+394):1);
    }
    if(rrj_s32(reply)>=0 && rrj_s32(material_half(m,normal_address+2))<-615){
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+274+2*i,2),(uint16_t)(0u-rrj_u16(rrj_at(m,normal_address+2*i,2))));
    }else{
        for(i=0;i<3;++i)captured[i]=rrj_u16(rrj_at(m,actor+522+2*i,2));
        plane=track_ram(actor+504);for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+274+2*i,2),captured[i]);
        if(!rrj_read32(m,actor+480) && material_byte(m,rrj_read32(m,actor+1084)+39)!=255){
            rrj_write32(m,actor+480,117188);(void)sub_8002EE50(m,117188,actor+450,actor+456);
        }
    }
    for(i=0;i<3;++i)difference[i]=track_read(m,plane,4*i)-rrj_read32(m,position+4*i);
    for(i=0;i<3;++i)dot+=(uint32_t)sub_8001FC90(rrj_s32(difference[i]),rrj_s32(material_half(m,actor+274+2*i)<<4));
    for(i=0;i<3;++i)rrj_write32(m,actor+504+4*i,(uint32_t)sub_8001FC90(rrj_s32(material_half(m,actor+274+2*i)<<4),rrj_s32(dot))+rrj_read32(m,position+4*i));
    rrj_write32(m,actor+260,dot);
    if(queried){
        reply=surface_hit(m,actor,actor+244,track_local(second_point),actor+268,NULL,rrj_read32(m,actor+572));
        flags=rrj_read32(m,actor+564)&0xfdffffffu;rrj_write32(m,actor+564,flags);
        if(rrj_s32(reply)<0){
            for(i=0;i<3;++i)captured[i]=rrj_u16(rrj_at(m,actor+274+2*i,2));
            for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+268+2*i,2),captured[i]);
            for(i=0;i<3;++i)rrj_write32(m,actor+280+4*i,track_read(m,plane,4*i));
        }else{
            if(reply!=rrj_read32(m,actor+572))rrj_write32(m,actor+564,flags|0x02000000u);
            captured[0]=rrj_u16(rrj_at(m,actor+268,2));captured[2]=rrj_u16(rrj_at(m,actor+272,2));
            rrj_put16(rrj_at(m,actor+268,2),(uint16_t)(0u-captured[0]));captured[1]=rrj_u16(rrj_at(m,actor+270,2));
            rrj_put16(rrj_at(m,actor+272,2),(uint16_t)(0u-captured[2]));rrj_put16(rrj_at(m,actor+270,2),(uint16_t)(0u-captured[1]));
            for(i=0;i<3;++i)rrj_write32(m,actor+280+4*i,second_point[i]);
        }
        result=material_byte(m,actor+534);rrj_write32(m,actor+572,reply);
        *(uint8_t *)rrj_at(m,actor+534,1)=(uint8_t)saved_material;
    }else{
        road=rrj_read32(m,actor+256);rrj_write32(m,actor+572,0);
        for(i=0;i<2;++i)rrj_put16(rrj_at(m,actor+268+2*i,2),(uint16_t)(0u-rrj_u16(rrj_at(m,road+8+2*i,2))));
        captured[2]=rrj_u16(rrj_at(m,road+12,2));other_road=rrj_read32(m,actor+340);
        rrj_put16(rrj_at(m,actor+272,2),(uint16_t)(0u-captured[2]));
        for(i=0;i<3;++i)rrj_write32(m,actor+280+4*i,rrj_read32(m,road+20+4*i));
        flags=rrj_read32(m,actor+564);flags=other_road==road?flags&0xfdffffffu:flags|0x02000000u;
        rrj_write32(m,actor+564,flags);result=rrj_read32(m,actor+372)?material_byte(m,actor+394):1;
    }
    *(uint8_t *)rrj_at(m,actor+535,1)=(uint8_t)result;
    linked=rrj_read32(m,actor+856);if(!linked)return result;
    result=rrj_read32(m,actor+1088);if(!result)return result;
    rrj_write32(m,linked+44,rrj_read32(m,actor+44));flags=rrj_read32(m,linked+388)&1u;
    rrj_write32(m,linked+48,rrj_read32(m,actor+48));
    if(flags){
        plane=track_local(first_point);local_normal=1;
        reply=surface_hit(m,linked,0,plane,0,normal_local,rrj_read32(m,linked+536));rrj_write32(m,linked+536,reply);
    }else{
        reply=0;*(uint8_t *)rrj_at(m,linked+534,1)=(uint8_t)(rrj_read32(m,linked+372)?material_byte(m,linked+394):1);
        rrj_write32(m,linked+536,0);
    }
    if(rrj_s32(reply)<=0){
        road=rrj_read32(m,linked+340);plane=track_ram(road+20);normal_address=road+8;local_normal=0;
        if(rrj_s32(reply)<0){
            if(rrj_read32(m,linked+480))return 65536;
            rrj_write32(m,linked+480,117188);return (uint32_t)sub_8002EE50(m,117188,linked+450,linked+456);
        }
    }
    result=local_normal?(uint32_t)(int32_t)(int16_t)normal_local[1]:material_half(m,normal_address+2);
    result=rrj_s32(result)<-614;if(!result)return result;
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,linked+814+2*i,2),(uint16_t)(0u-(local_normal?normal_local[i]:rrj_u16(rrj_at(m,normal_address+2*i,2)))));
    for(i=0;i<3;++i){result=track_read(m,plane,4*i);rrj_write32(m,linked+784+4*i,result);}
    return result;
}
static uint32_t contact_signed_ratio(uint32_t numerator,uint32_t denominator)
{
    int np=rrj_s32(numerator)>0,dp=rrj_s32(denominator)>0;
    uint32_t value=sub_80010028(np?numerator:0u-numerator,dp?denominator:0u-denominator);
    return np==dp?value:0u-value;
}
static void wheel_rate(RRJMemory *m,uint32_t actor,uint32_t step)
{
    uint32_t value=material_half(m,actor+840);
    if(rrj_s32(value)>=23)rrj_put16(rrj_at(m,actor+840,2),(uint16_t)(value-34));
    else if(!value)rrj_put16(rrj_at(m,actor+840,2),(uint16_t)step);
}
static void wheel_wrap(RRJMemory *m,uint32_t address)
{
    while(rrj_s32(material_half(m,address))< -4096)
        rrj_put16(rrj_at(m,address,2),(uint16_t)(rrj_u16(rrj_at(m,address,2))+4096));
}
/* 800807F0: wheel phase, bounds, linked placement and heading. */
uint32_t sub_800807F0(RRJMemory *m,uint32_t actor,uint32_t delta)
{
    uint32_t linked=rrj_read32(m,actor+856)!=0,step,x,z,other,value,i;
    if((int8_t)material_byte(m,actor+8)<2){
        step=pause_asr(2608u*(uint32_t)sub_8001FC90(rrj_s32(rrj_read32(m,actor+480)),rrj_s32(delta)),16);
        if(rrj_read32(m,actor+568)&0x7FFu){
            wheel_rate(m,actor,step);step=rrj_u16(rrj_at(m,actor+840,2));
            x=rrj_u16(rrj_at(m,actor+836,2))-step;z=rrj_u16(rrj_at(m,actor+838,2))-step;
            rrj_put16(rrj_at(m,actor+836,2),(uint16_t)x);rrj_put16(rrj_at(m,actor+838,2),(uint16_t)z);
            if(linked){other=rrj_read32(m,actor+856);rrj_put16(rrj_at(m,other+836,2),(uint16_t)(rrj_u16(rrj_at(m,other+836,2))-step));}
        }else{
            x=rrj_u16(rrj_at(m,actor+836,2));z=rrj_u16(rrj_at(m,actor+838,2));
            if(rrj_read32(m,actor+564)&0x08000000u){x-=step;z-=170;}
            else{value=rrj_read32(m,actor+616);x-=rrj_s32(value)>0?22:step;z-=rrj_s32(value)<0?22:step;}
            rrj_put16(rrj_at(m,actor+836,2),(uint16_t)x);rrj_put16(rrj_at(m,actor+838,2),(uint16_t)z);
            if(linked){
                value=rrj_read32(m,actor+636);
                if(rrj_s32(value)<0){wheel_rate(m,actor,step);step=rrj_u16(rrj_at(m,actor+840,2));}
                other=rrj_read32(m,actor+856);rrj_put16(rrj_at(m,other+836,2),(uint16_t)(rrj_u16(rrj_at(m,other+836,2))-step));
                if(rrj_s32(value)>=0)rrj_put16(rrj_at(m,actor+840,2),0);
                /* Unlike the flagged path, this branch wraps the linked wheel. */
                while(rrj_s32(material_half(m,rrj_read32(m,actor+856)+836))< -4096){other=rrj_read32(m,actor+856);rrj_put16(rrj_at(m,other+836,2),(uint16_t)(rrj_u16(rrj_at(m,other+836,2))+4096));}
            }
        }
        wheel_wrap(m,actor+836);wheel_wrap(m,actor+838);
    }
    (void)sub_8008BA18(m,actor);
    if(linked){
        other=rrj_read32(m,actor+856)+432;
        /* Actual 8003FA18 with count9: sequential halfword copy, including overlap. */
        for(i=0;i<9;++i)rrj_put16(rrj_at(m,other+2*i,2),rrj_u16(rrj_at(m,actor+432+2*i,2)));
        other=rrj_read32(m,actor+856);value=rrj_read32(m,actor+304)+rrj_read32(m,other+304);
        (void)sub_8002EAD8(m,actor+184,actor+432,value,other+184);
        (void)sub_8008BA18(m,rrj_read32(m,actor+856));
    }
    value=sub_80020018(m,material_half(m,actor+528)<<4,material_half(m,actor+532)<<4);rrj_write32(m,actor+292,value);
    x=material_half(m,0x8005624C+(((value&4095)*4)|2))<<4;
    value=0x8005624C+4*(rrj_read32(m,actor+292)&4095);rrj_write32(m,actor+296,x);
    value=material_half(m,value)<<4;rrj_write32(m,actor+300,value);return value;
}
/* 8007FA4C: contact response, orientation and final position commit. */
uint32_t sub_8007FA4C(RRJMemory *m,uint32_t actor)
{
    uint32_t special=(rrj_read32(m,actor+568)&0x600u)!=0,distance=rrj_read32(m,actor+260);
    uint32_t flags=rrj_read32(m,actor+564),mode=rrj_read32(m,actor+568),queried,allowed,reply=0;
    uint32_t vector[3],cross[3],axis[3],i,j,k,dot,ratio,scale,linked,owner,x,y,z,correction;
    uint16_t average[3],captured[3];int use_actor_normal=0,orientation_done=0;
    rrj_write32(m,actor+564,flags&0xFFBFFFFFu);
    if(!(mode&0x600u)){
        flags=rrj_read32(m,actor+388);queried=flags&1u;allowed=(mode&0x80000Fu)==0;
        if((flags&64u) && allowed)reply=queried && rrj_s32(distance)<0?
            sub_80075B08(m,actor,actor+274):sub_80075628(m,actor,actor+274,rrj_read32(m,actor+260));
        distance=0;if(reply)goto finish;
        for(i=0;i<3;++i)vector[i]=rrj_read32(m,actor+280+4*i)-rrj_read32(m,actor+244+4*i);
        dot=0;for(i=0;i<3;++i)dot+=(uint32_t)sub_8001FC90(rrj_s32(vector[i]),rrj_s32(material_half(m,actor+268+2*i)<<4));
        if((rrj_read32(m,actor+564)&0x02000000u) && allowed)reply=queried && rrj_s32(dot)<0?
            sub_80075B08(m,actor,actor+268):sub_80075628(m,actor,actor+268,dot);
        if(reply)goto finish;
        for(i=0;i<3;++i)captured[i]=rrj_u16(rrj_at(m,actor+274+2*i,2));
        for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+522+2*i,2),captured[i]);
        for(i=0;i<3;++i)average[i]=(uint16_t)((rrj_s32(material_half(m,actor+268+2*i))+rrj_s32(material_half(m,actor+522+2*i)))/2);
        ratio=contact_signed_ratio(dot,short_dot_ref(m,0,average,actor+268));
        for(i=0;i<3;++i)rrj_write32(m,actor+784+4*i,(uint32_t)sub_8001FC90((int32_t)(int16_t)average[i]*16,rrj_s32(ratio))+rrj_read32(m,actor+244+4*i));
        for(i=0;i<3;++i)vector[i]=rrj_read32(m,actor+784+4*i)-rrj_read32(m,actor+504+4*i);
        (void)rrj_normalize_vector32(m,vector);
        rrj_put16(rrj_at(m,actor+528,2),(uint16_t)pause_asr(vector[0],4));scale=rrj_read32(m,actor+308);
        for(i=1;i<3;++i)rrj_put16(rrj_at(m,actor+528+2*i,2),(uint16_t)pause_asr(vector[i],4));
        for(i=0;i<3;++i)rrj_write32(m,actor+504+4*i,(uint32_t)sub_8001FC90(rrj_s32(material_half(m,actor+528+2*i)<<4),rrj_s32(scale))+rrj_read32(m,actor+504+4*i));
        linked=rrj_read32(m,actor+856);
        if(linked && rrj_read32(m,actor+1088)){
            for(i=0;i<3;++i)average[i]=(uint16_t)((rrj_s32(material_half(m,linked+814+2*i))+rrj_s32(material_half(m,actor+522+2*i)))/2);
            reply=short_dot_ref(m,0,average,linked+814);
            for(i=0;i<3;++i)vector[i]=rrj_read32(m,linked+184+4*i)-rrj_read32(m,linked+784+4*i);
            dot=0;for(i=0;i<3;++i)dot+=(uint32_t)sub_8001FC90(rrj_s32(vector[i]),rrj_s32(material_half(m,linked+814+2*i)<<4));
            ratio=0u-contact_signed_ratio(dot,reply);
            for(i=0;i<3;++i)rrj_write32(m,linked+184+4*i,(uint32_t)sub_8001FC90((int32_t)(int16_t)average[i]*16,rrj_s32(ratio))+rrj_read32(m,linked+184+4*i));
            for(i=0;i<3;++i)vector[i]=rrj_read32(m,linked+184+4*i)-rrj_read32(m,actor+504+4*i);
            for(i=0;i<3;++i)axis[i]=material_half(m,actor+528+2*i)<<4;
            for(i=0;i<3;++i){j=(i+1)%3;k=(i+2)%3;cross[i]=(uint32_t)sub_8001FC90(rrj_s32(axis[j]),rrj_s32(vector[k]))-(uint32_t)sub_8001FC90(rrj_s32(axis[k]),rrj_s32(vector[j]));}
            (void)rrj_normalize_vector32(m,cross);use_actor_normal=1;
            if(rrj_s32(cross[1])>=9841){
                for(i=0;i<3;++i)rrj_put16(rrj_at(m,actor+522+2*i,2),(uint16_t)pause_asr(cross[i],4));
                motion_outer_product(m,actor+522,actor+528,actor+516);orientation_done=1;
            }
        }
        if(!orientation_done){
            motion_outer_product_ref(m,actor+522,use_actor_normal?NULL:average,actor+528,actor+516);
            (void)sub_8002E468(m,actor+516);
            motion_outer_product(m,actor+528,actor+516,actor+522);
        }
    }
finish:
    flags=rrj_read32(m,actor+564);
    if(flags&0x04000000u){
        mode=rrj_read32(m,actor+568);
        if(mode&0x400u){
            owner=rrj_read32(m,actor+828);
            if(owner){mode=rrj_s32(material_half(m,owner+50)^rrj_read32(m,actor+364))<0?mode&0xDFFFFFFFu:mode|0x20000000u;rrj_write32(m,actor+568,mode);}
            rrj_write32(m,actor+708,(rrj_read32(m,actor+568)&0x20000000u)?6553:0);
        }
        flags=rrj_read32(m,actor+564);rrj_write32(m,actor+828,0);rrj_write32(m,actor+564,flags&0xFBFFFFFFu);
    }
    if(special){
        reply=rrj_read32(m,actor+768);if(rrj_s32(distance)<rrj_s32(reply))distance=reply;
        flags=rrj_read32(m,actor+564)&0xFF7FFFFFu;rrj_write32(m,actor+768,distance);rrj_write32(m,actor+564,flags);
        if(rrj_s32(sub_8002E698(m,actor+522,actor+450))<0)flags|=0x800000u;
        rrj_write32(m,actor+564,flags);return flags;
    }
    flags=rrj_read32(m,actor+564);
    if(flags&0x400000u){
        x=rrj_read32(m,actor+468);y=rrj_read32(m,actor+472);z=rrj_read32(m,actor+476);
        correction=((rrj_read32(m,actor+560)>>27)^1)&1;
        reply=x-rrj_read32(m,actor+796);rrj_write32(m,actor+184,x);rrj_write32(m,actor+188,y);rrj_write32(m,actor+504,reply);rrj_write32(m,actor+192,z);
        y-=rrj_read32(m,actor+800);z-=rrj_read32(m,actor+804);rrj_write32(m,actor+508,y);rrj_write32(m,actor+512,z);
        return sub_800374D4(m,actor,correction);
    }
    if(flags&0x40000u){
        owner=rrj_read32(m,actor+832);(void)sub_8002ECB8(m,owner+260,owner+272,actor+504,rrj_read32(m,actor+776),rrj_read32(m,actor+780));
        for(i=0;i<3;++i)rrj_write32(m,actor+504+4*i,rrj_read32(m,actor+504+4*i)+rrj_read32(m,rrj_read32(m,actor+832)+12+4*i));
    }
    if(rrj_read32(m,actor+568)&0x600u){
        (void)sub_80071D24(m,actor);rrj_write32(m,actor+568,rrj_read32(m,actor+568)|0x08000000u);
        reply=rrj_read32(m,actor+564)|0x800000u;rrj_write32(m,actor+564,reply);return reply;
    }
    x=rrj_read32(m,actor+504)+rrj_read32(m,actor+796);y=rrj_read32(m,actor+800);z=rrj_read32(m,actor+804);
    rrj_write32(m,actor+184,x);reply=rrj_read32(m,actor+508)+y;z+=rrj_read32(m,actor+512);
    rrj_write32(m,actor+188,reply);rrj_write32(m,actor+192,z);return reply;
}

/* 8008A998: player state flags and actor target selection, exact residual v0. */
uint32_t sub_8008A998(RRJMemory *m,uint32_t player,uint32_t requested)
{
    uint32_t same=requested==rrj_read32(m,player+544),value,flags,target;
    if(requested<4 && rrj_read32(m,player+540)!=requested){
        value=rrj_read32(m,player+548);
        rrj_write32(m,player+544,requested);
        target=rrj_read32(m,player+568);
        rrj_write32(m,player+548,(value&0xffffffe7u)|(same?2u:6u));
        if(rrj_u16(rrj_at(m,target+172,2))>>5)
            rrj_write32(m,player+568,rrj_read32(m,0x8005B268+4*(rrj_u16(rrj_at(m,player+172,2))==158)));
    }
    flags=rrj_read32(m,player+552);
    rrj_write32(m,player+772,requested>=7);
    if(flags&1)rrj_write32(m,player+548,rrj_read32(m,player+548)|256u);
    flags=rrj_read32(m,player+552)&0xfffffffau;
    value=rrj_read32(m,player+540);
    rrj_write32(m,player+552,flags);
    value=value==12?flags|128u:flags&0xfffffc7fu;
    rrj_write32(m,player+552,value);rrj_write32(m,player+540,requested);
    if(same){
        value=rrj_read32(m,player+552)&64u;
        if(value){value=rrj_read32(m,player+552);rrj_write32(m,player+772,1);value|=16u;rrj_write32(m,player+552,value);}
    }
    return value;
}
/* 800BC7CC: reset event sequence, preserving prior last event 0 or 18. */
uint32_t sub_800BC7CC(RRJMemory *m,uint32_t actor)
{
    uint32_t count=(material_byte(m,actor+946)^128u)-128u;
    uint32_t flags=rrj_read32(m,actor+560),body=rrj_read32(m,actor+852);
    uint32_t last=rrj_u16(rrj_at(m,actor+956+8*(count-1u),2)),type,value;
    uint8_t event[8]={0};
    if(flags&0x08000000u){
        type=rrj_u16(rrj_at(m,body+544,2));
        if(rrj_u16(rrj_at(m,0x800541D4+8*type+2,2))==3){
            type=rrj_u16(rrj_at(m,rrj_read32(m,0x8005AD4C)+12*material_byte(m,body+569),2));
            (void)sub_800C4550(m,type,body,2);
        }
    }
    (void)sub_800BCD10(m,actor);
    rrj_put16(event,2);rrj_put16(event+2,224);
    (void)rrj_enqueue_actor_event_local(m,event,1,actor);
    value=18;
    if(last==0 || last==18){
        rrj_put16(event,(uint16_t)last);
        rrj_put16(event+2,last==18?rrj_u16(rrj_at(m,actor+172,2)):224);
        value=rrj_enqueue_actor_event_local(m,event,0,actor);
    }
    rrj_put16(rrj_at(m,actor+944,2),0);rrj_write32(m,actor+908,0);
    return value;
}
/* 80092C7C: mark actor stopped and commit the player reason/state. */
uint32_t sub_80092C7C(RRJMemory *m,uint32_t actor,uint32_t reason)
{
    uint32_t flags=rrj_read32(m,actor+560),descriptor=rrj_read32(m,actor+1084),value,player;
    rrj_write32(m,actor+560,flags|0x08000000u);
    value=rrj_read32(m,descriptor+40);rrj_write32(m,0x8005B230,0);
    if(!value){
        *(uint8_t *)rrj_at(m,descriptor+39,1)=(uint8_t)(reason==9?254:255);
        value=rrj_read32(m,rrj_read32(m,0x8005B2F8)+16);rrj_write32(m,descriptor+40,value);
        (void)sub_800BC7CC(m,actor);
        if(material_byte(m,descriptor+39)==255 && rrj_read32(m,rrj_read32(m,actor+852)+604)<2){
            (void)sub_8002090C(m,actor);
            rrj_write32(m,actor+568,rrj_read32(m,actor+568)|0x08000000u);
        }
    }
    player=0x800CD898+1132u*rrj_u16(rrj_at(m,actor+172,2));
    if(reason==10){
        rrj_write32(m,player+772,1);
        player=0x800CD898+1132u*rrj_u16(rrj_at(m,actor+172,2));
        rrj_write32(m,player+552,rrj_read32(m,player+552)|20u);return player;
    }
    value=rrj_read32(m,player+552)&4u;
    return value?value:sub_8008A998(m,player,reason);
}

/* 8002F0F4: truncate each signed square independently before wrapping sum. */
uint32_t sub_8002F0F4(RRJMemory *m,uint32_t vector)
{
    uint32_t sum=0;unsigned i;
    for(i=0;i<3;++i){int64_t v=rrj_s32(rrj_read32(m,vector+4*i));sum+=(uint32_t)((uint64_t)(v*v)>>16);}
    return sum;
}
uint32_t sub_8002E548(RRJMemory *m,uint32_t vector)
{return sub_8004CF74(m,sub_8002F0F4(m,vector))<<2;}
/* 8002ED94: local first result, then in-place second, then halfword copies. */
uint32_t sub_8002ED94(RRJMemory *m,uint32_t left,uint32_t right,uint32_t angle)
{
    uint32_t table=0x8005624c+4*(angle&4095u),cosine=material_half(m,table+2)<<4,sine=material_half(m,table)<<4;
    uint16_t saved[3];unsigned i;
    (void)combine_short_vectors(m,left,right,0,saved,cosine,0u-sine);
    (void)sub_8002EB78(m,left,right,right,sine,cosine);
    for(i=0;i<3;++i)rrj_put16(rrj_at(m,left+2*i,2),saved[i]);
    return saved[2];
}
uint32_t sub_8007F08C(RRJMemory *m,uint32_t actor)
{
    uint32_t value=rrj_read32(m,actor+560),linked=rrj_read32(m,actor+856);
    value&=0xff7fffffu;rrj_write32(m,actor+560,value);
    if(linked){value=rrj_read32(m,actor+488);rrj_write32(m,linked+488,value);}
    return value;
}

/* 800BFE58: participant counter; byte wraps, residual v0 does not. */
uint32_t sub_800BFE58(RRJMemory *m,uint32_t actor,uint32_t kind,uint32_t side)
{
    uint32_t count=rrj_read32(m,rrj_read32(m,0x8005B2F8)+48);
    uint32_t index=rrj_u16(rrj_at(m,actor+172,2)),address,value;
    if(index>=count){
        value=rrj_read32(m,actor+1088);if(value)return value;
        index=rrj_u16(rrj_at(m,rrj_read32(m,actor+856)+172,2))+2u;
    }
    address=0x800D81D8+36u*index+4u*side+kind+24u;
    value=material_byte(m,address)+1u;*(uint8_t *)rrj_at(m,address,1)=(uint8_t)value;
    return value;
}
/* 800BF51C: credit each matching recent interaction, including negative age. */
uint32_t sub_800BF51C(RRJMemory *m,uint32_t actor)
{
    uint32_t slot=0x800CD548,index,age,tag;
    for(index=0;index<4;++index,slot+=12){
        if(rrj_read32(m,slot+4)!=actor)continue;
        age=rrj_read32(m,rrj_read32(m,0x8005B2F8)+16)-rrj_read32(m,slot+8);
        if(rrj_s32(age)>=240)continue;
        (void)sub_800BFE58(m,rrj_read32(m,slot),1,0);
        (void)sub_800BFE58(m,actor,1,1);
        tag=rrj_u16(rrj_at(m,rrj_read32(m,slot)+172,2));
        if(tag>1)tag=1;
        rrj_write32(m,0x800D6198+224u*tag+140,1);
    }
    return 0;
}
