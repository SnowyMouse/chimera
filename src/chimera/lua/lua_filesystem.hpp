// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LUA_FS_HPP
#define CHIMERA_LUA_FS_HPP

#include <lua.hpp>

namespace Chimera {
    /**
     * Set up filesystem functions into Lua state
     */
    void set_fs_functions(lua_State *state) noexcept;
}

#endif