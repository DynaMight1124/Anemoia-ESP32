#include "profiler.h"

#ifdef ENABLE_PROFILING

ProfileEntry g_profile[PROF_COUNT] = {};

const char* const g_profile_names[PROF_COUNT] = {
    "bus::clock",

    "cpu::clock",
    "cpu::OAM_DMA",

    "ppu::renderBackground",
    "ppu::renderSprites",
    "ppu::finishScanline",
    "ppu::cpuRead",
    "ppu::cpuWrite",
    "ppu::ppuRead",
    "ppu::ppuWrite",
    "ppu::fakeSprite",

    "bus::cpuRead",
    "bus::cpuWrite",
    "bus::renderImage",

    "cart::cpuRead",
    "cart::cpuWrite",
    "cart::ppuRead",
    "cart::ppuWrite",
    "cart::ppuScanline",
};

void profileReport(uint32_t frame_interval)
{
    static uint32_t frame_count = 0;
    if (++frame_count < frame_interval) return;
    frame_count = 0;

    uint64_t elapsed = g_profile[PROF_BUS_CLOCK].cycles;
    if (elapsed == 0) return;
    
    LOG("==== Profiler (last interval) ====");
    for (int i = 0; i < PROF_COUNT; i++)
    {
        if (g_profile[i].hits == 0) continue;
        float pct = 100.0f * (float)g_profile[i].cycles / (float)elapsed;
        uint32_t avg = (uint32_t)(g_profile[i].cycles / g_profile[i].hits);
        LOGF("%-22s %6.2f%%  hits=%9lu  avg=%6lu cyc  total=%12llu cyc\n", g_profile_names[i], pct,
             (unsigned long)g_profile[i].hits, (unsigned long)avg,
             (unsigned long long)g_profile[i].cycles);
    }
    LOG("===================================");

    for (int i = 0; i < PROF_COUNT; i++) g_profile[i] = ProfileEntry{ 0, 0 };
}

#endif
