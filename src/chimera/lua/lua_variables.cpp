// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../halo_data/map.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/table.hpp"
#include "lua_variables.hpp"

namespace Chimera {
    void refresh_client_index(lua_State *state) noexcept {
        auto player_id = get_client_player_id();
        if(!player_id.is_null()) {
            lua_pushinteger(state, player_id.index.index);
        }
        else {
            lua_pushnil(state);
        }
        lua_setglobal(state, "local_player_index");
    }

    void refresh_variables(lua_State *state) noexcept {
        // Update client player index
        refresh_client_index(state);

        // Update map name
        lua_pushstring(state, get_map_header().name);
        lua_setglobal(state, "map");

        // map is protected?
        lua_pushboolean(state, map_is_protected());
        lua_setglobal(state, "map_is_protected");

        // Update server type
        const char *server = nullptr;
        switch(server_type()) {
            case SERVER_NONE:
                server = "none";
                break;
            case SERVER_DEDICATED:
                server = "dedicated";
                break;
            case SERVER_LOCAL:
                server = "local";
                break;
        }
        lua_pushstring(state, server);
        lua_setglobal(state, "server_type");

        // Update gametype
        if(std::strcmp(server, "local") != 0) {
            const char *current_gametype = nullptr;
            switch(gametype()) {
                case GAMETYPE_CTF:
                    current_gametype = "ctf";
                    break;
                case GAMETYPE_SLAYER:
                    current_gametype = "slayer";
                    break;
                case GAMETYPE_KING:
                    current_gametype = "king";
                    break;
                case GAMETYPE_ODDBALL:
                    current_gametype = "oddball";
                    break;
                case GAMETYPE_RACE:
                    current_gametype = "race";
                    break;
            }
            lua_pushstring(state, current_gametype);
        }
        else {
            lua_pushnil(state);
        }
        lua_setglobal(state, "gametype");
    }
}