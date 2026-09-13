#ifndef RRJ_RESOURCE_H
#define RRJ_RESOURCE_H
#include "psx_memory.h"
/* LoadImage boundary: copied little-endian RECT and original data address.
 * Required for actual uploads; a missing backend is an error, not success. */
typedef uint32_t (*RRJImageUpload)(RRJMemory *, const uint8_t rect[8], uint32_t pixels);
uint32_t sub_F_80065768(RRJMemory *, uint32_t descriptor, RRJImageUpload upload);
uint32_t sub_F_8007A400(RRJMemory *, uint32_t descriptor, uint32_t id, RRJImageUpload upload);
#endif
