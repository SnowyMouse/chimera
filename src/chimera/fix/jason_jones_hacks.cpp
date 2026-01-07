// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "jason_jones_hacks.hpp"
#include "map_hacks/map_hacks.hpp"
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
        if(!global_fix_flags.hud_number_scale) {
            highres_num_multipler = 0.5f;
            return;
        }

        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);

        // Get the hud digits tag
        auto *tag = get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC));
        if(!tag) {
            return;
        }

        auto *counter_numbers = reinterpret_cast<HUDNumber *>(tag->data);
        if(!counter_numbers) {
            return;
        }

        // Get bitmap tag
        auto *numbers_bitmap = get_bitmap_tag(counter_numbers->number_bitmap.tag_id);
        if(!numbers_bitmap) {
            return;
        }

        // Meme the numbers into working again
        highres_num_multipler = 1.0f;
        SET_FLAG(numbers_bitmap->flags, BITMAP_FLAGS_HALF_HUD_SCALE_BIT, true);

        // Divide these by 2, rounding up
        counter_numbers->character_width = std::ceil(static_cast<float>(counter_numbers->character_width) / 2);
        counter_numbers->screen_width = std::ceil(static_cast<float>(counter_numbers->screen_width) / 2);
        counter_numbers->x_offset = std::ceil(static_cast<float>(counter_numbers->x_offset) / 2);
        counter_numbers->y_offset = std::ceil(static_cast<float>(counter_numbers->y_offset) / 2);
        counter_numbers->decimal_point_width = std::ceil(static_cast<float>(counter_numbers->decimal_point_width) / 2);
        counter_numbers->colon_width = std::ceil(static_cast<float>(counter_numbers->colon_width) / 2);
    }

    static void jason_jones_sniper_ticks(WeaponHUDInterface *tag_data) noexcept {
        // 3 static elements? Probably stock sniper interface tag.
        if(tag_data->statics.count == 3 && tag_data->absolute_placement.anchor == HUD_ANCHOR_TOP_LEFT) {
            auto *s1 = GET_TAG_BLOCK_ELEMENT(WeaponHUDInterfaceStaticElement, &tag_data->statics, 1);
            auto *s2 = GET_TAG_BLOCK_ELEMENT(WeaponHUDInterfaceStaticElement, &tag_data->statics, 2);

            // Make sure it's the thing we want to bullshit.
            if(s1->header.child_anchor == HUD_CHILD_ANCHOR_FROM_PARENT &&
                s2->header.child_anchor == HUD_CHILD_ANCHOR_FROM_PARENT &&
                s1->static_element.multitexture_overlays.count == 1 &&
                s2->static_element.multitexture_overlays.count == 1 &&
                s1->static_element.placement.offset.x == 92 &&
                s1->static_element.placement.offset.y == 85 &&
                s2->static_element.placement.offset.x == 445 &&
                s2->static_element.placement.offset.y == 85
            ) {
                s1->static_element.placement.offset.x = -176;
                s2->static_element.placement.offset.x = 176;
                s1->static_element.placement.offset.y = s2->static_element.placement.offset.y = 9;
                s1->header.child_anchor = s2->header.child_anchor = HUD_CHILD_ANCHOR_CENTER;
            }
        }
    }

    // FIXME: This will dereference the bumpmap in shader_environment tags if they are p8 bump format.
    // This should be removed if software decoding for p8 bump is ever implemented.
    static void jason_jones_p8_bumps(ShaderEnvironment *tag_data) noexcept {
        if(tag_data->environment.diffuse.bump_map.tag_id.is_null()) {
            return;
        }

        auto *bitmap_group = get_bitmap_tag(tag_data->environment.diffuse.bump_map.tag_id);
        if(!bitmap_group) {
            return;
        }

        bool valid = true;
        for(std::uint32_t i = 0; i < bitmap_group->bitmap_data.count; i++) {
            auto *bitmap = GET_TAG_BLOCK_ELEMENT(BitmapData, &bitmap_group->bitmap_data, i);
            if(bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) {
                valid = false;
                break;
            }
        }

        if(!valid) {
            tag_data->environment.diffuse.bump_map.tag_id = TagID::null_id();
        }
    }

    static void jason_jones_detail_after_reflection(ShaderModel *tag_data) noexcept {
        if(!global_fix_flags.invert_detail_after_reflection) {
            return;
        }

        // Flip detail after reflection flag.
        SWAP_FLAG(tag_data->model.flags, SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT);
    }

    static void jason_jones_unintended_hud_scale(Bitmap *tag_data) noexcept {
        if(!global_fix_flags.disable_bitmap_hud_scale_flags) {
            return;
        }

        // Clear HUD scale flags.
        SET_FLAG(tag_data->flags, BITMAP_FLAGS_HALF_HUD_SCALE_BIT, false);
        SET_FLAG(tag_data->flags, BITMAP_FLAGS_FORCE_HUD_USE_HIGHRES_SCALE_BIT, false);
    }

    void jason_jones_tag_fixup() noexcept {
        auto &tag_data_header = get_tag_data_header();
        for(std::uint32_t i = 0; i < tag_data_header.tag_count; i++) {
            auto &tag = tag_data_header.tag_array[i];

            // Don't touch external tags. If this changes later make sure not to touch external HUD scale flags
            if(!tag.data || tag.externally_loaded) {
                continue;
            }

            switch(tag.primary_class) {
                case TAG_CLASS_BITMAP:
                    jason_jones_unintended_hud_scale(reinterpret_cast<Bitmap *>(tag.data));
                    break;
                case TAG_CLASS_SHADER_ENVIRONMENT:
                    jason_jones_p8_bumps(reinterpret_cast<ShaderEnvironment *>(tag.data));
                    break;
                case TAG_CLASS_SHADER_MODEL:
                    jason_jones_detail_after_reflection(reinterpret_cast<ShaderModel *>(tag.data));
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
