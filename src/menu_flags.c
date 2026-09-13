/* SLUS8002D250: clear bits0..64, preserve upper seven bits of ninth byte. */
#include "menu_flags.h"
uint32_t sub_8002D250(RRJMemory *m)
{
    uint32_t i;for(i=0;i<65;++i){uint8_t *p=rrj_at(m,0x800D81C8+(i>>3),1);*p=(uint8_t)(*p&~(1u<<(i&7)));}return 0;
}
