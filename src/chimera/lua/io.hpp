// SPDX-License-Identifier: GPL-3.0-only

#ifndef LUA__IO__HPP
#define LUA__IO__HPP

struct lua_State;

namespace Chimera {
    /**
     * Set whether or not to enable the sandbox
     * @param sandbox set the sandbox value
     * @return        old sandbox value
     */
    bool set_sandbox(bool sandbox) noexcept;

    /**
     * Set up IO functions for the Lua state
     * @param state state to set up functions for
     * @param api   API to target
     */
    void set_up_io_functions(lua_State *state, unsigned int api) noexcept;
}

#endif
