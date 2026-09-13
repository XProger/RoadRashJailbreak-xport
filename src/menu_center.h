#ifndef RRJ_MENU_CENTER_H
#define RRJ_MENU_CENTER_H
#include "menu_sprite.h"
typedef void (*RRJImageSelect)(RRJMemory *,uint32_t menu,uint32_t entry);
typedef uint32_t (*RRJImageSpecial)(RRJMemory *,uint32_t menu,uint32_t entry,uint32_t data);
uint32_t sub_F_8006E8FC(RRJMemory *,uint32_t menu,uint32_t entry,RRJMenuResource,RRJImageSelect,RRJImageSpecial);
#endif
