#ifndef RRJ_MENU_LOOP_H
#define RRJ_MENU_LOOP_H
#include "psx_memory.h"
typedef void (*RRJLoopCall)(RRJMemory *,uint32_t target,uint32_t a0,uint32_t a1);
/* One complete original iteration; host scheduling may yield only afterwards. */
void rrj_menu_iteration(RRJMemory *,RRJLoopCall);
uint32_t sub_F_80080274(RRJMemory *,RRJLoopCall);
#endif
