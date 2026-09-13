#ifndef RRJ_DUMMY_H
#define RRJ_DUMMY_H
#include <stdint.h>
/* User-authorized WIP boundaries. Explicit unavailable/completed results;
 * no host card/disc/video access and no fabricated resource data. */
int32_t rrj_dummy_card_present(void);
int32_t rrj_dummy_card_read(void *destination, uint32_t bytes);
int32_t rrj_dummy_card_write(const void *source, uint32_t bytes);
int32_t rrj_dummy_cd_read(uint32_t sector, uint32_t count, void *destination);
int32_t rrj_dummy_cd_busy(void);
int32_t rrj_dummy_str_start(const char *path);
int32_t rrj_dummy_str_busy(void);
int32_t rrj_dummy_mdec_decode(const void *source, void *destination);
#endif
