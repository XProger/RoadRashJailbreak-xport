#ifndef RRJ_AUDIO_FLUSH_H
#define RRJ_AUDIO_FLUSH_H
#include "psx_memory.h"
/* Only fields selected by the original 0x60093 attribute mask are defined. */
typedef struct RRJVoiceSetup { uint32_t voices,mask; uint16_t left,right,pitch; uint32_t address; uint16_t adsr1,adsr2; } RRJVoiceSetup;
typedef uint32_t (*RRJVoiceSetupCall)(RRJMemory *,uint32_t voice,const RRJVoiceSetup *);
uint32_t sub_8001EB7C(RRJMemory *,uint32_t voice,uint32_t pitch,uint32_t left,uint32_t right,RRJVoiceSetupCall);
uint32_t sub_8001EE94(RRJMemory *,RRJVoiceSetupCall,RRJSDKCall key);
#endif
