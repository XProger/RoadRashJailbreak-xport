#ifndef RRJ_ATTRACT_H
#define RRJ_ATTRACT_H
#include "psx_memory.h"
typedef uint32_t (*RRJAttractSound)(RRJMemory *,uint32_t event);
uint32_t sub_F_8006A8FC(RRJMemory *,uint32_t menu,RRJAttractSound);
#endif
