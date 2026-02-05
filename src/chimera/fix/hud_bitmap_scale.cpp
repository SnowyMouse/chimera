// SPDX-License-Identifier: GPL-3.0-only

#include "hud_bitmap_scale.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/hud_defs.hpp"
#include "../halo_data/resolution.hpp"
#include "../halo_data/game_engine.hpp"
#include "../event/frame.hpp"
#include "widescreen_fix.hpp"
#include "map_hacks/map_hacks.hpp"


namespace Chimera {
    extern "C" {
        void hud_scale_bitmap_bounds_w() noexcept;
        void hud_scale_bitmap_bounds_h() noexcept;
        const void *original_get_hud_bounds_w = nullptr;
        const void *original_get_hud_bounds_h = nullptr;

        void mask_scale_bitmap_bounds() noexcept;
        const void *original_get_mask_bounds = nullptr;

        void marker_scale_bitmap_bounds() noexcept;
        const void *original_get_marker_bounds = nullptr;

        void nav_number_offset_bitmap_bounds_w() noexcept;
        void nav_number_offset_bitmap_bounds_h() noexcept;
        const void *original_get_nav_bounds_w = nullptr;
        const void *original_get_nav_bounds_h = nullptr;

        std::uint32_t scaled_bound_w = 1;
        std::uint32_t scaled_bound_h = 1;
        std::uint32_t scaled_reg_pt_w = 1;
        std::uint32_t scaled_reg_pt_h = 1;

        bool use_highres_scale_is_set;

        void hud_static_element_child_anchor_asm() noexcept;
        void hud_meter_element_child_anchor_asm() noexcept;
        void hud_number_element_child_anchor_asm() noexcept;
        void hud_overlay_element_child_anchor_asm() noexcept;

        void hud_calculate_point_for_scaled_canvas_asm() noexcept;
        void hud_set_point_values_for_scaled_canvas_asm() noexcept;
        const void *original_set_point_instr = nullptr;

        float point_temp_result_x = 0;
        float point_temp_result_y = 0;

        std::uint32_t safe_zone_x = 0;
        std::uint32_t safe_zone_y = 0;

        void hud_offset_nav_numbers_for_safe_zones() noexcept;

        void hud_offset_left_damage_indicator() noexcept;
        void hud_offset_right_damage_indicator() noexcept;
        void hud_offset_top_damage_indicator() noexcept;
        void hud_offset_bottom_damage_indicator() noexcept;

        void hud_offset_left_damage_indicator_demo() noexcept;
        void hud_offset_right_damage_indicator_demo() noexcept;
        void hud_offset_top_damage_indicator_demo() noexcept;
        void hud_offset_bottom_damage_indicator_demo() noexcept;

        void hud_offset_gametype_indicator() noexcept;

        int hud_widescreen_right_edge_offset = 0;
    }

    static std::uint16_t child_anchor = 0;

    static float window_width = HUD_BASE_WIDTH - HUD_MARGIN;
    static float window_height = HUD_BASE_HEIGHT - HUD_MARGIN;

    static float viewport_width = HUD_BASE_WIDTH;
    static float viewport_height = HUD_BASE_HEIGHT;
    static float half_viewport_width = HUD_BASE_WIDTH / 2.0f;

    static std::uint32_t config_safe_zone_x = HUD_MARGIN;
    static std::uint32_t config_safe_zone_y = HUD_MARGIN;

    void set_safe_zone_margins(std::uint32_t x, std::uint32_t y) noexcept {
        config_safe_zone_x = PIN(x, HUD_MARGIN, XBOX_SAFE_ZONE_WIDTH);
        config_safe_zone_y = PIN(y, HUD_MARGIN, XBOX_SAFE_ZONE_HEIGHT);
    }

    void update_safe_zones(std::uint32_t x, std::uint32_t y) noexcept {
        safe_zone_x = x - HUD_MARGIN;
        safe_zone_y = y - HUD_MARGIN;
        window_width = HUD_BASE_WIDTH - (safe_zone_x - HUD_MARGIN) * 2.0f;
        window_height = HUD_BASE_HEIGHT - (safe_zone_y - HUD_MARGIN) * 2.0f;
    }

    void update_viewport_bounds() noexcept {
        if(global_fix_flags.old_widescreen_fix) {
            viewport_width = HUD_BASE_WIDTH;
            hud_widescreen_right_edge_offset = 0;
            update_safe_zones(HUD_MARGIN, HUD_MARGIN);
        }
        else {
            float aspect_ratio = static_cast<float>(get_resolution().width) / static_cast<float>(get_resolution().height);
            switch(get_widescreen_fix()) {
                case WIDESCREEN_CENTER_HUD: {
                    viewport_width = aspect_ratio >= (4.f / 3.f) ? HUD_BASE_HEIGHT * (4.f / 3.f) : HUD_BASE_HEIGHT * aspect_ratio;
                    hud_widescreen_right_edge_offset = 0;
                    break;
                }
                case WIDESCREEN_16_9_HUD: {
                    viewport_width = aspect_ratio >= (16.f / 9.f) ? HUD_BASE_HEIGHT * (16.f / 9.f) : HUD_BASE_HEIGHT * aspect_ratio;
                    hud_widescreen_right_edge_offset = static_cast<int>(viewport_width) - HUD_BASE_WIDTH;
                    break;
                }
                case WIDESCREEN_ON: {
                    viewport_width = HUD_BASE_HEIGHT * aspect_ratio;
                    hud_widescreen_right_edge_offset = static_cast<int>(viewport_width) - HUD_BASE_WIDTH;
                    break;
                }
                case WIDESCREEN_OFF: {
                    viewport_width = HUD_BASE_WIDTH;
                    hud_widescreen_right_edge_offset = 0;
                }
            }
            update_safe_zones(config_safe_zone_x, config_safe_zone_y);
        }
 
        half_viewport_width = viewport_width / 2.0f;
    }

    extern "C" void check_half_scale_bitmap_flag(BitmapData *bitmap, bool highres_scale) noexcept {
        int scale = 1;

        // This should always be valid, but it might not be.
        auto *bitmap_group = get_bitmap_tag(bitmap->tag_id);
        if(bitmap_group) {
            // Is 0.5 scale flag set?
            if(TEST_FLAG(bitmap_group->flags, BITMAP_FLAGS_HALF_HUD_SCALE_BIT)) {
                scale = 2;
            }

            // Is "force hud use highres scale" set in the bitmap tag AND "use highres scale" not set in the interface tag?
            // If so, make it scale by another half.
            if(TEST_FLAG(bitmap_group->flags, BITMAP_FLAGS_FORCE_HUD_USE_HIGHRES_SCALE_BIT) && !highres_scale) {
                scale *= 2;
            }
        }

        // Scale it.
        scaled_bound_w = bitmap->width / scale;
        scaled_bound_h = bitmap->height / scale;
        scaled_reg_pt_w = bitmap->registration_point.x / scale;
        scaled_reg_pt_h = bitmap->registration_point.y / scale;
    }

    void is_using_highres_scale() noexcept {
        use_highres_scale_is_set = true;
    }

    extern "C" void hud_static_element_use_child_anchors(std::uint16_t **anchor, WeaponHUDInterfaceStaticElement *element) noexcept {
        child_anchor = 0;
        if(element->header.child_anchor > 0 && element->header.child_anchor < 6) {
            child_anchor = element->header.child_anchor - 1;
            *anchor = &child_anchor;
        }
    }

    extern "C" void hud_meter_element_use_child_anchors(std::uint16_t **anchor, WeaponHUDInterfaceMeterElement *element) noexcept {
        child_anchor = 0;
        if(element->header.child_anchor > 0 && element->header.child_anchor < 6) {
            child_anchor = element->header.child_anchor - 1;
            *anchor = &child_anchor;
        }
    }

    extern "C" void hud_number_element_use_child_anchors(std::uint16_t **anchor, WeaponHUDInterfaceNumberElement *element) noexcept {
        child_anchor = 0;
        if(element->header.child_anchor > 0 && element->header.child_anchor < 6) {
            child_anchor = element->header.child_anchor - 1;
            *anchor = &child_anchor;
        }
    }

    extern "C" void hud_overlay_element_use_child_anchors(std::uint16_t **anchor, WeaponHUDInterfaceOverlaysElement *element) noexcept {
        child_anchor = 0;
        if(element->header.child_anchor > 0 && element->header.child_anchor < 6) {
            child_anchor = element->header.child_anchor - 1;
            *anchor = &child_anchor;
        }
    }

    extern "C" void hud_calculate_point_for_scaled_canvas(float scale, HUDPlacement *placement, HUDAbsolutePlacement *absolute_placement) noexcept {
        float highres_scale = TEST_FLAG(placement->scaling_flags, HUD_SCALE_FLAGS_USE_HIGHRES_ANCHOR_OFFSETS) ? 0.5f : 1.0f;
        float padding_x = safe_zone_x + HUD_MARGIN;
        float padding_y = safe_zone_y + HUD_MARGIN;

        if(absolute_placement->anchor < HUD_ANCHOR_CENTER) {
            point_temp_result_x = ((absolute_placement->anchor & 1) ? -1 : 1) * placement->offset.x * highres_scale * scale + ((absolute_placement->anchor & 1) ? (viewport_width - padding_x) : padding_x);
            point_temp_result_y = ((absolute_placement->anchor & 2) ? -1 : 1) * placement->offset.y * highres_scale * scale + ((absolute_placement->anchor & 2) ? (viewport_height - padding_y) : padding_y);
        }
        else {
            point_temp_result_x = placement->offset.x * highres_scale * scale + (viewport_width / 2.0f);
            point_temp_result_y = placement->offset.y * highres_scale * scale + (viewport_height / 2.0f);
        }
    }

    void set_up_hud_bitmap_scale_fix() noexcept {
        static Hook hook_w, hook_h;
        static Hook hud_static_hook, hud_meter_hook;
        static Hook unset_highres;

        // Scale bitmaps if the appropraite usage flag is set.
        write_function_override(get_chimera().get_signature("fix_counters_w_sig").data() + 4, hook_w, reinterpret_cast<const void *>(hud_scale_bitmap_bounds_w), &original_get_hud_bounds_w);
        write_function_override(get_chimera().get_signature("fix_counters_h_sig").data() + 4, hook_h, reinterpret_cast<const void *>(hud_scale_bitmap_bounds_h), &original_get_hud_bounds_h);

        // Game seems to only apply "use highres scale" flag on static elements, hud numbers and meters, despite what the tag might say.
        write_jmp_call(get_chimera().get_signature("hud_static_element_highres_sig").data(), hud_static_hook, reinterpret_cast<const void *>(is_using_highres_scale), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_meter_element_highres_sig").data(), hud_meter_hook, reinterpret_cast<const void *>(is_using_highres_scale), nullptr);

        // Apply to scope mask as well.
        auto *mask_ptr = get_chimera().get_signature("hud_mask_bitmap_scale_sig").data();
        static Hook mask;
        SigByte nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(mask_ptr, nop);
        write_function_override(mask_ptr, mask, reinterpret_cast<const void *>(mask_scale_bitmap_bounds), &original_get_mask_bounds);

        // And friendly indicators.
        auto *marker_ptr = get_chimera().get_signature("hud_multi_marker_bitmap_scale_sig").data();
        static Hook marker;
        SigByte nop2[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(marker_ptr, nop2);
        write_function_override(marker_ptr, marker, reinterpret_cast<const void *>(marker_scale_bitmap_bounds), &original_get_marker_bounds);

        // Make sure nav point numbers have correct offset for scaled bitmaps.
        auto *nav_w_ptr = get_chimera().get_signature("hud_nav_marker_number_offset_w_sig").data();
        auto *nav_h_ptr = get_chimera().get_signature("hud_nav_marker_number_offset_h_sig").data();
        static Hook nav_w, nav_h;
        SigByte nop3[] = {0x90, 0x90, 0x90, 0x90};
        write_code_s(nav_w_ptr, nop3);
        write_code_s(nav_h_ptr, nop3);
        write_function_override(nav_w_ptr, nav_w, reinterpret_cast<const void *>(nav_number_offset_bitmap_bounds_w), &original_get_nav_bounds_w);
        write_function_override(nav_h_ptr, nav_h, reinterpret_cast<const void *>(nav_number_offset_bitmap_bounds_h), &original_get_nav_bounds_h);

        // Child anchors.
        static Hook static_element_anchor, meter_element_anchor, number_element_anchor, overlay_element_anchor;
        write_jmp_call(get_chimera().get_signature("hud_weapon_draw_static_element_sig").data(), static_element_anchor, reinterpret_cast<const void *>(hud_static_element_child_anchor_asm), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_weapon_draw_meter_element_sig").data(), meter_element_anchor, reinterpret_cast<const void *>(hud_meter_element_child_anchor_asm), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_weapon_draw_number_element_sig").data(), number_element_anchor, reinterpret_cast<const void *>(hud_number_element_child_anchor_asm), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_weapon_draw_overlay_element_sig").data(), overlay_element_anchor, reinterpret_cast<const void *>(hud_overlay_element_child_anchor_asm), nullptr);

        // Anchor offsets
        auto *canvas_scale_1 = get_chimera().get_signature("hud_canvas_scale_1_sig").data();
        auto *canvas_scale_2 = get_chimera().get_signature("hud_canvas_scale_2_sig").data();
        SigByte nop4[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

        write_code_s(canvas_scale_2, nop4);
        static Hook canvas_hook_1, canvas_hook_2;
        write_jmp_call(canvas_scale_1, canvas_hook_1, reinterpret_cast<const void *>(hud_calculate_point_for_scaled_canvas_asm), nullptr);
        write_function_override(canvas_scale_2, canvas_hook_2, reinterpret_cast<const void *>(hud_set_point_values_for_scaled_canvas_asm), &original_set_point_instr);

        // Meme the nav indicators back into exsistence if safe zones are set.
        static Hook number_memes;
        auto *hud_numbers_offset = get_chimera().get_signature("hud_numbers_offset_sig").data();
        auto *hud_numbers_canvas = get_chimera().get_signature("hud_numbers_canvas_size_sig").data();

        write_jmp_call(hud_numbers_offset, number_memes, nullptr, reinterpret_cast<const void *>(hud_offset_nav_numbers_for_safe_zones), false);
        overwrite(hud_numbers_canvas + 2, &window_width);
        overwrite(hud_numbers_canvas + 26, &window_height);

        // Damage indicators for safe zones.
        static Hook top_ind, left_ind, bottom_ind, right_ind;
        auto *top = get_chimera().get_signature("hud_damage_indicator_top_sig").data();
        auto *left = get_chimera().get_signature("hud_damage_indicator_left_sig").data();
        auto *bottom = get_chimera().get_signature("hud_damage_indicator_bottom_sig").data();
        auto *right= get_chimera().get_signature("hud_damage_indicator_right_sig").data();
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            write_jmp_call(top, top_ind, reinterpret_cast<const void *>(hud_offset_top_damage_indicator_demo), nullptr, false);
            write_jmp_call(left, left_ind, reinterpret_cast<const void *>(hud_offset_left_damage_indicator_demo), nullptr, false);
            write_jmp_call(bottom, bottom_ind, reinterpret_cast<const void *>(hud_offset_bottom_damage_indicator_demo), nullptr, false);
            write_jmp_call(right, right_ind, reinterpret_cast<const void *>(hud_offset_right_damage_indicator_demo), nullptr, false);
        }
        else {
            write_jmp_call(top, top_ind, reinterpret_cast<const void *>(hud_offset_top_damage_indicator), nullptr, false);
            write_jmp_call(left, left_ind, reinterpret_cast<const void *>(hud_offset_left_damage_indicator), nullptr, false);
            write_jmp_call(bottom, bottom_ind, reinterpret_cast<const void *>(hud_offset_bottom_damage_indicator), nullptr, false);
            write_jmp_call(right, right_ind, reinterpret_cast<const void *>(hud_offset_right_damage_indicator), nullptr, false);
        }
        overwrite(top + 22, &half_viewport_width);
        overwrite(bottom + 22, &half_viewport_width);

        // Gametype indicator for safe zones.
        static Hook gametype_1, gametype_2;
        write_jmp_call(get_chimera().get_signature("hud_gametype_indicator_draw_1_sig").data(), gametype_1, reinterpret_cast<const void *>(hud_offset_gametype_indicator), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_gametype_indicator_draw_2_sig").data(), gametype_2, reinterpret_cast<const void *>(hud_offset_gametype_indicator), nullptr);

        add_preframe_event(update_viewport_bounds);
    }
}
