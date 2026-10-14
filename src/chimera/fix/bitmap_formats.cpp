// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "bitmap_formats.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/tag.hpp"
#include "../event/map_load.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        void check_for_invalid_bitmap_format_asm() noexcept;
    }

    TagID sweeper_tag = TagID::null_id();

    extern "C" void check_for_invalid_bitmap_format(BitmapData *bitmap) noexcept {
        // Convert invalid formats
        // Intel d3d9 drivers hate monochrome. Convert all monochrome bitmaps to ARGB
        if(get_chimera().get_ini()->get_value_bool("debug.convert_monochrome_to_argb").value_or(false)) {
            if((bitmap->format >= BITMAP_DATA_FORMAT_A8 && bitmap->format <= BITMAP_DATA_FORMAT_A8Y8) || bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) {
                void *new_bitmap = bitmap_covert_format(bitmap, true);
                GlobalFree(bitmap->base_address);
                bitmap->base_address = new_bitmap;
                bitmap->format = (bitmap->format == BITMAP_DATA_FORMAT_Y8) ? BITMAP_DATA_FORMAT_X8R8G8B8 : BITMAP_DATA_FORMAT_A8R8G8B8;
            }
        }
        // Converting A8 and AY8 to A8Y8 should be fine most of the time.
        else {
            if(bitmap->format == BITMAP_DATA_FORMAT_A8 || bitmap->format == BITMAP_DATA_FORMAT_AY8 || bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) {
                void *new_bitmap = bitmap_covert_format(bitmap, false);
                GlobalFree(bitmap->base_address);
                bitmap->base_address = new_bitmap;
                bitmap->format = (bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) ? BITMAP_DATA_FORMAT_A8R8G8B8 : BITMAP_DATA_FORMAT_A8Y8;
            }

            // Hack because wrappers, monochrome and D3DSAMP_BORDERCOLOR don't get along.
            if(bitmap->tag_id == sweeper_tag) {
                if(bitmap->format == BITMAP_DATA_FORMAT_A8Y8) {
                    void *new_bitmap = bitmap_covert_format(bitmap, true);
                    GlobalFree(bitmap->base_address);
                    bitmap->base_address = new_bitmap;
                    bitmap->format = BITMAP_DATA_FORMAT_A8R8G8B8;
                }
            }
        }
    }

    void find_sweeper_tag() noexcept {
        auto *globals_tag = get_tag("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        auto *interface_bitmaps = *reinterpret_cast<std::byte **>(globals_tag->data + 0x140 + 0x4);
        sweeper_tag = *reinterpret_cast<TagID *>(interface_bitmaps + 0x70 + 0xC);
    }

    void set_up_bitmap_formats() noexcept {
        // Support Y8 and A8Y8. These are supported natively by d3d9.
        auto *y8 = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("supported_bitmap_formats").data() + 8) + 1;
        auto *a8y8 = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("supported_bitmap_formats").data() + 8) + 3;

        overwrite(y8, D3DFMT_L8);
        overwrite(a8y8, D3DFMT_A8L8);

        // Translate A8, AY8 or P8 textures to something that is supported by d3d9.
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("d3d_create_texture_sig").data() + 4, hook, reinterpret_cast<const void *>(check_for_invalid_bitmap_format_asm), nullptr);
        add_map_load_event(find_sweeper_tag);
    }
}
