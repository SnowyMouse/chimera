// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_HACKS_HPP
#define CHIMERA_MAP_HACKS_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Chimera {

    // Not all of these config option are currently valid.
    struct ChimeraFixes {
        bool gearbox_chicago_multiply; // Use gearbox shader_transparent_chicago multiply behaviour
        bool gearbox_meters; // Use the gearbox fixed function meters instead of the Xbox shader.
        bool gearbox_multitexture_blend_modes; // Use gearbox order multitexture blend functions
        bool alternate_bump_attenuation; // Force enable the alternate bump attenuation in shader_environment_lightmap
        bool gearbox_bump_attenuation; // Force the gearbox bump attenuation (ie none) in shader_environment_lightmap
        bool invert_detail_after_reflection; // Flips the flag in shader model tags
        bool embedded_lua; // Doesn't do anything yet
        bool hud_number_scale; // Hacks the tag to scale down oversized HUD counter numbers.
        bool disable_bitmap_hud_scale_flags; // Clears HUD scale flags from internal bitmap tags
    };

    struct MapFixConfig {
        char map_name[32];
        std::uint32_t tags_checksum;
        ChimeraFixes config;
    };

    extern ChimeraFixes global_fix_flags;
    extern MapFixConfig chimera_fix_blacklist[];
    extern std::uint32_t blacklist_map_count;
    extern bool map_hacks_debug_enabled;

    /**
     * Enable/disable fixes that may or may not break old maps.
     */
    void set_up_map_config_control() noexcept;

}

#endif
