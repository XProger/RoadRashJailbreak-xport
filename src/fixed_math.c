/* Hand translation from orig/images/SLUS_010.53/pseudocode/*.c,
 * audited against the corresponding .lst and original four-byte MIPS words.
 * NFS4 rmult has a +0x8000 rounding step which is ABSENT in this game.
 */
#include "fixed_math.h"

int64_t sub_8001FC90(int32_t a, int32_t b)
{
    const int64_t product = (int64_t)a * b;
    /* MULT, MFHI/MFLO, SRL 16, SLL 16, OR; JR delay: SRA v1,hi,16.
     * Explicit floor avoids an implementation-defined signed right shift. */
    return product / 65536 - (product < 0 && product % 65536 != 0);
}

int64_t sub_8002E874(const void *left, const void *right, void *out)
{
    const uint8_t *a = (const uint8_t *)left, *b = (const uint8_t *)right;
    uint8_t *r = (uint8_t *)out;
    uint32_t first;
    int64_t last;
    first = (uint32_t)sub_8001FC90(rrj_s32(rrj_u32(a + 4)), rrj_s32(rrj_u32(b + 8)));
    last = sub_8001FC90(rrj_s32(rrj_u32(a + 8)), rrj_s32(rrj_u32(b + 4)));
    rrj_put32(r, first - (uint32_t)last); /* 8002E8B8: before the next input loads */
    first = (uint32_t)sub_8001FC90(rrj_s32(rrj_u32(a + 8)), rrj_s32(rrj_u32(b)));
    last = sub_8001FC90(rrj_s32(rrj_u32(a)), rrj_s32(rrj_u32(b + 8)));
    rrj_put32(r + 4, first - (uint32_t)last); /* 8002E8E0 */
    first = (uint32_t)sub_8001FC90(rrj_s32(rrj_u32(a)), rrj_s32(rrj_u32(b + 4)));
    last = sub_8001FC90(rrj_s32(rrj_u32(a + 4)), rrj_s32(rrj_u32(b)));
    rrj_put32(r + 8, first - (uint32_t)last); /* SUBU wraps at 32 bits */
    return last; /* Preserve observed v0/v1, without claiming a meaningful source return. */
}

int64_t sub_8002E928(const void *vector, const void *matrix, void *out)
{
    const uint8_t *v = (const uint8_t *)vector, *m = (const uint8_t *)matrix;
    uint8_t *r = (uint8_t *)out;
    uint32_t first, second;
    int64_t last = 0;
    unsigned column;
    for (column = 0; column != 3; ++column) {
        first = (uint32_t)sub_8001FC90(rrj_s32(rrj_u32(v)), rrj_s32(rrj_u32(m + column * 4)));
        second = (uint32_t)sub_8001FC90(rrj_s32(rrj_u32(v + 4)), rrj_s32(rrj_u32(m + 12 + column * 4)));
        last = sub_8001FC90(rrj_s32(rrj_u32(v + 8)), rrj_s32(rrj_u32(m + 24 + column * 4)));
        /* Original unrolled stores: 8002E984, 8002E9C0, 8002E9FC.
         * Do not use NFS4's temporary result vector: that changes overlap. */
        rrj_put32(r + column * 4, first + second + (uint32_t)last);
    }
    return last;
}
