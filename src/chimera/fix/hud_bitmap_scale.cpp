// SPDX-License-Identifier: GPL-3.0-only

#include "hud_bitmap_scale.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/map.hpp"
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

    std::uint32_t scaled_bound_w = 1;
    std::uint32_t scaled_bound_h = 1;
    std::uint32_t scaled_reg_pt_w = 1;
    std::uint32_t scaled_reg_pt_h = 1;

    bool use_highres_scale_is_set;
}

namespace Chimera {

    extern "C" void check_half_scale_bitmap_flag(std::byte *bitmap) noexcept {
        auto tag_id = *reinterpret_cast<TagID *>(bitmap + 0x20);
        int scale = 1;

        // TagID shouldn't be null, but in case it is..
        if(!tag_id.is_null()) {
            auto *tag_data = get_tag(tag_id)->data;
            auto *usage_flags = reinterpret_cast<std::uint16_t *>(tag_data + 6);

            // Is 0.5 scale flag set?
            if((*usage_flags >> 4) & 1) {
                scale = 2;
            }

            // Is "force hud use highres scale" set in the bitmap tag AND "use highres scale" not set in the interface tag?
            // If so, make it scale by another half.
            if((*usage_flags >> 7) & 1 && !use_highres_scale_is_set) {
                scale *= 2;
            }
        }

        // Scale it.
        scaled_bound_w = *reinterpret_cast<std::uint16_t *>(bitmap + 0x4) / scale;
        scaled_bound_h = *reinterpret_cast<std::uint16_t *>(bitmap + 0x6) / scale;
        scaled_reg_pt_w = *reinterpret_cast<std::uint16_t *>(bitmap + 0x10) / scale;
        scaled_reg_pt_h = *reinterpret_cast<std::uint16_t *>(bitmap + 0x12) / scale;
    }

    void is_using_highres_scale() noexcept {
        use_highres_scale_is_set = true;
    }

    void set_up_hud_bitmap_scale_fix() noexcept {
        static Hook hook_w, hook_h;
        static Hook hud_static_hook, hud_meter_hook;
        static Hook mask, marker;
        static Hook unset_highres;

        // Scale bitmaps if the appropraite usage flag is set.
        write_function_override(get_chimera().get_signature("fix_counters_w_sig").data() + 4, hook_w, reinterpret_cast<const void *>(hud_scale_bitmap_bounds_w), &original_get_hud_bounds_w);
        write_function_override(get_chimera().get_signature("fix_counters_h_sig").data() + 4, hook_h, reinterpret_cast<const void *>(hud_scale_bitmap_bounds_h), &original_get_hud_bounds_h);

        // Game seems to only apply "use highres scale" flag on static elements, hud numbers and meters, despite what the tag might say.
        write_jmp_call(get_chimera().get_signature("hud_static_element_highres_sig").data(), hud_static_hook, reinterpret_cast<const void *>(is_using_highres_scale), nullptr);
        write_jmp_call(get_chimera().get_signature("hud_meter_element_highres_sig").data(), hud_meter_hook, reinterpret_cast<const void *>(is_using_highres_scale), nullptr);

        // Apply to scope mask as well
        auto *mask_ptr = get_chimera().get_signature("hud_mask_bitmap_scale_sig").data();
        SigByte nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(mask_ptr, nop);
        write_function_override(mask_ptr, mask, reinterpret_cast<const void *>(mask_scale_bitmap_bounds), &original_get_mask_bounds);

        // And friendly indicators as well.
        auto *marker_ptr = get_chimera().get_signature("hud_multi_marker_bitmap_scale_sig").data();
        SigByte nop2[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(marker_ptr, nop2);
        write_function_override(marker_ptr, marker, reinterpret_cast<const void *>(marker_scale_bitmap_bounds), &original_get_marker_bounds);

    }
}
