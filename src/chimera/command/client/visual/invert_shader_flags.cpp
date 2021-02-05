// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/inverted_flag.hpp"

namespace Chimera {
    void enable_easter_eggs();
    void disable_easter_eggs();
    
    bool invert_shader_flags_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(!new_enabled) {
                    set_up_inverted_flag_fix();
                    enable_easter_eggs();
                }
                else {
                    unset_up_inverted_flag_fix();
                    disable_easter_eggs();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
