// SPDX-License-Identifier: GPL-3.0-only

#include "nav_numbers.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../event/map_load.hpp"
#include "../event/frame.hpp"

namespace Chimera {
    static bool can_jason_jones = false;
    static bool jason_jones = false;
    static int div;

    static std::uint16_t *dimensions = nullptr;
    static std::uint16_t old_dimensions[2];
    static std::uint8_t *spacing, old_spacing, new_spacing;

    static void on_map_load() {
        dimensions = nullptr;
        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);

        // Get the hud digits tag
        auto *tag = get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC));
        if(!tag) {
            return;
        }
        auto *tag_data = tag->data;

        // Get bitmap tag
        auto *bitmap_tag = get_tag(*reinterpret_cast<TagID *>(tag_data + 0xC));
        if(!bitmap_tag) {
            return;
        }

        // Get bitmap tag data
        auto *bitmap_tag_data = bitmap_tag->data;
        if(!*reinterpret_cast<std::uint32_t *>(bitmap_tag_data + 0x60)) {
            return;
        }

        // Get old data
        auto *bitmap = *reinterpret_cast<std::byte **>(bitmap_tag_data + 0x64);
        auto *maybe_dimensions = reinterpret_cast<std::uint16_t *>(bitmap + 0x4);

        // Make sure we can work here
        if(maybe_dimensions[0] <= 128) {
            return;
        }

        // Get tag data
        spacing = reinterpret_cast<std::uint8_t *>(tag_data + 0x11);
        old_spacing = *spacing;

        dimensions = maybe_dimensions;
        std::copy(dimensions, dimensions + 2, old_dimensions);

        div = dimensions[0] / 128;
        new_spacing = *spacing / div;
    }

    static void set_can_jason_jones() noexcept {
        if(!spacing) {
            return;
        }

        can_jason_jones = true;
        *spacing = new_spacing;
    }

    static void unset_can_jason_jones() noexcept {
        if(can_jason_jones) {
            can_jason_jones = false;
            *spacing = old_spacing;
        }
    }

    static void jason_jones_the_numbers() noexcept;
    static void unjason_jones_the_numbers() noexcept;
    void set_up_nav_numbers_fix() noexcept {
        static Hook hook_h, hook_w, hook_f;
        write_jmp_call(get_chimera().get_signature("fix_counters_h_sig").data(), hook_h, reinterpret_cast<const void *>(jason_jones_the_numbers), reinterpret_cast<const void *>(unjason_jones_the_numbers));
        write_jmp_call(get_chimera().get_signature("fix_counters_w_sig").data(), hook_w, reinterpret_cast<const void *>(jason_jones_the_numbers), reinterpret_cast<const void *>(unjason_jones_the_numbers));
        write_jmp_call(get_chimera().get_signature("fix_counters_sig").data(), hook_f, reinterpret_cast<const void *>(set_can_jason_jones), reinterpret_cast<const void *>(unset_can_jason_jones));

        static Hook timer_before, timer_after;
        write_jmp_call(get_chimera().get_signature("fix_counters_timer_begin_sig").data(), timer_before, reinterpret_cast<const void *>(set_can_jason_jones));
        write_jmp_call(get_chimera().get_signature("fix_counters_timer_end_sig").data() + 4, timer_after, reinterpret_cast<const void *>(unset_can_jason_jones));

        add_map_load_event(on_map_load);
    }
    void undo_nav_numbers_fix() noexcept {
        get_chimera().get_signature("fix_counters_sig").rollback();
        remove_map_load_event(on_map_load);
    }

    static void jason_jones_the_numbers() noexcept {
        // If we can jason jones it, do so
        if(dimensions && can_jason_jones) {
            dimensions[0] /= div;
            dimensions[1] /= div;
            jason_jones = true;
        }
    }

    static void unjason_jones_the_numbers() noexcept {
        if(!jason_jones) {
            return;
        }
        jason_jones = false;

        std::copy(old_dimensions, old_dimensions + 2, dimensions);
    }
}
