// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../fix/custom_edition_bridge/map_support.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../fix/custom_edition_bridge/netcode.hpp"

namespace Chimera {
    bool custom_edition_netcode_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                if(custom_edition_maps_supported_on_retail()) {
                    active = new_value;
                    if(active) {
                        enable_custom_edition_netcode_support();
                    }
                    else {
                        disable_custom_edition_netcode_support();
                    }
                }
                else {
                    console_error(localize("custom_edition_netcode_command_error_needs_custom_edition_map_support"));
                    return false;
                }
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
