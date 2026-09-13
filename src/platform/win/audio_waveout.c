/* Reused AgentArmstrong WinMM backend. RRJ adapter uses synchronized SPU.
 * Start worker after buffer setup; join worker before resetting the device. */
#include <stdlib.h>
#include <string.h>
#include "../../spu.h"
#include "audio_waveout.h"
#include <windows.h>
#include <mmsystem.h>

/* Types. */
typedef struct WAVEOUT_state
{
    HWAVEOUT device;
    WAVEHDR headers[WAVEOUT_buffer_count];
    sint16 samples[WAVEOUT_buffer_count][WAVEOUT_buffer_frames * 2];
    HANDLE semaphore;
    HANDLE thread;
    volatile LONG running;
    uint8 prepared[WAVEOUT_buffer_count];
} WAVEOUT_state;

/* Variables. */
static WAVEOUT_state output;
volatile uint32 g_waveout_errors;

volatile uint32 g_waveout_submitted_buffers;

volatile uint32 g_waveout_nonzero_buffers;

volatile uint32 g_waveout_peak;

volatile uint32 g_waveout_backend_active;

/* Functions. */
__declspec(dllexport) volatile uint32 g_waveout_callback_overruns;

static void CALLBACK wave_callback(HWAVEOUT device, UINT message, DWORD_PTR instance, DWORD_PTR parameter1, DWORD_PTR parameter2)
{
    WAVEOUT_state *state = (WAVEOUT_state *)instance;
    (void)device;
    (void)parameter1;
    (void)parameter2;
    if (message == WOM_DONE && state != 0 && state->semaphore != 0 && InterlockedCompareExchange(&state->running, 1, 1) != 0)
        if (!ReleaseSemaphore(state->semaphore, 1, 0))
            InterlockedIncrement((volatile LONG *)&g_waveout_callback_overruns);
}

static DWORD WINAPI wave_thread(void *argument)
{
    WAVEOUT_state *state = (WAVEOUT_state *)argument;
    while (InterlockedCompareExchange(&state->running, 1, 1) != 0)
    {
        sint32 index;
        WaitForSingleObject(state->semaphore, INFINITE);
        if (InterlockedCompareExchange(&state->running, 1, 1) == 0)
            break;
        for (index = 0; index < WAVEOUT_buffer_count; ++index)
        {
            WAVEHDR *header = state->headers + index;
            if (!(header->dwFlags & WHDR_DONE))
                continue;
            if (state->prepared[index])
            {
                if (waveOutUnprepareHeader(state->device, header, sizeof(*header)) != MMSYSERR_NOERROR)
                { ++g_waveout_errors; continue; }
                state->prepared[index] = 0;
            }
            rrj_spu_render(state->samples[index], WAVEOUT_buffer_frames);
            {
                sint32 sample_index;
                uint32 peak = 0;
                for (sample_index = 0; sample_index < WAVEOUT_buffer_frames * 2; ++sample_index)
                {
                    sint32 value = state->samples[index][sample_index];
                    uint32 magnitude = (uint32)(value < 0 ? -value : value);
                    if (magnitude > peak)
                        peak = magnitude;
                }
                if (peak != 0)
                    ++g_waveout_nonzero_buffers;
                if (peak > g_waveout_peak)
                    g_waveout_peak = peak;
            }
            header->dwFlags = 0;
            header->dwLoops = 0;
            if (waveOutPrepareHeader(state->device, header, sizeof(*header)) != MMSYSERR_NOERROR)
            { ++g_waveout_errors; continue; }
            state->prepared[index] = 1;
            if (waveOutWrite(state->device, header, sizeof(*header)) != MMSYSERR_NOERROR)
            {
                ++g_waveout_errors;
                if (waveOutUnprepareHeader(state->device, header, sizeof(*header)) != MMSYSERR_NOERROR) ++g_waveout_errors;
                state->prepared[index] = 0;
            }
            else
                ++g_waveout_submitted_buffers;
        }
    }
    return 0;
}

void waveout_shutdown(void)
{
    sint32 index;
    HANDLE thread;
    if (output.device == 0 && output.semaphore == 0)
        return;
    InterlockedExchange(&output.running, 0);
    g_waveout_backend_active = 0;
    if (output.semaphore != 0)
        ReleaseSemaphore(output.semaphore, 1, 0);
    thread = output.thread;
    if (thread != 0)
    {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        output.thread = 0;
    }
    if (output.device != 0)
    {
        if (waveOutReset(output.device) != MMSYSERR_NOERROR) ++g_waveout_errors;
        for (index = 0; index < WAVEOUT_buffer_count; ++index)
        {
            if (!output.prepared[index])
                continue;
            if (waveOutUnprepareHeader(output.device, output.headers + index, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) ++g_waveout_errors;
            output.prepared[index] = 0;
        }
        if (waveOutClose(output.device) != MMSYSERR_NOERROR) ++g_waveout_errors;
        output.device = 0;
    }
    if (output.semaphore != 0)
    {
        CloseHandle(output.semaphore);
        output.semaphore = 0;
    }
}

sint32 waveout_init(void)
{
    WAVEFORMATEX format;
    MMRESULT result;
    sint32 index;
    if (InterlockedCompareExchange(&output.running, 1, 1) != 0)
        return 1;
    memset(&output, 0, sizeof(output));
    g_waveout_errors = 0;
    g_waveout_submitted_buffers = 0;
    g_waveout_nonzero_buffers = 0;
    g_waveout_peak = 0;
    g_waveout_callback_overruns = 0;
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = SPU_sample_rate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (WORD)(format.nChannels * sizeof(sint16));
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize = 0;
    output.semaphore = CreateSemaphore(0, 0, WAVEOUT_buffer_count, 0);
    if (output.semaphore == 0)
        return 0;
    result = waveOutOpen(&output.device, WAVE_MAPPER, &format, (DWORD_PTR)wave_callback, (DWORD_PTR)&output, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        CloseHandle(output.semaphore);
        memset(&output, 0, sizeof(output));
        return 0;
    }
    InterlockedExchange(&output.running, 1);
    g_waveout_backend_active = 1;
    for (index = 0; index < WAVEOUT_buffer_count; ++index)
    {
        WAVEHDR *header = output.headers + index;
        memset(header, 0, sizeof(*header));
        memset(output.samples[index], 0, sizeof(output.samples[index]));
        header->lpData = (LPSTR)output.samples[index];
        header->dwBufferLength = sizeof(output.samples[index]);
        if (waveOutPrepareHeader(output.device, header, sizeof(*header)) != MMSYSERR_NOERROR)
            break;
        output.prepared[index] = 1;
        if (waveOutWrite(output.device, header, sizeof(*header)) != MMSYSERR_NOERROR)
            break;
    }
    if (index != WAVEOUT_buffer_count)
    {
        waveout_shutdown();
        return 0;
    }
    output.thread = CreateThread(0, 0, wave_thread, &output, 0, 0);
    if (output.thread == 0)
    {
        waveout_shutdown();
        return 0;
    }
    return 1;
}

sint32 waveout_is_running(void)
{
    return InterlockedCompareExchange(&output.running, 1, 1) != 0;
}
