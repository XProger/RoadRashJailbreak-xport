#ifndef RRJ_MENU_H
#define RRJ_MENU_H
#include "psx_memory.h"
uint32_t sub_8001C428(RRJMemory *memory);
uint32_t sub_8001C3F4(RRJMemory *memory);
void sub_F_800803FC(RRJMemory *memory);
uint32_t sub_F_80064B30(RRJMemory *memory, uint32_t menu, uint32_t entry);
void sub_F_800685BC(RRJMemory *memory, uint32_t entry);
#endif
