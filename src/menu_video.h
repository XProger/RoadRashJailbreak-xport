#ifndef RRJ_MENU_VIDEO_H
#define RRJ_MENU_VIDEO_H
#include "psx_memory.h"
typedef uint32_t (*RRJVideoCall)(RRJMemory *,uint32_t target,uint32_t descriptor);
uint32_t sub_F_8006E4D8(RRJMemory *,uint32_t menu,uint32_t entry,RRJVideoCall);
uint32_t sub_F_8006E6F4(RRJMemory *,uint32_t menu,uint32_t descriptor,RRJVideoCall);
/* Runtime game lifecycle binding; NULL retains isolated probe boundary. */
void rrj_video_bind(RRJVideoCall);
uint32_t rrj_video_dummy(RRJMemory *,uint32_t target,uint32_t descriptor);
#endif
