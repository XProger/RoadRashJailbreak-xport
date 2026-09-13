#ifndef RRJ_MENU_DRAW_H
#define RRJ_MENU_DRAW_H
#include "psx_memory.h"
/* Resolve original callback address to a native renderer. Both game arguments
 * are significant: A0=menu, A1=entry. A2 contains the callback address in MIPS. */
typedef void (*RRJMenuDraw)(RRJMemory *, uint32_t function, uint32_t menu, uint32_t entry);
uint32_t sub_F_8006D3E0(RRJMemory *, uint32_t menu, RRJMenuDraw draw);
uint32_t sub_F_8006D630(RRJMemory *, uint32_t menu, RRJMenuDraw draw);
#endif
