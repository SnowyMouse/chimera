// SPDX-License-Identifier: GPL-3.0-only

#include "player.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "server.hpp"

namespace Chimera {
    PlayerID Player::get_full_id() const noexcept {
        PlayerID player_id;
        player_id.index.id = this->player_id;
        player_id.index.index = static_cast<std::uint16_t>(this - PlayerTable::get_player_table().first_element);
        return player_id;
    }

    PlayerID get_client_player_id() noexcept {
        static PlayerID *player_id = reinterpret_cast<PlayerID *>(**reinterpret_cast<std::byte ***>(get_chimera().get_signature("player_id_sig").data() + 2) + 4);
        return *player_id;
    }

    Player *PlayerTable::get_player(PlayerID player_id) noexcept {
        if(player_id.is_null()) {
            return nullptr;
        }
        if(player_id.index.index >= this->current_size) {
            return nullptr;
        }
        auto &player = this->first_element[player_id.index.index];
        if(player.player_id != player_id.index.id) {
            return nullptr;
        }
        return &player;
    }

    Player *PlayerTable::get_player_by_rcon_id(std::size_t rcon_id) noexcept {
        auto *server_info = ServerInfoPlayerList::get_server_info_player_list();
        if(!server_info) {
            return nullptr;
        }

        // Make sure we have enough thingies
        if(rcon_id < sizeof(server_info->players) / sizeof(server_info->players[rcon_id])) {
            auto *player = server_info->players[rcon_id].get_player_table_player();
            if(player && player->player_id == 0xFFFF) {
                return nullptr;
            }
            return player;
        }
        else {
            return nullptr;
        }
    }

    Player *PlayerTable::get_client_player() noexcept {
        return this->get_player(get_client_player_id());
    }

    PlayerTable &PlayerTable::get_player_table() noexcept {
        static PlayerTable *table = nullptr;
        if(!table) {
            table = *reinterpret_cast<PlayerTable **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("player_table_sig").data() + 1));
        }
        return *table;
    }
}
