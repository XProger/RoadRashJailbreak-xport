#ifndef RRJ_MENU_RESET_H
#define RRJ_MENU_RESET_H
#include "psx_memory.h"
typedef uint32_t (*RRJResetCall)(RRJMemory *,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t sub_F_8006883C(RRJMemory *,uint32_t,RRJResetCall);
#endif
