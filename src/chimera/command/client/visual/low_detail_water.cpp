// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/water_fix.hpp"

namespace Chimera {
    extern bool apply_water_mipmap_hack;

    bool low_detaiL_water_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(!new_enabled) {
                    apply_water_mipmap_hack = false;
                }
                else {
                    apply_water_mipmap_hack = true;
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
