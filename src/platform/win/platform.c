#include "platform.h"
#include "psx.h"
#include "raster.h"
#include "gpu.h"
#include <stdio.h>
#include <string.h>

/* Platform integration test, not a replacement main menu. */
int rrj_platform_smoke(unsigned frames, int headless)
{
    PSX_CONFIG config;
    POLY_F4 quad;
    unsigned frame, colored = 0, checksum = 2166136261u;
    sint32 ticks;
    memset(&config, 0, sizeof config);
    config.window_title = "Road Rash: Jailbreak - platform integration";
    config.window_width = 800; config.window_height = 600;
    config.refresh_rate = 60; config.headless = headless;
    rrj_gpu_bind_upload(&config);
    psx_configure(&config);
    ResetGraph(0); PadInit(0);
    memset(&quad, 0, sizeof quad);
    setPolyF4(&quad);
    setRGB0(&quad, 180, 50, 20);
    setXY4(&quad, 20, 20, 180, 20, 20, 100, 180, 100);
    for (frame = 0; frame < frames && !psx_quit_requested(); ++frame) {
        rrj_raster_clear(); rrj_raster_packet(&quad);
        if (!psx_window_present(rrj_raster_pixels(), rrj_raster_width(), rrj_raster_height(), config.window_title)) return 2;
        (void)PadRead(0);
        ticks = VSync(-1);
        if (VSync(1) != 0 || VSync(-1) != ticks || VSync(0) != 1 || VSync(-1) != ticks + 1) {
            fprintf(stderr, "platform smoke: VSync tick contract failed\n");
            return 2;
        }
    }
    if (frame != frames) return 2;
    for (frame = 0; frame < (unsigned)(rrj_raster_width()*rrj_raster_height()); ++frame) {
        colored += rrj_raster_pixels()[frame] != 0;
        checksum = (checksum ^ rrj_raster_pixels()[frame]) * 16777619u;
    }
    printf("platform smoke: vblank=%d, colored_pixels=%u, headless=%d, pacing=%s, framebuffer_hash=%08X\n",
        VSync(-1), colored, headless, psx_frame_pacing_enabled() ? "realtime" : "uncapped", checksum);
    return colored > 0 && (unsigned)VSync(-1) == frames ? 0 : 2;
}
