// SPDX-License-Identifier: GPL-3.0-only

#include "../../../config/config.hpp"
#include "../../../chimera.hpp"

namespace Chimera {
    bool Chimera::block_all_bullshit_command(int, const char **) noexcept {
        get_chimera().p_config->set_saving(false);
        get_chimera().execute_command("chimera_block_buffering 1", nullptr, true);
        get_chimera().execute_command("chimera_block_gametype_indicator 1", nullptr, true);
        get_chimera().execute_command("chimera_block_gametype_rules 1", nullptr, true);
        get_chimera().execute_command("chimera_block_hold_f1 1", nullptr, true);
        get_chimera().execute_command("chimera_block_loading_screen 1", nullptr, true);
        get_chimera().execute_command("chimera_block_zoom_blur 1", nullptr, true);
        get_chimera().execute_command("chimera_block_mouse_acceleration 1", nullptr, true);
        get_chimera().p_config->set_saving(true);
        get_chimera().p_config->save();
        return true;
    }
}
