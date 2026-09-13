#ifndef RRJ_RACE_SERVICES_H
#define RRJ_RACE_SERVICES_H
#include "psx_memory.h"
typedef uint32_t (*RRJRaceServiceCall)(RRJMemory *,uint32_t target,uint32_t a0,uint32_t a1);
uint32_t sub_G_8008CD88(RRJMemory *,uint32_t,RRJRaceServiceCall);
uint32_t sub_G_8008AC80(RRJMemory *,uint32_t,RRJRaceServiceCall);
uint32_t sub_G_8008ACE8(RRJMemory *,uint32_t,RRJRaceServiceCall);
#endif
