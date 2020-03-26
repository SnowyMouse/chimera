#include "../../../localization/localization.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool teleport_command(int argc, const char **argv) {
        // Prevent desyncs if needed
        if(server_type() == ServerType::SERVER_DEDICATED) {
            console_error(localize("chimera_error_must_be_host"));
            return false;
        }

        float x, y, z;
        auto &player_table = PlayerTable::get_player_table();
        auto &object_table = ObjectTable::get_object_table();
        try {
            if(argc == 1) {
                // Get the player
                Player *player;
                int index;
                try {
                    index = std::stoul(*argv) - 1;
                }
                catch(std::exception &) {
                    console_error(localize("chimera_error_takes_player_number"));
                    return false;
                }
                player = player_table.get_player_by_rcon_id(index);

                // If the player does not exist show an error
                if(!player) {
                    console_error(localize("chimera_error_player_not_found"), argc ? *argv : nullptr);
                    return false;
                }

                // Is the player alive?
                auto *object = object_table.get_dynamic_object(player->object_id);
                if(!object) {
                    console_error(localize("chimera_teleport_dead"));
                    return false;
                }

                x = object->center_position.x;
                y = object->center_position.y;
                z = object->center_position.z;
            }
            else if(argc == 3) {
                x = std::stof(argv[0]);
                y = std::stof(argv[1]);
                z = std::stof(argv[2]);
            }
            else {
                console_error(localize("chimera_teleport_invalid_arguments"));
                return false;
            }
        }
        catch(std::exception &) {
            console_error(localize("chimera_teleport_invalid_arguments"));
            return false;
        }

        // Get the player
        Player *local_player = player_table.get_client_player();
        if(!local_player) {
            console_error(localize("chimera_teleport_dead_self"));
            return false;
        }

        // Is the player alive?
        auto *player_object = object_table.get_dynamic_object(local_player->object_id);
        if(!player_object) {
            console_error(localize("chimera_teleport_dead_self"));
            return false;
        }

        // Get the parent
        while(true) {
            auto *parent_object = object_table.get_dynamic_object(player_object->parent);
            if(parent_object) {
                player_object = parent_object;
            }
            else {
                break;
            }
        }

        player_object->position.x = x;
        player_object->position.y = y;
        player_object->position.z = z;

        return true;
    }
}
