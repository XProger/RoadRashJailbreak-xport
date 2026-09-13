#ifndef RRJ_MENU_NAVIGATION_H
#define RRJ_MENU_NAVIGATION_H
#include "psx_memory.h"
/* Required binding for original F:8007EAC0. Unit probes may intercept it;
 * live integration must install its actual translation. No silent sound stub. */
typedef uint32_t (*RRJMenuSound)(RRJMemory *memory, uint32_t event);
uint32_t sub_F_8006C3A4(RRJMemory *memory, uint32_t menu, RRJMenuSound sound);
uint32_t sub_F_8006C558(RRJMemory *memory, uint32_t menu, RRJMenuSound sound);
#endif
