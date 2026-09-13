#include "menu_navigation.h"
#include <stdlib.h>

static int32_t signed_half(RRJMemory *memory, uint32_t address)
{
    uint32_t bits = rrj_u16(rrj_at(memory, address, 2));
    return bits < 0x8000 ? (int32_t)bits : (int32_t)bits - 65536;
}

/* Shared predicate follows the same ordered MIPS block in both functions.
 * IDA's unsigned byte for the player index is wrong: MIPS uses LB. The
 * resource mode is loaded once by the caller, before scanning entries. */
static int selectable(RRJMemory *memory, uint32_t entry, uint32_t mode)
{
    uint32_t type = rrj_u16(rrj_at(memory, entry + 8, 2));
    uint32_t flags;
    if (type != 12 && type != 13 && type != 17) return 0;
    flags = rrj_read32(memory, entry);
    if ((flags & 0x40000000) && (*(uint8_t *)rrj_at(memory, 0x800D70E1, 1) >> 4) != 8) return 0;
    if (flags & 0x02000000) {
        uint32_t bits = *(uint8_t *)rrj_at(memory, 0x8009C5E6, 1);
        int32_t player = bits < 128 ? (int32_t)bits : (int32_t)bits - 256;
        if (rrj_read32(memory, 0x800D742C + (uint32_t)player * 24) != 1) return 0;
    }
    if ((flags & 0x10000000) && !*(uint8_t *)rrj_at(memory, 0x800D80F3, 1)) return 0;
    if ((flags & 0x08000000) && mode != 4) return 0;
    if ((flags & 0x04000000) && mode == 4) return 0;
    return 1;
}

/* F:8006C3A4, 436 bytes. The 0x7fff sentinel resets selection before
 * comparison, so it reports the unchanged-selection sound for index zero.
 * The selected halfword is stored AFTER the sound call, as in the original. */
uint32_t sub_F_8006C3A4(RRJMemory *memory, uint32_t menu, RRJMenuSound sound)
{
    int32_t old = signed_half(memory, menu + 4), next = old + 1;
    uint32_t result, event = 4;
    if (!sound) abort();
    if (old == 0x7fff) {
        rrj_put16(rrj_at(memory, menu + 4, 2), 0); next = 0;
    } else if (next >= signed_half(memory, menu + 8)) next = 0;
    old = signed_half(memory, menu + 4);
    if (next != old) {
        uint32_t mode = rrj_read32(memory, 0x800D80D8);
        uint32_t offset = (uint32_t)next * 120;
        do {
            if (selectable(memory, rrj_read32(memory, menu + 16) + offset, mode)) break;
            ++next; offset += 120;
            if (next >= signed_half(memory, menu + 8)) { next = 0; offset = 0; }
        } while (next != old);
        event = next == signed_half(memory, menu + 4) ? 4 : 0;
    }
    result = sound(memory, event);
    rrj_put16(rrj_at(memory, menu + 4, 2), (uint32_t)next);
    return result;
}

/* F:8006C558, 424 bytes. Unlike the forward scan, the sentinel resets to
 * zero and then decrements/wraps, selecting the last eligible entry. */
uint32_t sub_F_8006C558(RRJMemory *memory, uint32_t menu, RRJMenuSound sound)
{
    int32_t old = signed_half(memory, menu + 4), next = old - 1;
    uint32_t result, event = 4;
    if (!sound) abort();
    if (old == 0x7fff) {
        rrj_put16(rrj_at(memory, menu + 4, 2), 0);
        old = signed_half(memory, menu + 4); next = old - 1;
    }
    if (next < 0) next = signed_half(memory, menu + 8) - 1;
    if (next != old) {
        uint32_t mode = rrj_read32(memory, 0x800D80D8);
        do {
            uint32_t entry = rrj_read32(memory, menu + 16) + (uint32_t)next * 120;
            if (selectable(memory, entry, mode)) break;
            --next;
            if (next < 0) next = signed_half(memory, menu + 8) - 1;
        } while (next != old);
        event = next == signed_half(memory, menu + 4) ? 4 : 0;
    }
    result = sound(memory, event);
    rrj_put16(rrj_at(memory, menu + 4, 2), (uint32_t)next);
    return result;
}
