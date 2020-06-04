// SPDX-License-Identifier: GPL-3.0-only

#include "server.hpp"
#include "multiplayer.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "player.hpp"

namespace Chimera {
    Player *ServerInfoPlayer::get_player_table_player() const noexcept {
        auto &player_table = PlayerTable::get_player_table();
        if(this->player_id >= player_table.current_size) {
            return nullptr;
        }
        return player_table.first_element + this->player_id;
    }

    ServerInfoPlayer *ServerInfoPlayerList::get_player(PlayerID player) noexcept {
        for(auto &p : this->players) {
            if(p.player_id == player.index.index) {
                return &p;
            }
        }
        return nullptr;
    }

    ServerInfoPlayerList *ServerInfoPlayerList::get_server_info_player_list() noexcept {
        auto *info = ServerInfo::get_server_info();
        if(!info) {
            return nullptr;
        }

        static std::uint32_t offset = *reinterpret_cast<std::uint32_t *>(get_chimera().get_signature("server_info_player_list_offset_sig").data() + 4) - 1;
        return reinterpret_cast<ServerInfoPlayerList *>(reinterpret_cast<std::byte *>(info) + offset);
    }

    ServerInfo *ServerInfo::get_server_info() noexcept {
        #define RETURN_TABLE_FOR_SIGNATURE(SIGNATURE) { \
            static ServerInfo *table = nullptr; \
            if(!table) { \
                table = reinterpret_cast<ServerInfo *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature(SIGNATURE).data() + 1) - 8); \
            } \
            return table; \
        }

        switch(server_type()) {
            // In a server (hosting)
            case ServerType::SERVER_LOCAL:
                RETURN_TABLE_FOR_SIGNATURE("server_info_host_sig")

            // In a server (as client)
            case ServerType::SERVER_DEDICATED:
                RETURN_TABLE_FOR_SIGNATURE("server_info_client_sig")

            // Not in a server
            default:
                return nullptr;
        }
    }
}
