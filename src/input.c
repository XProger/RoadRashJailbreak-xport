/* Windows digital-pad boundary backed by the reused AA PsyQ PadRead.
 * Keyboard devices have no analog mode or vibration actuators. */
#include "input.h"
#include "pad_poll.h"
#include "menu_latch.h"
#include "psx.h"
#include <stdlib.h>
#include <string.h>
static uint32_t digital_sdk(RRJMemory *m,uint32_t fn,uint32_t port,uint32_t a1,uint32_t a2)
{
    (void)m;
    switch(fn){
    case 0x80040550: return port==0?6:0; /* stable/disconnected */
    case 0x8004061C:
        if(a1!=2 || a2!=0) abort();
        return 0; /* digital device has no extended mode */
    case 0x80040910:
        if(a2!=2) abort();
        return 0; /* PadSetAct is void; no keyboard actuators */
    case 0x80040890: return 0; /* no actuator alignment supported */
    default: abort();
    }
}
static void critical(RRJMemory *m,uint32_t fn,uint32_t a0,uint32_t a1)
{
    (void)m;(void)a0;(void)a1;
    /* Input and game callbacks are serialized on the native main thread. */
    if(fn!=0x80043DA4 && fn!=0x80043DB4) abort();
}
void rrj_input_read(RRJMemory *m)
{
    uint32_t buttons=PadRead(0)^0xffff;
    uint8_t *raw=(uint8_t *)rrj_at(m,0x800D70E0,72);
    memset(raw,0xff,72);
    raw[0]=0;raw[1]=0x41;raw[2]=(uint8_t)(buttons>>8);raw[3]=(uint8_t)buttons;
    memset(raw+4,0,4);
    (void)rrj_pad_poll(m,digital_sdk);
}
void rrj_input_latch(RRJMemory *m){sub_8001CB3C_menu(m,critical);}
