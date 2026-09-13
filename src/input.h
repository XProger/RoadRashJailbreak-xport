#ifndef RRJ_INPUT_H
#define RRJ_INPUT_H
#include "psx_memory.h"
/* One keyboard-backed digital controller; port two disconnected. */
void rrj_input_read(RRJMemory *);
void rrj_input_latch(RRJMemory *);
#endif
