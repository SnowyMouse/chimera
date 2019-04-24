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

    ServerInfoPlayer *ServerInfo::get_player(PlayerID player) noexcept {
        for(auto &p : this->players) {
            if(p.player_id == player.index.index) {
                return &p;
            }
        }
        return nullptr;
    }

    ServerInfo *ServerInfo::get_server_info() noexcept {
        #define RETURN_TABLE_FOR_SIGNATURE(SIGNATURE) { \
            static ServerInfo *table = nullptr; \
            if(!table) { \
                table = reinterpret_cast<ServerInfo *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature(SIGNATURE).data() + 1)); \
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
