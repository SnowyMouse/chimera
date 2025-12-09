// SPDX-License-Identifier: GPL-3.0-only

#include "hud_bitmap_scale.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/hud_defs.hpp"
#include "../event/map_load.hpp"

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
}

namespace Chimera {

    static std::uint16_t child_anchor = 0;

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
    }
}
