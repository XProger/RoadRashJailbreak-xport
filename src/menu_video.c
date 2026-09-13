#include "menu_video.h"
#include "entry_kind.h"
#include "resource_record.h"
static uint32_t h(RRJMemory *m,uint32_t a){return rrj_u16(rrj_at(m,a,2));}
static uint32_t selected(RRJMemory *m,uint32_t menu)
{
    uint32_t index=h(m,menu+4);if(index>=32768) index|=0xffff0000;
    return sub_F_80072100(m,rrj_read32(m,menu+16)+120*index);
}
static RRJVideoCall video_backend;
void rrj_video_bind(RRJVideoCall backend){video_backend=backend;}
uint32_t rrj_video_dummy(RRJMemory *m,uint32_t target,uint32_t descriptor)
{
    /* Unbound is the historical differential-probe boundary. Live execution
     * installs actual game lifecycle with explicit CD/codec WIP effects. */
    return video_backend?video_backend(m,target,descriptor):1;
}
uint32_t sub_F_8006E6F4(RRJMemory *m,uint32_t menu,uint32_t descriptor,RRJVideoCall video)
{
    if(!descriptor) return 1;
    if(!video) video=rrj_video_dummy;
    if(!(h(m,menu)&2) || rrj_s32(rrj_read32(m,0x8005ACAC))<151)
        return video(m,0x8006FE6C,descriptor);
    if(h(m,0x8009C5DA)&1) (void)video(m,0x80070018,descriptor);
    else if(h(m,menu+6)==h(m,0x8009C5D0)) (void)video(m,0x8006FEF4,descriptor);
    return 1;
}
uint32_t sub_F_8006E4D8(RRJMemory *m,uint32_t menu,uint32_t entry,RRJVideoCall video)
{
    uint32_t local=entry+16,desc=0,kind,type=h(m,entry+8),fallback=0;
    switch(type) {
    case 2:desc=local;break;
    case 3:desc=local;rrj_write32(m,desc,selected(m,menu)==3?20:11);break;
    case 4:case 6:
        desc=sub_F_800641D4(m);
        if(desc){rrj_put16(rrj_at(m,desc+12,2),(uint16_t)h(m,local+12));rrj_put16(rrj_at(m,desc+14,2),(uint16_t)h(m,local+14));}
        else if(type==6){kind=selected(m,menu);fallback=kind!=6 && kind!=4 && kind!=5 && kind!=7;}
        break;
    case 5:fallback=1;break;
    default:break;
    }
    if(fallback) {
        desc=local;
        if(selected(m,menu)==2) rrj_write32(m,desc,19);
        else switch(rrj_read32(m,0x800D80D8)) {
        case 1:case 17:rrj_write32(m,desc,13);break;
        case 4:rrj_write32(m,desc,14);break;
        case 8:case 24:rrj_write32(m,desc,17);break;
        case 16:rrj_write32(m,desc,15);break;
        case 32:rrj_write32(m,desc,12);break;
        default:desc=0;break;
        }
    }
    return desc?sub_F_8006E6F4(m,menu,desc,video):1;
}
