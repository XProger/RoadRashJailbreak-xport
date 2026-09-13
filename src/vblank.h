#ifndef RRJ_VBLANK_H
#define RRJ_VBLANK_H
#include "psx_memory.h"
/* All remaining callees here have zero or one semantic game argument. */
typedef uint32_t (*RRJVBlankCall)(RRJMemory *,uint32_t target,uint32_t arg);
uint32_t sub_8001B700(RRJMemory *,RRJVBlankCall);
uint32_t sub_F_80064C30(RRJMemory *,RRJVBlankCall);
uint32_t sub_F_800600F8(RRJMemory *);
#endif
