#include "wip.h"
/* RRJ's typed PsyQ voice/key boundary -> reused AgentArmstrong SPU core. */
#include "spu.h"
#include "audio/spu_core.h"
#include <windows.h>
#include <stdio.h>
static SRWLOCK spu_lock = SRWLOCK_INIT;
#include <stdlib.h>
/* Exact dry profile from State1; see status/menu/state1-spu-profile.json.
 * Master reverb disabled and both output gains zero. Wet profiles are WIP. */
static uint32_t reverb_mask;
uint32_t rrj_spu_reverb(RRJMemory *m,uint32_t mode,uint32_t mask)
{
    uint32_t result;
    AcquireSRWLockExclusive(&spu_lock);
    (void)m;mask&=0xffffff;
    if(mode==0)reverb_mask&=~mask;
    else if(mode==1)reverb_mask|=mask;
    else abort();
    result=reverb_mask;ReleaseSRWLockExclusive(&spu_lock);return result;
}
void rrj_spu_initialize(const void *ram)
{
    AcquireSRWLockExclusive(&spu_lock);
    reverb_mask=0;spu_core_init();
    if(!spu_core_upload(0,ram,SPU_ram_size)) abort();
    spu_core_set_master_volume(0x3fff,0x3fff);
    ReleaseSRWLockExclusive(&spu_lock);
}
uint32_t rrj_spu_setup(RRJMemory *m,uint32_t voice,const RRJVoiceSetup *a)
{
    SPU_voice_registers regs;
    AcquireSRWLockExclusive(&spu_lock);
    (void)m;
    if(voice>=SPU_voice_count || (a->mask!=0x60093 && a->mask!=0x40000)) abort();
    if(!spu_core_get_voice_registers((sint32)voice,&regs)) abort();
    if(a->mask==0x60093){
        if(a->address>=SPU_ram_size)abort();
        regs.volume_left=(sint16)a->left;regs.volume_right=(sint16)a->right;regs.pitch=a->pitch;
        regs.start_address=(uint16)(a->address>>3);regs.adsr1=a->adsr1;
    }
    regs.adsr2=a->adsr2;
    spu_core_set_voice_registers((sint32)voice,&regs);
    ReleaseSRWLockExclusive(&spu_lock);
    return 0; /* SpuNSetVoiceAttr SDK result is not consumed by flush. */
}
void rrj_spu_command(RRJMemory *m,uint32_t fn,uint32_t mode,uint32_t mask)
{
    (void)m;
    if(fn==0x80050D08){
        AcquireSRWLockExclusive(&spu_lock);
        if(mode==0) spu_core_key_off(mask);
        else if(mode==1) spu_core_key_on(mask);
        else abort();
        ReleaseSRWLockExclusive(&spu_lock);return;
    }
    if(fn==0x80050678){(void)rrj_spu_reverb(m,mode,mask);return;}
    RRJ_WIP3(m,fn,"spu_command",mode,mask,0);
}
void rrj_spu_render(int16_t *stereo,uint32_t frames){AcquireSRWLockExclusive(&spu_lock);spu_core_render(stereo,frames);ReleaseSRWLockExclusive(&spu_lock);}
