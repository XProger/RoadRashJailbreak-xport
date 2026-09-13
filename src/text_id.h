#ifndef RRJ_TEXT_ID_H
#define RRJ_TEXT_ID_H
#include "psx_memory.h"
uint32_t sub_F_800662CC(RRJMemory *,uint32_t font,uint32_t id,uint32_t rect,uint32_t gap,uint32_t link,uint32_t color);
uint32_t rrj_blink_text(RRJMemory *,uint32_t font,uint32_t id,uint32_t rect,uint32_t link,uint32_t color);
#endif
