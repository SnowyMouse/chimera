// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../halo_data/map.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static void print_object_info(ObjectID object_id, const char *name, int depth);
    #define OUTPUT_WITH_COLOR(...) console_output(body_color, __VA_ARGS__)

    bool player_info_command(int argc, const char **argv) {
        // Get the player
        auto &player_table = PlayerTable::get_player_table();
        Player *player;
        if(argc) {
            int index;
            try {
                index = std::stoul(*argv) - 1;
            }
            catch(std::exception &) {
                console_error(localize("chimera_error_takes_player_number"));
                return false;
            }
            player = player_table.get_player_by_rcon_id(index);
        }
        else {
            player = player_table.get_client_player();
        }

        // If the player does not exist show an error
        if(!player) {
            console_error(localize("chimera_error_player_not_found"), argc ? *argv : nullptr);
            return false;
        }

        // Show info
        extern const char *output_prefix;
        auto *old_prefix = output_prefix;
        output_prefix = nullptr;

        if(*player->name) {
            console_output(ConsoleColor::header_color(), "Info for %S", player->name);
        }
        else {
            console_output(ConsoleColor::header_color(), "Info for Local Player");
        }

        // Show some player things
        auto body_color = ConsoleColor::body_color();
        OUTPUT_WITH_COLOR("Player ID: %08X (Address: 0x%08X)", player->get_full_id().whole_id, reinterpret_cast<std::uintptr_t>(player));
        if(server_type() != ServerType::SERVER_NONE) {
            OUTPUT_WITH_COLOR("Server Ping: %i ms", player->ping);
            OUTPUT_WITH_COLOR("Server Stats: %i K / %i A / %i D -- %.02f K/D", player->kills, player->assists, player->deaths, player->deaths == 0 ? player->kills : static_cast<float>(player->kills) / player->deaths);
        }

        // See if we can get the player's object
        print_object_info(player->object_id, "Player", 2);

        output_prefix = old_prefix;
        return true;
    }

    static void print_object_info(ObjectID object_id, const char *name, int depth) {
        depth--;
        if(depth < 0) {
            return;
        }

        auto body_color = ConsoleColor::body_color();
        auto *object = ObjectTable::get_object_table().get_dynamic_object(object_id);
        if(!object) {
            return;
        }

        OUTPUT_WITH_COLOR("    %s Object ID: %08X (Address: 0x%08X)", name, object_id.whole_id, reinterpret_cast<std::uintptr_t>(object));
        auto *tag = get_tag(object->tag_id);
        if(tag) {
            OUTPUT_WITH_COLOR("    %s Object Tag ID: %08X (Path: %s)", name, object->tag_id, map_is_protected() ? localize("chimera_tag_map_is_protected") : tag->path);
        }
        if(object->base_shield) {
            OUTPUT_WITH_COLOR("    %s Object Shield: %.02f%% (%.02f / %.02f hitpoints)", name, object->shield * 100.0F, object->base_shield * object->shield, object->base_shield);
        }
        if(object->base_health) {
            OUTPUT_WITH_COLOR("    %s Object Health: %.02f%% (%.02f / %.02f hitpoints)", name, object->health * 100.0F, object->base_health * object->health, object->base_health);
        }
        OUTPUT_WITH_COLOR("    %s Object Coords (Rel.): %.05f, %.05f, %.05f", name, object->position.x, object->position.y, object->position.z);
        OUTPUT_WITH_COLOR("    %s Object Coords (Script): %.05f, %.05f, %.05f", name, object->center_position.x, object->center_position.y, object->center_position.z);

        print_object_info(object->parent, "Parent", depth);
        print_object_info(object->weapon, "Weapon", depth);
    }
}
