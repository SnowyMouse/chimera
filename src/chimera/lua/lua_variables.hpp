// SPDX-License-Identifier: GPL-3.0-only

#include <lua.hpp>

namespace Chimera {
    /**
     * Refresh client index variable of a given Lua script state
     */
    void refresh_client_index(lua_State *state) noexcept;

    /**
     * Reflesh global variables of a given Lua script state
     */
    void refresh_variables(lua_State *state) noexcept;
}