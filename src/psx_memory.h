#ifndef RRJ_MEMORY_H
#define RRJ_MEMORY_H
#include "types.h"

/* Transitional storage for original 32-bit addresses and packed data. This is
 * a byte-addressed data arena, not a CPU emulator. Native functions use ordinary
 * C arguments/locals. Keep physical/KSEG aliases in packet links unchanged. */
typedef struct RRJMemory RRJMemory;
typedef void (*RRJSDKCall)(RRJMemory *memory, uint32_t function, uint32_t a0, uint32_t a1);
struct RRJMemory {
    uint8_t *ram;
    RRJSDKCall sdk_call;
    void *sdk_user;
    /* Explicit shared scratchpad storage; never aliases main RAM. */
    uint8_t *scratchpad;
};
void *rrj_at(RRJMemory *memory, uint32_t address, size_t bytes);
uint32_t rrj_read32(RRJMemory *memory, uint32_t address);
void rrj_write32(RRJMemory *memory, uint32_t address, uint32_t value);
void rrj_sdk_call(RRJMemory *memory, uint32_t function, uint32_t a0, uint32_t a1);
#endif
