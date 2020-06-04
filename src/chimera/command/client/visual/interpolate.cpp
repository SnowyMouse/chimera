// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/interpolate/interpolate.hpp"

namespace Chimera {
    bool interpolate_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    set_up_interpolation();
                }
                else {
                    disable_interpolation();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
