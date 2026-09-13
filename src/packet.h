#ifndef RRJ_PACKET_H
#define RRJ_PACKET_H
#include "psx_memory.h"
/* RRJ-specific packet queue; no equivalent NFS4 allocator is assumed. */
uint32_t sub_80021C98(RRJMemory *memory, uint32_t cursor, uint32_t bytes);
uint32_t sub_80021BE8(RRJMemory *memory);
#endif
