// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "bitmap_formats.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        void check_for_invalid_bitmap_format_asm() noexcept;
    }

    extern "C" void check_for_invalid_bitmap_format(BitmapData *bitmap) noexcept {
        // Convert invalid formats
        if(bitmap->format == BITMAP_DATA_FORMAT_A8 || bitmap->format == BITMAP_DATA_FORMAT_AY8 || bitmap->format == BITMAP_DATA_FORMAT_P8_BUMP) {
            void *new_bitmap = bitmap_covert_format(bitmap);
            GlobalFree(bitmap->base_address);
            bitmap->base_address = new_bitmap;

            // Tell the game to treat this bitmap data block as either A8Y8 or A8R8G8B8.
            switch(bitmap->format) {
                case BITMAP_DATA_FORMAT_A8:
                case BITMAP_DATA_FORMAT_AY8:
                    bitmap->format = BITMAP_DATA_FORMAT_A8Y8;
                    break;

                case BITMAP_DATA_FORMAT_P8_BUMP:
                    bitmap->format = BITMAP_DATA_FORMAT_A8R8G8B8;
                    break;

                default:
                    break;
            }
        }
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
    }
}
