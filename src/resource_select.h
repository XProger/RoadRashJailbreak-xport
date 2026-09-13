#ifndef RRJ_RESOURCE_SELECT_H
#define RRJ_RESOURCE_SELECT_H
#include "psx_memory.h"
uint32_t sub_F_800649BC(RRJMemory *,uint32_t resource);
uint32_t sub_F_800680E8(RRJMemory *,uint32_t menu,uint32_t entry);
void rrj_select_menu_image(RRJMemory *,uint32_t menu,uint32_t entry);
#endif
