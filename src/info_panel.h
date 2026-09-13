#ifndef RRJ_INFO_PANEL_H
#define RRJ_INFO_PANEL_H
#include "menu_info.h"
/* Untranslated special panels receive the original a0/a1/a2 and target. */
typedef void (*RRJSpecialPanel)(RRJMemory *,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t sub_F_8006FAC8(RRJMemory *,uint32_t menu,uint32_t entry,RRJModeLabel,RRJSpecialPanel);
#endif
