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
        auto *counter_numbers = reinterpret_cast<HUDNumber *>(get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC))->data);
        if(!counter_numbers) {
            return;
        }

        // Get bitmap tag
        auto *numbers_bitmap = get_bitmap_tag(counter_numbers->number_bitmap.tag_id);
        if(!numbers_bitmap) {
            return;
        }

        // Get the first sprite sheet bitmap
        auto *first_bitmap = GET_TAG_BLOCK_ELEMENT(BitmapData, &numbers_bitmap->bitmap_data, 0);
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

    static void jason_jones_sniper_ticks(WeaponHUDInterface *tag_data) noexcept {
        // 3 static elements? Probably stock sniper interface tag.
        if(tag_data->statics.count == 3) {
            auto *static_elements = GET_TAG_BLOCK_ELEMENT(WeaponHUDInterfaceStaticElement, &tag_data->statics, 0);

            // Make sure it's the thing we want to bullshit.
            if(static_elements[0].static_element.multitexture_overlays.count == 0 &&
                static_elements[1].static_element.multitexture_overlays.count == 1 &&
                static_elements[2].static_element.multitexture_overlays.count == 1 &&
                static_elements[0].static_element.placement.offset.x == 7 &&
                static_elements[0].static_element.placement.offset.y == 21 &&
                static_elements[1].static_element.placement.offset.x == 92 &&
                static_elements[1].static_element.placement.offset.y == 85 &&
                static_elements[2].static_element.placement.offset.x == 445 &&
                static_elements[2].static_element.placement.offset.y == 85) {

                static_elements[1].static_element.placement.offset.x = 132;
                static_elements[1].static_element.placement.offset.y = 113;
                static_elements[2].static_element.placement.offset.x = 484;
                static_elements[2].static_element.placement.offset.y = 113;
            }
        }

        // 2 static elements? Check for old refined sniper ticks and do a different hack here to fix incorrect multitexture blend mode usage
        // There is no need to do this on the demo version, as these tags were only used in modded stock maps that can be updated.
        else if(tag_data->statics.count == 2 && game_engine() != GameEngine::GAME_ENGINE_DEMO) {
            auto *static_elements = GET_TAG_BLOCK_ELEMENT(WeaponHUDInterfaceStaticElement, &tag_data->statics, 0);
            auto mc0 = static_elements[0].static_element.multitexture_overlays.count;
            auto mc1 = static_elements[1].static_element.multitexture_overlays.count;
            auto x0 = static_elements[0].static_element.placement.offset.x;
            auto y0 = static_elements[0].static_element.placement.offset.y;
            auto x1 = static_elements[1].static_element.placement.offset.x;
            auto y1 = static_elements[1].static_element.placement.offset.y;

            // y = 7: old refined, y = 9: newer refined
            if(mc0 == 1 && mc1 == 1 && x0 == -176 && (y0 == 7 || y0 == 9) && x1 == 176 && (y1 == 7 || y1 == 9)) {
                auto *m0 = GET_TAG_BLOCK_ELEMENT(HUDMultitextureOverlay, &static_elements[0].static_element.multitexture_overlays, 0);
                auto *m1 = GET_TAG_BLOCK_ELEMENT(HUDMultitextureOverlay, &static_elements[1].static_element.multitexture_overlays, 0);

                // Apply filth
                if(m0->map_blending_function[0] == HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_SUBTRACT && m1->map_blending_function[0] == HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_SUBTRACT) {
                    m0->map_blending_function[0] = HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_MULTIPLY2X;
                    m1->map_blending_function[0] = HUD_MULTITEXTURE_OVERLAY_BLEND_FUNCTION_MULTIPLY2X;
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
                auto *bitmap = GET_TAG_BLOCK_ELEMENT(BitmapData, &bitmap_group->bitmap_data, i);
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
                    jason_jones_sniper_ticks(reinterpret_cast<WeaponHUDInterface *>(tag.data));
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
