#ifndef AUDIO_WAVEOUT_H
#define AUDIO_WAVEOUT_H

#include "../../audio/spu_core.h"

#define WAVEOUT_buffer_count 2
#define WAVEOUT_buffer_frames ((SPU_sample_rate / 30) * 2)

/* Read-only runtime evidence counters used by the CDB SPU integration gate. */
extern __declspec(dllexport) volatile uint32 g_waveout_submitted_buffers;
extern __declspec(dllexport) volatile uint32 g_waveout_nonzero_buffers;
extern __declspec(dllexport) volatile uint32 g_waveout_peak;
extern __declspec(dllexport) volatile uint32 g_waveout_backend_active;
extern __declspec(dllexport) volatile uint32 g_waveout_callback_overruns;

extern __declspec(dllexport) volatile uint32 g_waveout_errors;

/* BEGIN GENERATED MODULE API */
sint32 waveout_init(void);
void waveout_shutdown(void);
/* END GENERATED MODULE API */

#endif
