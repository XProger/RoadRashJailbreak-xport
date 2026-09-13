#ifndef RRJ_MUSIC_START_H
#define RRJ_MUSIC_START_H
#include "psx_memory.h"
/* Typed flattened boundaries: voice(group, three parameter words, flags),
 * CD request(eight words), other calls use ordinary args padded with zero. */
typedef uint32_t (*RRJMusicCall)(RRJMemory *,uint32_t,uint32_t args[8]);
uint32_t sub_F_8007EDE0(RRJMemory *,uint32_t,RRJMusicCall);
uint32_t sub_F_8007EEB8(RRJMemory *,uint32_t,RRJMusicCall);
uint32_t sub_F_8007EF64(RRJMemory *,RRJMusicCall);
#endif
