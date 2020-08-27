// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../event/damage.hpp"
#include "../../../event/tick.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../halo_data/tag.hpp"

namespace Chimera {
    static bool negate_damage(ObjectID &, TagID &, float &, PlayerID &, ObjectID &) {
        return false;
    }

    static void disable_negate_damage_if_not_host() {
        if(server_type() == ServerType::SERVER_DEDICATED) {
            get_chimera().execute_command("chimera_block_damage false");
        }
    }

    bool block_damage_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    if(server_type() == ServerType::SERVER_DEDICATED) {
                        console_error(localize("chimera_error_must_be_host"));
                        return false;
                    }
                    add_damage_event(negate_damage);
                    add_pretick_event(disable_negate_damage_if_not_host);
                }
                else {
                    remove_damage_event(negate_damage);
                    remove_pretick_event(disable_negate_damage_if_not_host);
                }
            }
            enabled = new_enabled;
        }

        console_output(enabled ? "on" : "off");

        return true;
    }
}
