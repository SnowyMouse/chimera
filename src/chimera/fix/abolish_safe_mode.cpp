// SPDX-License-Identifier: GPL-3.0-only

#include "abolish_safe_mode.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_up_abolish_safe_mode() noexcept {
        auto *safe_mode = get_chimera().get_signature("auto_save_mode_sig").data();
        const SigByte fuck_safe_mode[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(safe_mode, fuck_safe_mode);

        // Nuke UnsupportedCard and UMA config.txt settings
        struct ConfigEntry {
            char *config;
            std::byte *function_ptr;
        };

        const SigByte nuke_bs[] = { 0xC3 };
        auto *configs = *reinterpret_cast<ConfigEntry **>(get_chimera().get_signature("config_txt_function_ptr_sig").data() + 3);
        write_code_s(configs[7].function_ptr, nuke_bs);
        write_code_s(configs[21].function_ptr, nuke_bs);
    }
}
