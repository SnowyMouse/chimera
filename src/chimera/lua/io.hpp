// SPDX-License-Identifier: GPL-3.0-only

#ifndef LUA__IO__HPP
#define LUA__IO__HPP

struct lua_State;

namespace Chimera {
    void set_up_io_functions(lua_State *state, unsigned int api_version) noexcept;
}

#endif
