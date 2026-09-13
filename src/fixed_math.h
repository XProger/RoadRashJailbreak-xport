#ifndef RRJ_MATH_H
#define RRJ_MATH_H
#include "types.h"

/* SLUS_010.53. NFS4 aliases are hypotheses, not original RRJ symbols.
 * Byte buffers retain little-endian layout and permitted input/output overlap.
 * No restrict: the MIPS reloads inputs after each output store.
 */
int64_t sub_8001FC90(int32_t a, int32_t b); /* fixed multiply, floor(product / 65536) */
int64_t sub_8002E874(const void *left, const void *right, void *out); /* crossproduct */
int64_t sub_8002E928(const void *vector, const void *matrix, void *out); /* transform */
#endif
