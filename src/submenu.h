#ifndef RRJ_SUBMENU_H
#define RRJ_SUBMENU_H
#include "psx_memory.h"
typedef uint32_t (*RRJSubmenuCall)(RRJMemory *,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t sub_F_8006AE6C(RRJMemory *,uint32_t,RRJSubmenuCall);
#endif
