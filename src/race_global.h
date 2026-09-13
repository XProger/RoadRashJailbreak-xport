#ifndef RRJ_RACE_GLOBAL_H
#define RRJ_RACE_GLOBAL_H
#include "psx_memory.h"
typedef uint32_t (*RRJRaceGlobalCall)(RRJMemory *,uint32_t target,uint32_t argument);
uint32_t sub_G_8008AB00(RRJMemory *,uint32_t delta,RRJRaceGlobalCall);
uint32_t sub_G_8008AD38(RRJMemory *,uint32_t delta,RRJRaceGlobalCall);
uint32_t sub_G_8008AD40(RRJMemory *,uint32_t delta,uint32_t player,uint32_t base,RRJRaceGlobalCall);
#endif
