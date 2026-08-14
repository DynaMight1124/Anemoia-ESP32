#ifndef PROFILER_H
#define PROFILER_H

#include "../config.h"
#include "debug.h"
#include <Arduino.h>

#ifdef ENABLE_PROFILING

enum ProfileTag : uint8_t
{
    PROF_BUS_CLOCK = 0,
    
    PROF_CPU_CLOCK,
    PROF_CPU_OAM_DMA,

    PROF_PPU_RENDER_BG,
    PROF_PPU_RENDER_SPRITES,
    PROF_PPU_FINISH_SCANLINE,
    PROF_PPU_CPU_READ,
    PROF_PPU_CPU_WRITE,
    PROF_PPU_PPU_READ,
    PROF_PPU_PPU_WRITE,
    PROF_PPU_FAKE_SPRITE,

    PROF_BUS_CPU_READ,
    PROF_BUS_CPU_WRITE,
    PROF_BUS_RENDER_IMAGE,

    PROF_CART_PPU_SCANLINE,

    PROF_COUNT
};

struct ProfileEntry
{
    uint64_t cycles;
    uint32_t hits;
};

extern ProfileEntry g_profile[PROF_COUNT];
extern const char* const g_profile_names[PROF_COUNT];

struct ProfileScope
{
    uint8_t tag;
    uint32_t start;
    inline ProfileScope(uint8_t t) : tag(t), start(xthal_get_ccount())
    {
    }
    inline ~ProfileScope()
    {
        g_profile[tag].cycles += (uint32_t)(xthal_get_ccount() - start);
        g_profile[tag].hits++;
    }
};

void profileReport(uint32_t frame_interval = 60);

    #define PROFILE_SCOPE(tag) ProfileScope _prof_scope_##tag(tag)

#else

    #define PROFILE_SCOPE(tag)                                                                     \
        do {                                                                                       \
        } while (0)
inline void profileReport(uint32_t frame_interval = 60)
{
    (void)frame_interval;
}

#endif
#endif
