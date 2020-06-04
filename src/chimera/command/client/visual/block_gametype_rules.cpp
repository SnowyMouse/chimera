// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_gametype_rules_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &gametype_rules_sig = get_chimera().get_signature("gametype_rules_sig");
                if(new_enabled) {
                    overwrite(gametype_rules_sig.data() + 6, static_cast<std::uint32_t>(0));
                }
                else {
                    gametype_rules_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
