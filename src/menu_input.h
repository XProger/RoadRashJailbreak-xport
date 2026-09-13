#ifndef RRJ_MENU_INPUT_H
#define RRJ_MENU_INPUT_H
#include "psx_memory.h"
/* WIP dependency for F:8007EDE0 music replay, required only on its branch. */
typedef void (*RRJMenuMusic)(RRJMemory *, uint32_t);
uint32_t sub_F_8006C700(RRJMemory *, uint32_t menu, uint32_t subtype);
uint32_t sub_F_8006B03C(RRJMemory *, uint32_t menu, uint32_t first, uint32_t end, RRJMenuMusic music);
uint32_t sub_F_80069418(RRJMemory *, uint32_t menu, RRJMenuMusic music);
#endif
