// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "bitmap_formats.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_up_bitmap_formats() noexcept {
        auto *y8 = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("supported_bitmap_formats").data() + 8) + 1;
        auto *a8y8 = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("supported_bitmap_formats").data() + 8) + 3;

        overwrite(y8, D3DFMT_L8);
        overwrite(a8y8, D3DFMT_A8L8);
    }
}
