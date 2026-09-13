#ifndef RRJ_MENU_ITEM_H
#define RRJ_MENU_ITEM_H
#include "menu_sprite.h"
/* Return from the resident convenience wrapper is unused by menu callers.
 * Its original no-string-table path leaves v0 undefined. */
void sub_8002CD78(RRJMemory *,uint32_t font,uint32_t string_id,uint32_t x,uint32_t y,uint32_t link,uint32_t color);
uint32_t sub_F_8006F764(RRJMemory *,uint32_t menu,uint32_t entry,RRJMenuResource resource);
#endif
