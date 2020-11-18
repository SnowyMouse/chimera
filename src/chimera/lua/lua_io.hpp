// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LUA_IO_HPP
#define CHIMERA_LUA_IO_HPP

#include "lua_script.hpp"

namespace Chimera {
    /**
     * Set up I/O functions into state
    */
    void set_io_functions(lua_State *state) noexcept;
}

#endif
