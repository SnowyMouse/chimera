// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HUD_BITMAP_SCALE_HPP
#define CHIMERA_HUD_BITMAP_SCALE_HPP

#include <cstdint>
#include <cstddef>

namespace Chimera {
    const std::uint16_t HUD_MARGIN = 8;

    // Safe zone margin. Xbox values are 48, 36. Gearbox is 8 (except on bottom edge because yes).
    const std::uint16_t XBOX_SAFE_ZONE_WIDTH = 48;
    const std::uint16_t XBOX_SAFE_ZONE_HEIGHT = 36;

    const std::uint16_t HUD_BASE_WIDTH = 640;
    const std::uint16_t HUD_BASE_HEIGHT = 480;

    extern "C" std::uint32_t safe_zone_x;
    extern "C" std::uint32_t safe_zone_y;
    extern "C" int hud_widescreen_right_edge_offset;

    void set_safe_zone_margins(std::uint32_t x, std::uint32_t y) noexcept;

    /**
     * Support highres scaling flags in bitmap tags: "hud scale 0.5" from MCC and
     * the custom "force hud use highres scale"
     */
    void set_up_hud_bitmap_scale_fix() noexcept;
}

#endif
