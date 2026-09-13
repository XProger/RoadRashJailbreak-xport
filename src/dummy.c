/* WIP: typed host-boundary placeholders requested for the menu milestone.
 * Map each original API's success/error/complete encoding in the SDK adapter;
 * these internal contracts are not claimed to be original PsyQ return codes. */
#include "dummy.h"
int32_t rrj_dummy_card_present(void) { return 0; }
int32_t rrj_dummy_card_read(void *destination, uint32_t bytes)
{ (void)destination; (void)bytes; return -1; }
int32_t rrj_dummy_card_write(const void *source, uint32_t bytes)
{ (void)source; (void)bytes; return -1; }
int32_t rrj_dummy_cd_read(uint32_t sector, uint32_t count, void *destination)
{ (void)sector; (void)count; (void)destination; return -1; }
int32_t rrj_dummy_cd_busy(void) { return 0; }
int32_t rrj_dummy_str_start(const char *path) { (void)path; return 0; }
int32_t rrj_dummy_str_busy(void) { return 0; }
int32_t rrj_dummy_mdec_decode(const void *source, void *destination)
{ (void)source; (void)destination; return -1; }
