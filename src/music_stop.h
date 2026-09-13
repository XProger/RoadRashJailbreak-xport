#ifndef RRJ_MUSIC_STOP_H
#define RRJ_MUSIC_STOP_H
#include "audio_flush.h"
typedef uint32_t (*RRJReverbCall)(RRJMemory *,uint32_t mode,uint32_t mask);
uint32_t sub_8001FB58(RRJMemory *,uint32_t voice);
uint32_t sub_8001F7EC(RRJMemory *,uint32_t handle,RRJReverbCall);
uint32_t sub_8001F5D4(RRJMemory *,uint32_t handle,uint32_t release,RRJVoiceSetupCall,RRJReverbCall);
uint32_t sub_F_8007F158(RRJMemory *,uint32_t reset,RRJVoiceSetupCall,RRJReverbCall);
#endif
