// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/custom_edition_bridge/netcode.hpp"

namespace Chimera {
    bool custom_edition_netcode_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                active = new_value;
                if(active) {
                    enable_custom_edition_netcode_support();
                }
                else {
                    disable_custom_edition_netcode_support();
                }
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
