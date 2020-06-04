// SPDX-License-Identifier: GPL-3.0-only

#include "../../../halo_data/map.hpp"

namespace Chimera {
    bool load_ui_map_command(int, const char **) {
        load_ui_map();
        return true;
    }
}
