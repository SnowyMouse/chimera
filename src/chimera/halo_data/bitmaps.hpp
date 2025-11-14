// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_BITMAPS_HPP
#define CHIMERA_BITMAPS_HPP

#include <cstdint>
#include "type.hpp"
#include "tag.hpp"
#include "pad.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

    enum BitmapDataType : std::uint16_t {
        BITMAP_TYPE_2D,
        BITMAP_TYPE_3D,
        BITMAP_TYPE_CUBE_MAP,
        NUMBER_OF_BITMAP_TYPES
    };

    enum BitmapUsages : std::uint16_t {        
        BITMAP_USAGE_ADDITIVE,
        BITMAP_USAGE_MULTIPLICATIVE,
        BITMAP_USAGE_DETAIL,
        BITMAP_USAGE_VECTOR,
        NUMBER_OF_BITMAP_USAGES
    };

    enum BitmapDataFormat : std::uint16_t {
        BITMAP_FORMAT_A8,
        BITMAP_FORMAT_Y8,
        BITMAP_FORMAT_AY8,
        BITMAP_FORMAT_A8Y8,
        BITMAP_FORMAT_UNUSED1,
        BITMAP_FORMAT_UNUSED2,
        BITMAP_FORMAT_R5G6B5,
        BITMAP_FORMAT_UNUSED3,
        BITMAP_FORMAT_A1R5G5B5,
        BITMAP_FORMAT_A4R4G4B4,
        BITMAP_FORMAT_X8R8G8B8,
        BITMAP_FORMAT_A8R8G8B8,
        BITMAP_FORMAT_UNUSED4,
        BITMAP_FORMAT_UNUSED5,
        BITMAP_FORMAT_DXT1,
        BITMAP_FORMAT_DXT3,
        BITMAP_FORMAT_DXT5,
        BITMAP_FORMAT_P8_BUMP,
        NUMBER_OF_BITMAP_FORMATS,
    };

    struct BitmapDataFlags {
        // on-disk flags
        std::uint16_t power_of_two_dimensions : 1;
        std::uint16_t compressed : 1;
        std::uint16_t palettized : 1;
        std::uint16_t swizzled : 1;
        std::uint16_t linear : 1;
        std::uint16_t v16u16 : 1;
        std::uint16_t runtime_created : 1;
        std::uint16_t externel : 1;
        std::uint16_t environment : 1;
    };
    static_assert(sizeof(BitmapDataFlags) == 0x2);

    struct BitmapData {
        char signature[4];
        
        short width;
        short height;
        short depth;
        BitmapDataType type;
        BitmapDataFormat format;
        BitmapDataFlags flags;
        
        Vector2DInt registration_point;
        
        short mipmap_count;
        short pad;
        long pixels_offset;
        long pixels_size;
        TagID tag_id;
        
        long cache_block_index;
        void *hardware_format;
        void *base_address;
    };
    static_assert(sizeof(BitmapData) == 0x30);

    struct Bitmap {
        //fill in the rest later lol.
        PAD(0x60);

        TagBlock bitmap_data;
    };
    static_assert(sizeof(Bitmap) == 0x6C);

}


#endif
