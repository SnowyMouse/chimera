// SPDX-License-Identifier: GPL-3.0-only


#include "jason_jones_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/map_load.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/tag_class.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/hud_defs.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

    static float highres_num_multipler = 0.5f;

    void jason_jones_numbers() noexcept {
        // No need to do this on demo
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            return;
        }

        highres_num_multipler = 0.5f;

        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);

        // Get the hud digits tag
        auto *counter_numbers = reinterpret_cast<HudNumber *>(get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC))->data);
        if(!counter_numbers) {
            return;
        }

        // Get bitmap tag
        auto *numbers_bitmap = get_bitmap_tag(counter_numbers->number_bitmap.tag_id);
        if(!numbers_bitmap) {
            return;
        }

        // Get the first sprite sheet bitmap
        auto *first_bitmap = get_bitmap_data_element(numbers_bitmap, 0);
        if(!first_bitmap) {
            return;
        }

        // If it's 2x size, set the 0.5 scale flag
        if(first_bitmap->width == 256 && first_bitmap->height <= 256) {
            // Check the number spacing
            if(counter_numbers->screen_width == 18) {
                // These are almost certainly using highres numbers, so meme it to work again.
                highres_num_multipler = 1.0f;
                SET_FLAG(numbers_bitmap->flags, BITMAP_FLAGS_HALF_HUD_SCALE_BIT, true);

                // Set stock tag values
                counter_numbers->character_width = 12;
                counter_numbers->screen_width = 9;
                counter_numbers->x_offset = 1;
                counter_numbers->y_offset = 0;
                counter_numbers->decimal_point_width = 6;
                counter_numbers->colon_width = 6;
            }
        }
    }

    static void jason_jones_sniper_ticks(std::byte *tag_data) noexcept {
        auto *static_element_count = reinterpret_cast<std::uint32_t *>(tag_data + 0x60);

        // 3 static elements? Probably stock sniper interface tag.
        if(*static_element_count == 3) {
            auto *static_element_data = *reinterpret_cast<std::byte **>(tag_data + 0x64);

            const auto *multitexture_count_0 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
            const auto *anchor_x0 = reinterpret_cast<short *>(static_element_data + 0x24);
            const auto *anchor_y0 = reinterpret_cast<short *>(static_element_data + 0x26);
            static_element_data += 0xB4;
            const auto *multitexture_count_1 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
            auto *anchor_x1 = reinterpret_cast<short *>(static_element_data + 0x24);
            auto *anchor_y1 = reinterpret_cast<short *>(static_element_data + 0x26);
            static_element_data += 0xB4;
            const auto *multitexture_count_2 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
            auto *anchor_x2 = reinterpret_cast<short *>(static_element_data + 0x24);
            auto *anchor_y2 = reinterpret_cast<short *>(static_element_data + 0x26);

            // Make sure it's the thing we want to bullshit.
            if(*multitexture_count_0 == 0 &&
                *anchor_x0 == 7 && *anchor_y0 == 21 &&
                *multitexture_count_1 == 1 && *anchor_x1 == 92 && *anchor_y1 == 85 &&
                *multitexture_count_2 == 1 && *anchor_x2 == 445 && *anchor_y2 == 85) {

                *anchor_x1 = 132;
                *anchor_y1 = 113;
                *anchor_x2 = 484;
                *anchor_y2 = 113;
            }
        }

        // 2 static elements? Check for old refined sniper ticks and do a different hack here to fix incorrect multitexture blend mode usage
        // There is no need to do this on the demo version, as these tags were only used in modded stock maps that can be updated.
        else if(*static_element_count == 2 && game_engine() != GameEngine::GAME_ENGINE_DEMO) {
            auto *static_element_data = *reinterpret_cast<std::byte **>(tag_data + 0x64);

            const auto *multitexture_overlay_count_0 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
            auto *multitexture_overlay_0 = *reinterpret_cast<std::byte **>(static_element_data + 0x7C + 4);
            const auto *anchor_x0 = reinterpret_cast<short *>(static_element_data + 0x24);
            const auto *anchor_y0 = reinterpret_cast<short *>(static_element_data + 0x26);

            static_element_data += 0xB4;
            const auto *multitexture_overlay_count_1 = reinterpret_cast<std::uint32_t *>(static_element_data + 0x7C);
            auto *multitexture_overlay_1 = *reinterpret_cast<std::byte **>(static_element_data + 0x7C + 4);
            const auto *anchor_x1 = reinterpret_cast<short *>(static_element_data + 0x24);
            const auto *anchor_y1 = reinterpret_cast<short *>(static_element_data + 0x26);

            if(*multitexture_overlay_count_0 == 1 &&
                *anchor_x0 == -176 && (*anchor_y0 == 7 || *anchor_y0 == 9) && // 7: old refined, 9: newer refined
                *multitexture_overlay_count_1 == 1 &&
                *anchor_x1 == 176 && (*anchor_y1 == 7 || *anchor_y1 == 9)) {
                auto *multitexture_blend_function_0 = reinterpret_cast<short *>(multitexture_overlay_0 + 0x2E); // zero to one blend function
                auto *multitexture_blend_function_1 = reinterpret_cast<short *>(multitexture_overlay_1 + 0x2E);

                // Apply filth
                if(*multitexture_blend_function_0 == 1 && *multitexture_blend_function_1 == 1) { // subtract
                    *multitexture_blend_function_0 = 3; // multiply2x
                    *multitexture_blend_function_1 = 3;
                }
            }
        }
    }

    // FIXME: This will dereference the bumpmap in shader_environment tags if they are p8 bump format.
    // This should be removed if software decoding for p8 bump is ever implemented.
    static void jason_jones_p8_bumps(std::byte *tag_data) noexcept {
        bool valid = true;
        TagReference *bump_map = reinterpret_cast<TagReference *>(tag_data + 0x128);
        if(!bump_map->tag_id.is_null()) {
            auto *bitmap_group = get_bitmap_tag(bump_map->tag_id);
            if(!bitmap_group) {
                return;
            }

            for(std::uint32_t i = 0; i < bitmap_group->bitmap_data.count; i++) {
                auto *bitmap = get_bitmap_data_element(bitmap_group, i);
                if(bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) {
                    valid = false;
                    break;
                }
            }

            if(!valid) {
                bump_map->tag_id = TagID::null_id();
            }
        }
    }

    void jason_jones_tag_fixup() noexcept {
        auto &tag_data_header = get_tag_data_header();
        for(std::uint32_t i = 0; i < tag_data_header.tag_count; i++) {
            auto &tag = tag_data_header.tag_array[i];
            if(!tag.data) {
                continue;
            }

            switch(tag.primary_class) {
                case TAG_CLASS_SHADER_ENVIRONMENT:
                    jason_jones_p8_bumps(tag.data);
                    break;
                case TAG_CLASS_WEAPON_HUD_INTERFACE:
                    jason_jones_sniper_ticks(tag.data);
                    break;
                default:
                    break;
            }
        }
    }

    void set_up_jason_jones_hacks() noexcept {
        add_map_load_event(jason_jones_tag_fixup);
        add_map_load_event(jason_jones_numbers);
        overwrite(get_chimera().get_signature("hud_number_element_highres_sig").data() + 2, &highres_num_multipler);
    }
}
