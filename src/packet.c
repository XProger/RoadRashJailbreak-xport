/* IDA draft + MIPS SLUS_010.53:80021BE8..80022018.
 * DAT names/offsets are retained. Queue field names are provisional.
 * All pointer comparisons in the allocator are unsigned, as SLTU dictates.
 */
#include "packet.h"
#define RD(a) rrj_read32(memory, (a))
#define WR(a,v) rrj_write32(memory, (a), (v))
#define CONTEXT 0x8005B470u
#define LIMIT 0x8005B4D0u
#define END 0x8005B4DCu
#define LAST 0x8005AE00u
#define CURRENT 0x8005AE04u
#define POINTERS 0x800D75B0u
#define TIMES 0x800D75C0u
#define ACTIVE 0x800D75D0u

uint32_t sub_80021BE8(RRJMemory *memory)
{
    uint32_t context, slot, start;
    rrj_sdk_call(memory, 0x80044894, 0x80010B2C, 0); /* printf(string) */
    context = RD(CONTEXT);
    rrj_sdk_call(memory, 0x80048DB4, RD(context + 0x108) + RD(0x8005ADFC) * 4 - 4, 0); /* DrawOTag */
    rrj_sdk_call(memory, 0x800487C0, 0, 0); /* DrawSync(0) */
    rrj_sdk_call(memory, 0x80044894, 0x80010B6C, 0);
    context = RD(CONTEXT);
    rrj_sdk_call(memory, 0x80048CAC, RD(context + 0x108), RD(0x8005ADFC)); /* ClearOTagR */
    rrj_sdk_call(memory, 0x800487C0, 0, 0);
    context = RD(CONTEXT);
    start = RD(context + 0xF0) & 0xFFFFFF;
    WR(LIMIT, RD(END));
    slot = POINTERS + RD(LAST) * 4;
    WR(slot, start);
    return slot; /* residual v0 at 80021C80, not a source-level semantic result */
}

/* Both original scanning blocks start from LAST+1 on each outer iteration.
 * Do not "repair" that into CURRENT+1. No artificial retry count in game code. */
static void advance_queue(RRJMemory *memory, uint32_t context, uint32_t last, uint32_t next)
{
    uint32_t index = RD(CURRENT);
    if (RD(0x8005B46C) - RD(TIMES + index * 4) >= 61)
        WR(ACTIVE + index * 64, 0);
    WR(CURRENT, next);
    if (rrj_s32(next) >= *(uint8_t *)rrj_at(memory, context + 0xF4, 1)) WR(CURRENT, 0);
    index = RD(CURRENT);
    while (RD(ACTIVE + index * 64) == 0) {
        if (index == last) break;
        ++index;
        WR(CURRENT, index);
        if (rrj_s32(index) >= *(uint8_t *)rrj_at(memory, context + 0xF4, 1)) WR(CURRENT, 0);
        index = RD(CURRENT);
    }
}

uint32_t sub_80021C98(RRJMemory *memory, uint32_t cursor, uint32_t bytes)
{
    uint32_t context, last, next, end, candidate, request_end;
    int fits = 0, fits_end;
    end = RD(END);
    if (RD(LIMIT) == end) goto wrap_buffer;
    if (RD(CURRENT) == RD(LAST)) goto final_check;
    last = RD(LAST);
    next = last + 1;
    context = RD(CONTEXT);
    request_end = cursor + bytes;
    fits_end = request_end < end;
    for (;;) {
        advance_queue(memory, context, last, next); /* 80021E64..80021F18 */
        candidate = RD(POINTERS + RD(CURRENT) * 4);
        if (RD(LIMIT) != candidate) {
            if (cursor < candidate) WR(LIMIT, candidate);
            else {
                WR(LIMIT, end);
                if (!fits_end) goto wrap_buffer;
                fits = 1;
                goto final_check;
            }
        }
        if (request_end < RD(LIMIT)) { fits = 1; goto final_check; }
        if (RD(CURRENT) == last) break;
    }
final_check: /* 80021F80 */
    if (fits) return cursor;
    candidate = RD(POINTERS + RD(CURRENT) * 4);
    if (RD(LIMIT) != candidate) {
        if (cursor < candidate) WR(LIMIT, candidate);
        else {
            end = RD(END);
            WR(LIMIT, end);
            if (cursor + bytes >= end) goto wrap_buffer;
            fits = 1;
        }
    }
    if (cursor + bytes < RD(LIMIT)) fits = 1;
    goto finish;
wrap_buffer: /* 80021CBC; keep 24-bit physical address, not KSEG0 */
    context = RD(CONTEXT);
    cursor = RD(context + 0xF0) & 0xFFFFFF;
    request_end = cursor + bytes;
    candidate = RD(POINTERS + RD(CURRENT) * 4);
    if (request_end < candidate) { WR(LIMIT, candidate); return cursor; }
    if (RD(CURRENT) != RD(LAST)) {
        last = RD(LAST); next = last + 1;
        for (;;) {
            advance_queue(memory, context, last, next); /* 80021D34..80021DE8 */
            candidate = RD(POINTERS + RD(CURRENT) * 4);
            if (request_end < candidate) { WR(LIMIT, candidate); fits = 1; break; }
            if (RD(CURRENT) == last) break;
        }
    }
finish: /* 80021FF8 */
    if (!fits) sub_80021BE8(memory);
    return cursor;
}
