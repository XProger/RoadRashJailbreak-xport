#ifndef RRJ_MENU_SPRITE_H
#define RRJ_MENU_SPRITE_H
#include "psx_memory.h"
/* Required resolver for original resource functions 8007A400/80078F44.
 * Cached descriptors need no resource call; unresolved loads abort. */
typedef uint32_t (*RRJMenuResource)(RRJMemory *, uint32_t function, uint32_t descriptor, uint32_t id);
uint32_t sub_F_800700F0(RRJMemory *, uint32_t menu, uint32_t image, uint32_t link, RRJMenuResource resource);
uint32_t sub_F_8006E894(RRJMemory *, uint32_t menu, uint32_t entry, RRJMenuResource resource);
/* Packed local C descriptor: ID, color, x/y halfwords. No PSX stack address. */
uint32_t rrj_menu_image_local(RRJMemory *,uint32_t menu,const uint8_t image[12],uint32_t link,RRJMenuResource resource);
#endif
