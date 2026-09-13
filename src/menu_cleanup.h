#ifndef RRJ_MENU_CLEANUP_H
#define RRJ_MENU_CLEANUP_H
#include "psx_memory.h"
typedef uint32_t (*RRJCleanupCall)(RRJMemory *,uint32_t target,uint32_t arg);
uint32_t sub_F_80080488(RRJMemory *,RRJCleanupCall);
#endif
