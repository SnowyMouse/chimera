// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../fix/fp_reverb.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool fp_reverb_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_setting = STR_TO_BOOL(argv[0]);
            if(new_setting) {
                set_up_fp_reverb_fix();
            }
            else {
                disable_fp_reverb_fix();
            }
            enabled = new_setting;
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
