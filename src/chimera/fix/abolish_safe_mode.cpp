// SPDX-License-Identifier: GPL-3.0-only

#include "abolish_safe_mode.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/config.hpp"


namespace Chimera {
    extern "C" {
        void set_memory_limit_for_demo_asm() noexcept;
    }

    void set_up_abolish_safe_mode() noexcept {
        auto *safe_mode = get_chimera().get_signature("auto_save_mode_sig").data();
        const SigByte fuck_safe_mode[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(safe_mode, fuck_safe_mode);

        // Nuke UnsupportedCard and UMA config.txt options
        const SigByte nuke_bs[] = { 0xC3 };
        auto *configs = *reinterpret_cast<ConfigEntry **>(get_chimera().get_signature("config_txt_function_ptr_sig").data() + 3);
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            write_code_s(configs[CONFIG_DEMO_UNSUPPORTED_CARD].function_ptr, nuke_bs);
            write_code_s(configs[CONFIG_DEMO_UMA].function_ptr, nuke_bs);

            //Stop demo failing to work out how much memory is available.
            static Hook hook;
            write_jmp_call(reinterpret_cast<std::byte *>(0x5782ff), hook, reinterpret_cast<const void *>(set_memory_limit_for_demo_asm), nullptr, false);

        }
        else {
            write_code_s(configs[CONFIG_UNSUPPORTED_CARD].function_ptr, nuke_bs);
            write_code_s(configs[CONFIG_UMA].function_ptr, nuke_bs);
        }
    }
}
