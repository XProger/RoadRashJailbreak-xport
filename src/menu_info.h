#ifndef RRJ_MENU_INFO_H
#define RRJ_MENU_INFO_H
#include "psx_memory.h"
typedef uint32_t (*RRJModeLabel)(RRJMemory *,uint32_t,uint32_t,uint32_t);
uint32_t sub_F_80063C7C(RRJMemory *,uint32_t selector,RRJModeLabel mode_label);
#endif
