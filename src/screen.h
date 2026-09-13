#ifndef RRJ_SCREEN_H
#define RRJ_SCREEN_H
#include "psx_memory.h"
/* ClearImage has fixed black RGB; other boundaries take zero or one argument. */
typedef uint32_t (*RRJScreenCall)(RRJMemory *,uint32_t target,uint32_t arg);
uint32_t sub_8001BF1C(RRJMemory *,uint32_t,uint32_t,uint32_t,uint32_t,RRJScreenCall);
uint32_t sub_F_80080D08(RRJMemory *,RRJScreenCall);
typedef void (*RRJScreenEnv)(RRJMemory *,uint32_t target,uint32_t address,uint32_t x,uint32_t y,uint32_t width,uint32_t height);
uint32_t sub_8001BE08(RRJMemory *,uint32_t x,uint32_t y,uint32_t width,uint32_t height,RRJScreenCall,RRJScreenEnv);
#endif
