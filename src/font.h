#ifndef RRJ_FONT_H
#define RRJ_FONT_H
#include "psx_memory.h"
uint32_t sub_8002D1F8(RRJMemory *,uint32_t character,uint32_t table,uint32_t count,uint32_t stride);
uint32_t sub_8002D1A0(RRJMemory *,uint32_t character,uint32_t header,uint32_t glyphs);
uint32_t sub_8002D0D8(RRJMemory *,uint32_t font,uint32_t text);
uint32_t sub_8002CDC8(RRJMemory *,uint32_t font,uint32_t text,uint32_t x,uint32_t y,uint32_t link,uint32_t color);
#endif
