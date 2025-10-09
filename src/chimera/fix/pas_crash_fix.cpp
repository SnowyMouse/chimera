// SPDX-License-Identifier: GPL-3.0-only

#include "pas_crash_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        void skip_accessing_sound_pas_data_asm() noexcept;
        const void *original_cluster_sound_distances_inst = nullptr;
    }

    void set_up_sound_pas_crash_fix() noexcept {
        auto *ptr = get_chimera().get_signature("cluster_sound_distance_func_sig").data() + 1;
        static Hook hook;
        write_function_override(ptr, hook, reinterpret_cast<const void *>(skip_accessing_sound_pas_data_asm), &original_cluster_sound_distances_inst);
    }
}
