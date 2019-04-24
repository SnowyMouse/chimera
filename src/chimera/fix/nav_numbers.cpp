#include "nav_numbers.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"

namespace Chimera {
    static void jason_jones_the_numbers() noexcept;
    static void unjason_jones_the_numbers() noexcept;
    void set_up_nav_numbers_fix() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("fix_counters_sig").data(), hook, reinterpret_cast<const void *>(jason_jones_the_numbers), reinterpret_cast<const void *>(unjason_jones_the_numbers));
    }
    void undo_nav_numbers_fix() noexcept {
        get_chimera().get_signature("fix_counters_sig").rollback();
    }

    static bool jason_jones = false;

    static std::uint8_t *spacing;
    static std::uint8_t old_spacing_value;

    static std::uint16_t *dimensions;
    static std::uint16_t old_dimensions[2];

    static void jason_jones_the_numbers() noexcept {
        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);

        // Get the hudg tag
        auto *tag = get_tag(*reinterpret_cast<TagID *>(interface_bitmaps + 0xB0 + 0xC));
        if(!tag) {
            return;
        }
        auto *tag_data = tag->data;

        // Get tag data
        spacing = reinterpret_cast<std::uint8_t *>(tag_data + 0x11);
        old_spacing_value = *spacing;

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

        dimensions = maybe_dimensions;
        std::copy(dimensions, dimensions + 2, old_dimensions);

        std::int32_t div = maybe_dimensions[0] / 128;
        dimensions[0] /= div;
        dimensions[1] /= div;
        *spacing /= div;

        jason_jones = true;
    }

    static void unjason_jones_the_numbers() noexcept {
        if(!jason_jones) {
            return;
        }
        jason_jones = false;

        *spacing = old_spacing_value;
        std::copy(old_dimensions, old_dimensions + 2, dimensions);
    }
}
