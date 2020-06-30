// SPDX-License-Identifier: GPL-3.0-only

#include "floor_decal_memery.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"

namespace Chimera {
    extern "C" {
        void set_decal_memery_asm() noexcept;
        void fix_decal_memery_asm() noexcept;
    }

    void set_up_floor_decals_fix() noexcept {
        // Basically, we need to get whether or not we're rendering an alpha tested double multiplied decal. If so, meme Halo into rendering it anyway
        auto *ptr_1 = get_chimera().get_signature("floor_decal_meme_1_sig").data() + 4;
        static Hook hook_1;
        write_jmp_call(ptr_1, hook_1, nullptr, reinterpret_cast<const void *>(set_decal_memery_asm), false);

        auto *ptr_2 = get_chimera().get_signature("floor_decal_meme_2_sig").data() + 7;
        static Hook hook_2;
        write_jmp_call(ptr_2, hook_2, nullptr, reinterpret_cast<const void *>(fix_decal_memery_asm), false);
    }
}
