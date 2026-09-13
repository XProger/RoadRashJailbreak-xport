#ifndef RRJ_RESOURCE_REFRESH_H
#define RRJ_RESOURCE_REFRESH_H
#include "psx_memory.h"
/* Caller ignores scratch v0; audio effects remain required callbacks. */
void sub_F_8006738C(RRJMemory *,uint32_t context,RRJSDKCall effect);
#endif
