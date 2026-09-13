#ifndef RRJ_MENU_LATCH_H
#define RRJ_MENU_LATCH_H
#include "psx_memory.h"
uint32_t sub_8001E0B4(RRJMemory *,uint32_t dst,uint32_t src,uint32_t bytes);
/* WIP: only the original state==2 menu branch; other game states fail. */
void sub_8001CB3C_menu(RRJMemory *,RRJSDKCall critical);
#endif
