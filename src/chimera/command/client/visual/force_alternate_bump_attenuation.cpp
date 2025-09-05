// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/alternate_bump_attenuation.hpp"

namespace Chimera {
    bool force_alternate_bump_attenuation_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(!new_enabled) {
                    disable_forced_alternate_bump_attenuation();
                }
                else {
                    enable_forced_alternate_bump_attenuation();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
