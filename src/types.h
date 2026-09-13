#ifndef RRJ_TYPES_H
#define RRJ_TYPES_H
#include <stdint.h>
#include <stddef.h>

/* Fixed PSX word widths; never use host long for an original long/pointer. */
static inline int32_t rrj_s32(uint32_t bits)
{
    return bits <= INT32_MAX ? (int32_t)bits : -1 - (int32_t)~bits;
}
static inline uint16_t rrj_u16(const void *p)
{
    const uint8_t *b = (const uint8_t *)p;
    return (uint16_t)((uint32_t)b[0] | ((uint32_t)b[1] << 8));
}
static inline uint32_t rrj_u32(const void *p)
{
    const uint8_t *b = (const uint8_t *)p;
    return b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
static inline void rrj_put16(void *p, uint32_t v)
{
    uint8_t *b = (uint8_t *)p;
    b[0] = (uint8_t)v; b[1] = (uint8_t)(v >> 8);
}
static inline void rrj_put32(void *p, uint32_t v)
{
    uint8_t *b = (uint8_t *)p;
    b[0] = (uint8_t)v; b[1] = (uint8_t)(v >> 8);
    b[2] = (uint8_t)(v >> 16); b[3] = (uint8_t)(v >> 24);
}
#endif
