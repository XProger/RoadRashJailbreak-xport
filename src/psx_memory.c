#include "psx_memory.h"
#include "wip.h"
#include <stdio.h>
#include <stdlib.h>

void *rrj_at(RRJMemory *memory, uint32_t address, size_t bytes)
{
    const uint32_t physical = address & 0x1fffffff;
    if (physical >= 0x1f800000 && physical < 0x1f800400 && memory->scratchpad
        && bytes <= 0x1f800400 - physical)
        return memory->scratchpad + (physical - 0x1f800000);
    if (physical > 0x200000 || bytes > 0x200000 - physical) {
        fprintf(stderr, "Unsupported PSX data address: %08X\n", address);
        rrj_wip_stop(__func__,__FILE__,__LINE__);
    }
    return memory->ram + physical;
}
uint32_t rrj_read32(RRJMemory *memory, uint32_t address)
{
    return rrj_u32(rrj_at(memory, address, 4));
}
void rrj_write32(RRJMemory *memory, uint32_t address, uint32_t value)
{
    rrj_put32(rrj_at(memory, address, 4), value);
}
void rrj_sdk_call(RRJMemory *memory, uint32_t function, uint32_t a0, uint32_t a1)
{
    if (!memory->sdk_call) {
        fprintf(stderr, "Unbound SDK boundary %08X; no GPU backend is installed yet.\n", function);
        rrj_wip_stop(__func__,__FILE__,__LINE__);
    }
    memory->sdk_call(memory, function, a0, a1);
}
