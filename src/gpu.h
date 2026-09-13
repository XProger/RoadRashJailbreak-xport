#ifndef RRJ_GPU_H
#define RRJ_GPU_H
#include "psx_memory.h"
#include "psx.h"
/* Native host adapter for game-generated PSX packet data. */
int rrj_gpu_draw_ot(RRJMemory *memory, uint32_t address, int origin_x, int origin_y);
void rrj_gpu_clear_ot(RRJMemory *memory, uint32_t address, uint32_t count);
/* Bind the implemented VRAM upload backend to the reused PsyQ wrapper. */
void rrj_gpu_bind_upload(PSX_CONFIG *config);
uint32_t rrj_gpu_upload(RRJMemory *memory, const uint8_t rect[8], uint32_t pixels);
#endif
