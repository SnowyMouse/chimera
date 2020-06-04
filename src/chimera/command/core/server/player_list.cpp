// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool player_list_command(int, const char **) {
        auto &player_table = PlayerTable::get_player_table();
        bool player_present = false;

        extern const char *output_prefix;
        auto *old_prefix = output_prefix;
        output_prefix = nullptr;

        for(std::size_t i = 0; i < 256; i++) {
            auto *player = player_table.get_player_by_rcon_id(i);
            if(player) {
                player_present = true;
                char team[256] = {};
                ConsoleColor color = { 1.0F, 0.5F, 0.8F, 0.6F };
                if(is_team()) {
                    switch(player->team) {
                        case 0:
                            std::snprintf(team, sizeof(team), " (%s)", localize("chimera_color_red"));
                            color.r = 1.0F;
                            color.g = 0.4F;
                            color.b = 0.4F;
                            break;
                        case 1:
                            std::snprintf(team, sizeof(team), " (%s)", localize("chimera_color_blue"));
                            color.r = 0.4F;
                            color.g = 0.4F;
                            color.b = 1.0F;
                            break;
                        default:
                            unsigned int team_int = player->team;
                            std::strncpy(team, " (%u)", team_int);
                            color.r = 0.4F;
                            color.g = 1.0F;
                            color.b = 0.4F;
                            break;
                    }
                }
                console_output(color, "%zu. %S%s", i + 1, player->name, team);
            }
        }
        output_prefix = old_prefix;
        if(!player_present) {
            console_output(localize("chimera_player_list_command_none_found"));
        }
        return true;
    }
}
