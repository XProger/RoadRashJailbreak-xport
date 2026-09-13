#ifndef RRJ_PAD_SERVICE_H
#define RRJ_PAD_SERVICE_H
#include "psx_memory.h"
typedef uint32_t (*RRJPadSDK)(RRJMemory *,uint32_t function,uint32_t a0,uint32_t a1,uint32_t a2);
uint32_t sub_8001DDC4(RRJMemory *,uint32_t player,uint32_t multitap,RRJPadSDK);
#endif
