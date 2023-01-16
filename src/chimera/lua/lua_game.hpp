// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LUA_GAME_HPP
#define CHIMERA_LUA_GAME_HPP

#include "lua_script.hpp"

namespace Chimera {
    /**
     * Set up game functions into Lua state
     */
    void set_game_functions(lua_State *state) noexcept;
}

#endif