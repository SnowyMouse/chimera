// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_BITMAPS_HPP
#define CHIMERA_BITMAPS_HPP

#include <cstdint>
#include "type.hpp"
#include "tag.hpp"
#include "pad.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

    enum BitmapFlags {
        BITMAP_FLAGS_DIFFUSION_DITHER_BIT,
        BITMAP_FLAGS_DISABLE_VECTOR_COMPRESSION_BIT,
        BITMAP_FLAGS_UNIFORM_SPRITE_SEQUENCES_BIT,
        BITMAP_FLAGS_EXTRACT_SPRITES_FILTHY_BUG_FIX_BIT,
        BITMAP_FLAGS_HALF_HUD_SCALE_BIT,
        BITMAP_FLAGS_INVERT_DETAIL_FADE_BIT,
        BITMAP_FLAGS_USE_AVERAGE_COLOR_FOR_DETAIL_FADE_BIT,
        BITMAP_FLAGS_FORCE_HUD_USE_HIGHRES_SCALE_BIT,
        NUMBER_OF_BITMAP_FLAGS
    };

    enum BitmapDataFlags {
        BITMAP_DATA_FLAGS_POWER_OF_TWO_DIMENSIONS_BIT,
        BITMAP_DATA_FLAGS_COMPRESSED_BIT,
        BITMAP_DATA_FLAGS_PALETTIZED_BIT,
        BITMAP_DATA_FLAGS_SWIZZLED_BIT,
        BITMAP_DATA_FLAGS_LINEAR_BIT,
        BITMAP_DATA_FLAGS_V16U16_BIT,
        BITMAP_DATA_FLAGS_RUNTIME_CREATED_BIT,
        BITMAP_DATA_FLAGS_CACHED_BIT,
        BITMAP_DATA_FLAGS_EXTERNAL_BIT,
        BITMAP_DATA_FLAGS_ENVIRONMENT_BIT,
        NUMBER_OF_BITMAP_DATA_FLAGS
    };

    enum BitmapType : std::uint16_t {
        BITMAP_TYPE_2D_TEXTURES,
        BITMAP_TYPE_3D_TEXTURES,
        BITMAP_TYPE_CUBE_MAPS,
        BITMAP_TYPE_SPRITES,
        BITMAP_TYPE_INTERFACE_BITMAPS,
        NUMBER_OF_BITMAP_TYPES
    };

    enum BitmapFormat : std::uint16_t {
        BITMAP_FORMAT_COMPRESSED_COLOR_KEY_TRANSPARENCY,
        BITMAP_FORMAT_COMPRESSED_EXPLICIT_ALPHA,
        BITMAP_FORMAT_COMPRESSED_INTERPOLATED_ALPHA,
        BITMAP_FORMAT_16BIT_COLOR,
        BITMAP_FORMAT_32BIT_COLOR,
        BITMAP_FORMAT_MONOCHROME,
        BITMAP_FORMAT_HIGH_QUALITY_COMPRESSION,
        NUMBER_OF_BITMAP_FORMATS
    };

    enum BitmapUsage : std::uint16_t {
        BITMAP_USAGE_ADDITIVE,
        BITMAP_USAGE_MULTIPLICATIVE,
        BITMAP_USAGE_DETAIL,
        BITMAP_USAGE_VECTOR,
        NUMBER_OF_BITMAP_USAGES
    };

    enum BitmapSpriteBudget : std::uint16_t {
        BITMAP_SPRITE_BUDGET_32,
        BITMAP_SPRITE_BUDGET_64,
        BITMAP_SPRITE_BUDGET_128,
        BITMAP_SPRITE_BUDGET_256,
        BITMAP_SPRITE_BUDGET_512,
        BITMAP_SPRITE_BUDGET_1024,
        NUMBER_OF_BITMAP_SPRITE_BUDGETS
    };

    enum BitmapSpriteUsage : std::uint16_t {
        BITMAP_SPRITE_USAGE_BLEND_ADD_SUB_MAX,
        BITMAP_SPRITE_USAGE_MUL_MIN,
        BITMAP_SPRITE_USAGE_DOUBLE_MULTIPLY,
        NUMBER_OF_BITMAP_SPRITE_USAGES
    };

    enum BitmapDataType : std::uint16_t {
        BITMAP_DATA_TYPE_2D,
        BITMAP_DATA_TYPE_3D,
        BITMAP_DATA_TYPE_CUBE_MAP,
        NUMBER_OF_BITMAP_DATA_TYPES
    };

    enum BitmapDataFormat : std::uint16_t {
        BITMAP_DATA_FORMAT_A8,
        BITMAP_DATA_FORMAT_Y8,
        BITMAP_DATA_FORMAT_AY8,
        BITMAP_DATA_FORMAT_A8Y8,
        BITMAP_DATA_FORMAT_UNUSED1,
        BITMAP_DATA_FORMAT_UNUSED2,
        BITMAP_DATA_FORMAT_R5G6B5,
        BITMAP_DATA_FORMAT_UNUSED3,
        BITMAP_DATA_FORMAT_A1R5G5B5,
        BITMAP_DATA_FORMAT_A4R4G4B4,
        BITMAP_DATA_FORMAT_X8R8G8B8,
        BITMAP_DATA_FORMAT_A8R8G8B8,
        BITMAP_DATA_FORMAT_UNUSED4,
        BITMAP_DATA_FORMAT_UNUSED5,
        BITMAP_DATA_FORMAT_DXT1,
        BITMAP_DATA_FORMAT_DXT3,
        BITMAP_DATA_FORMAT_DXT5,
        BITMAP_DATA_FORMAT_P8_BUMP,
        BITMAP_DATA_FORMAT_BC7,
        NUMBER_OF_BITMAP_DATA_FORMATS
    };

    struct BitmapSprite {
        std::int16_t bitmap_index;
        PAD(0x6);
        Bounds2D bounds;
        Point2D registration_point;
    };
    static_assert(sizeof(BitmapSprite) == 0x20);

    struct BitmapSequence {
        TagString name;
        std::int16_t first_bitmap_index;
        std::int16_t bitmap_count;
        PAD(0x10);
        TagBlock sprites;
    };
    static_assert(sizeof(BitmapSequence) == 0x40);

    struct BitmapData {
        TagClassInt signature;
        std::int16_t width;
        std::int16_t height;
        std::int16_t depth;
        BitmapDataType type;
        BitmapDataFormat format;
        std::uint16_t flags;
        Vector2DInt registration_point;
        std::int16_t mipmap_count;
        PAD(0x2);
        std::int32_t pixels_offset;
        std::int32_t pixels_size;
        TagID tag_id;
        std::int32_t cache_block_index;
        void *hardware_format;
        void *base_address;
    };
    static_assert(sizeof(BitmapData) == 0x30);

    struct Bitmap {
        BitmapType type;
        BitmapFormat format;
        BitmapUsage usage;
        std::uint16_t flags;
        float detail_fade;
        float sharpen_amount;
        float bump_height;
        std::int16_t sprite_budget_size;
        std::int16_t sprite_budget_count;
        std::int16_t import_width;
        std::int16_t import_height;
        TagData import_bitmap;
        TagData pixel_data;
        float smoothing_filter_size;
        float alpha_bias;
        std::int16_t mipmap_count;
        std::int16_t sprite_usage;
        std::int16_t sprite_spacing;
        PAD(0x2);
        TagBlock sequences;
        TagBlock bitmap_data;
    };
    static_assert(sizeof(Bitmap) == 0x6C);

    /**
     * Get bitmap tag data
     * @param  tag_id id of the tag
     * @return pointer to the tag data if found, nullptr if not
     */
    Bitmap *get_bitmap_tag(TagID tag_id) noexcept;

}

#endif
