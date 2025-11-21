// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HUD_DEFS_HPP
#define CHIMERA_HUD_DEFS_HPP

#include <cstdint>
#include "type.hpp"
#include "tag.hpp"
#include "pad.hpp"

namespace Chimera {

    struct HudNumber {
        TagReference number_bitmap;
        std::int8_t character_width;
        std::int8_t screen_width;
        std::int8_t x_offset;
        std::int8_t y_offset;
        std::int8_t decimal_point_width;
        std::int8_t colon_width;
        PAD(0x4E);
    };
    static_assert(sizeof(HudNumber) == 0x64);

}

#endif