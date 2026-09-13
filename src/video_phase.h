#ifndef RRJ_VIDEO_PHASE_H
#define RRJ_VIDEO_PHASE_H
#include "psx_memory.h"
/* Nine defined codec arguments. Unused arguments in other calls are zero. */
typedef uint32_t (*RRJVideoPhaseCall)(RRJMemory *,uint32_t target,const uint32_t args[9]);
/* Host formats DATA filename then opens read-only; no native/PSX stack pointer crosses boundary. */
typedef uint32_t (*RRJVideoPhaseOpen)(RRJMemory *,uint32_t format,uint32_t name);
uint32_t sub_F_8006DB5C(RRJMemory *,uint32_t menu,RRJVideoPhaseCall,RRJVideoPhaseOpen);
#endif
