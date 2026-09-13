#ifndef RRJ_RACE_STEP_H
#define RRJ_RACE_STEP_H
#include "psx_memory.h"
typedef uint32_t (*RRJRaceStepCall)(RRJMemory *,uint32_t target,uint32_t a0,uint32_t a1);
/* Resident update dispatcher. G-overlay callees require native bindings. */
uint32_t sub_80012524(RRJMemory *,RRJRaceStepCall);
#endif
