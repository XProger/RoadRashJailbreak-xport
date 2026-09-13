#ifndef RRJ_RASTER_H
#define RRJ_RASTER_H
#include <stdint.h>
void rrj_raster_clear(void);
void rrj_raster_packet(void *packet);
const uint32_t *rrj_raster_pixels(void);
uint16_t *rrj_raster_vram(void);
int rrj_raster_width(void);
int rrj_raster_height(void);
void rrj_raster_environment(uint32_t word, int origin_x, int origin_y);
#endif
