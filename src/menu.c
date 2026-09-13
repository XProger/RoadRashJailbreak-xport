#include "menu.h"

/* F:80064B30, IDA draft checked against all 32 MIPS words. The first
 * argument is unused. Keep the second slot-byte load after the global store:
 * a synthetic aliased record can observe that store. v0 is observable here. */
uint32_t sub_F_80064B30(RRJMemory *memory, uint32_t menu, uint32_t entry)
{
    uint32_t slot, descriptor, value_address, value;
    (void)menu;
    rrj_write32(memory, 0x8009C64C, 0);
    rrj_write32(memory, 0x8009C650, 0);
    if (!entry) return 0x800A0000;
    if (rrj_u16(rrj_at(memory, entry + 8, 2)) != 17) return 17;
    if (rrj_u16(rrj_at(memory, entry + 100, 2)) >= 6) return 0;
    slot = *(uint8_t *)rrj_at(memory, entry + 102, 1);
    descriptor = 0x8009C548 + slot * 16;
    rrj_write32(memory, 0x8009C64C, descriptor);
    slot = *(uint8_t *)rrj_at(memory, entry + 102, 1);
    value_address = 0x800D81AC + slot * 4;
    rrj_write32(memory, 0x8009C650, value_address);
    value = rrj_read32(memory, value_address);
    rrj_write32(memory, descriptor + 12, value);
    return value;
}

/* F:800685BC. Selection changes the shared resource mode. Default cases
 * preserve it. The caller ignores residual v0; null input does not define it.
 * Jump-table labels at F:8005BC64 are checked by the differential probe. */
void sub_F_800685BC(RRJMemory *memory, uint32_t entry)
{
    uint32_t type, mode;
    if (!entry) return;
    type = rrj_u16(rrj_at(memory, entry + 8, 2));
    if (type != 12 && type != 13) return;
    switch (rrj_u16(rrj_at(memory, entry + 18, 2))) {
    case 2: case 3: case 5: mode = 32; break;
    case 6: mode = 1; break;
    case 7: case 31: mode = 4; break;
    case 27: mode = 16; break;
    case 28: mode = 17; break;
    case 29: mode = 8; break;
    case 30: mode = 24; break;
    default: return;
    }
    rrj_write32(memory, 0x800D80D8, mode);
}

/* Original SLUS_010.53:8001C428. IDA draft corrected by MIPS; unsigned
 * rollover, exact ==60 test, and the reload of DAT_8005B2F8 are retained. */
uint32_t sub_8001C428(RRJMemory *memory)
{
    uint32_t state = rrj_read32(memory, 0x8005B2F8);
    uint32_t counter = rrj_read32(memory, 0x8005AD8C);
    uint32_t ticks = rrj_read32(memory, state + 12);
    uint32_t delta, mode;
    ++counter;
    rrj_write32(memory, 0x8005AD8C, counter);
    if (counter == 60) {
        rrj_write32(memory, 0x8005AD8C, 0);
        rrj_write32(memory, state + 100, 0);
    }
    state = rrj_read32(memory, 0x8005B2F8);
    mode = *(uint8_t *)rrj_at(memory, state, 1);
    delta = mode - 3 < 2 ? 0 : ticks - rrj_read32(memory, 0x8005B458);
    rrj_write32(memory, state + 32, delta);
    rrj_write32(memory, 0x8005B458, ticks);
    return delta;
}

/* Original SLUS_010.53:8001C3F4. Companion wait is interrupt driven;
 * it must be connected to the native VBlank callback, not converted to a spin. */
uint32_t sub_8001C3F4(RRJMemory *memory)
{
    uint32_t context = rrj_read32(memory, 0x8005B470);
    *(uint8_t *)rrj_at(memory, context + 4, 1) = 0;
    return context;
}

/* Original RASHCDF.BIN:800803FC. Submit old OT, alternate two pairs of
 * 72-word OTs. Native void return: the original leaves the SDK return in v0. */
void sub_F_800803FC(RRJMemory *memory)
{
    uint32_t sequence, index, offset, ot;
    rrj_sdk_call(memory, 0x80048DB4, rrj_read32(memory, 0x8009CFC8) + 284, 0);
    sequence = rrj_read32(memory, 0x800A0B00) + 1;
    index = sequence & 1;
    offset = index * 288;
    ot = 0x8009CCE8 + offset;
    rrj_write32(memory, 0x800A0B00, sequence);
    rrj_write32(memory, 0x8009CFC8, ot);
    rrj_write32(memory, 0x800A0B04, index);
    rrj_write32(memory, 0x8009CCE4, 0x8009D1B0 + offset); /* JAL delay slot */
    rrj_sdk_call(memory, 0x80048CAC, ot, 72);
    rrj_sdk_call(memory, 0x80048CAC, rrj_read32(memory, 0x8009CCE4), 72);
}
