#ifndef RRJ_VIDEO_STOP_H
#define RRJ_VIDEO_STOP_H
#include "psx_memory.h"
/* Shutdown decoder and close CD file are explicit host boundaries. */
uint32_t sub_F_8006FE6C(RRJMemory *,RRJSDKCall);
uint32_t sub_F_8006FED4(RRJMemory *,RRJSDKCall);
#endif
