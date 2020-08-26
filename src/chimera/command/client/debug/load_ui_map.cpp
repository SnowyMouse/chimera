// SPDX-License-Identifier: GPL-3.0-only

#include "../../../halo_data/map.hpp"
#include "../../../halo_data/menu.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool load_ui_map_command(int, const char **) {
        if(server_type() == SERVER_NONE) {
            load_ui_map();
            load_main_menu();
        }
        else {
            console_output("You should use the \"disconnect\" command");
        }
        return true;
    }
}
