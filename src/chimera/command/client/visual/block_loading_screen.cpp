// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_loading_screen_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &loading_screen_singleplayer_sig = get_chimera().get_signature("loading_screen_singleplayer_sig");
                auto &loading_screen_host_sig = get_chimera().get_signature("loading_screen_host_sig");
                auto &loading_screen_join_sig = get_chimera().get_signature("loading_screen_join_sig");
                if(new_enabled) {
                    overwrite(loading_screen_singleplayer_sig.data() + 6, static_cast<std::uint32_t>(0));
                    overwrite(loading_screen_host_sig.data() + 6, static_cast<std::uint32_t>(0));
                    overwrite(loading_screen_join_sig.data() + 6, static_cast<std::uint32_t>(0));
                }
                else {
                    loading_screen_singleplayer_sig.rollback();
                    loading_screen_host_sig.rollback();
                    loading_screen_join_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
