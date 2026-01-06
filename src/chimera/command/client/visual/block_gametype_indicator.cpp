// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/gametype_indicator_memes.hpp"

namespace Chimera {
    bool block_gametype_indicator_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {

                if(new_enabled) {
                    gametype_indicator_disabled = true;
                }
                else {
                    gametype_indicator_disabled = false;
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
