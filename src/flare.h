#ifndef RRJ_FLARE_H
#define RRJ_FLARE_H
#include "psx_memory.h"
/* Provisional NFS4 aliases, in address order:
 * Flare_Quad, Flare_QuadRing, Flare_TextureQuad, Flare_SingleColorTex,
 * Flare_SingleColorHex, Flare_SingleColorOct, Flare_SingleColorOctRing.
 * Unlike NFS4's otz index, RRJ receives a pointer to a link/OT slot.
 * xy: 2 little-endian halfwords; color: 4 bytes; quad: 4 packed xy words.
 */
uint32_t sub_8002AF9C(RRJMemory *, const void *quad, const void *color, void *link);
uint32_t sub_8002B164(RRJMemory *, const void *quad, const void *color, void *link);
uint32_t sub_8002B258(RRJMemory *, const void *quad, const void *color, uint32_t type, void *link);
uint32_t sub_8002B3FC(RRJMemory *, const void *xy, const void *color, uint32_t width, uint32_t height, uint8_t type, void *link);
uint32_t sub_8002B4A0(RRJMemory *, const void *xy, const void *color, uint32_t width, uint32_t height, void *link);
uint32_t sub_8002B5B4(RRJMemory *, const void *xy, const void *color, uint32_t width, uint32_t height, void *link);
uint32_t sub_8002B68C(RRJMemory *, const void *xy, const void *color, uint32_t width, uint32_t height, void *link);
#endif
