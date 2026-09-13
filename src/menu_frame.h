#ifndef RRJ_MENU_FRAME_H
#define RRJ_MENU_FRAME_H
#include "psx_memory.h"
/* Resolve a required game callee. No implicit no-op for missing callbacks. */
typedef uint32_t (*RRJMenuFrameCall)(RRJMemory *, uint32_t target, uint32_t a0, uint32_t a1);
/* The resident caller discards the original scratch v0. */
void sub_F_80066C34(RRJMemory *, RRJMenuFrameCall);
#endif
