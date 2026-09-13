#ifndef RRJ_SPU_H
#define RRJ_SPU_H
#include "audio_flush.h"
void rrj_spu_initialize(const void *ram);
uint32_t rrj_spu_setup(RRJMemory *,uint32_t voice,const RRJVoiceSetup *);
void rrj_spu_command(RRJMemory *,uint32_t fn,uint32_t mode,uint32_t mask);
void rrj_spu_render(int16_t *stereo,uint32_t frames);
uint32_t rrj_spu_reverb(RRJMemory *,uint32_t mode,uint32_t mask);
#endif
