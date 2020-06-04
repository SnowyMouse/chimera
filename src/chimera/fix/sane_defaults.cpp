// SPDX-License-Identifier: GPL-3.0-only

#include "sane_defaults.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    extern "C" void set_sane_defaults_asm() noexcept;

    void set_up_sane_defaults() noexcept {
        auto &chimera = get_chimera();
        if(!chimera.feature_present("client_fp_reverb")) {
            return;
        }

        auto *default_settings_sig = chimera.get_signature("default_settings_sig").data();
        static Hook hook;
        write_jmp_call(reinterpret_cast<std::byte *>(default_settings_sig + 7), hook, reinterpret_cast<const void *>(set_sane_defaults_asm), nullptr,  false);
    }
}
