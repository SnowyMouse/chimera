// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FONT_HPP
#define CHIMERA_FONT_HPP

#include <cstdint>
#include "type.hpp"
#include "tag.hpp"
#include "pad.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

    enum FontFlags {
        FONT_FLAGS_DISABLE_FONT_OVERRIDE_BIT,
        NUMBER_OF_FONT_FLAGS
    };

    struct FontCharacterTableEntry {
        std::uint16_t character_index;
    };
    static_assert(sizeof(struct FontCharacterTableEntry) == 0x2);

    struct FontCharacterTablesEntry {
        TagBlock table;
    };
    static_assert(sizeof(struct FontCharacterTablesEntry) == 0xC);

    struct FontCharacter {
        std::uint16_t character;
        std::int16_t character_width;
        std::int16_t bitmap_width;
        std::int16_t bitmap_height;
        Vector2DInt bitmap_origin;
        std::uint16_t hardware_character_index;
        PAD(0x2);
        std::uint32_t pixels_offset;
    };
    static_assert(sizeof(struct FontCharacter) == 0x14);

    struct Font {
        std::uint32_t flags;
        std::int16_t ascending_height;
        std::int16_t descending_height;
        std::int16_t leading_height;
        std::int16_t leading_width;
        PAD(0x24);
        TagBlock character_tables;
        TagReference style_fonts[4];
        TagBlock characters;
        TagData pixels;
    };
    static_assert(sizeof(Font) == 0x9C);

}

#endif
