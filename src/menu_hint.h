#ifndef RRJ_MENU_HINT_H
#define RRJ_MENU_HINT_H
#include "menu_sprite.h"
/* Required resolver for the resident mode-3 call into overlay address 662CC.
 * The original passes (font,id,rectangle,0,link,color). */
typedef uint32_t (*RRJBlinkText)(RRJMemory *,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t sub_8002CB08(RRJMemory *,uint32_t font,uint32_t id,uint32_t rect,uint32_t link,uint32_t color,uint32_t mode,RRJBlinkText blink);
uint32_t sub_F_8006E7A4(RRJMemory *,uint32_t menu,uint32_t entry,RRJMenuResource resource,RRJBlinkText blink);
#endif
