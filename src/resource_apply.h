#ifndef RRJ_RESOURCE_APPLY_H
#define RRJ_RESOURCE_APPLY_H
#include "psx_memory.h"
/* Required game audio/music effects; absent effects must not silently succeed. */
void sub_F_8006310C(RRJMemory *,uint32_t resource,RRJSDKCall effect);
#endif
