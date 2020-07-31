// SPDX-License-Identifier: GPL-3.0-only

#include "instance.hpp"
#include <lauxlib.h>
#include <lualib.h>

namespace Chimera {
    LuaScriptInstance::LuaLoadException::LuaLoadException(lua_State *state) noexcept : error(lua_tostring(state, -1)) {
        lua_pop(state, 1);
        lua_close(state);
    }

    LuaScriptInstance::LuaScriptInstance(const char *lua_data, std::size_t lua_size, bool sandboxed) {
        auto *instance = luaL_newstate();
        luaL_openlibs(instance);

        // Sandbox if needed
        if(sandboxed) {
            const char sandbox[] = "io = nil\n"
                                   "dofile = nil\n"
                                   "getfenv = nil\n"
                                   "load = nil\n"
                                   "loadfile = nil\n"
                                   "loadstring = nil\n"
                                   "require = nil\n"
                                   "os.execute = nil\n"
                                   "os.exit = nil\n"
                                   "os.remove = nil\n"
                                   "os.rename = nil\n"
                                   "os.tmpname = nil\n"
                                   "require = nil\n";
            if(luaL_loadbuffer(state, sandbox, sizeof(sandbox), "chimera-internal-sandbox") != LUA_OK || lua_pcall(state, 0, 0, 0) != LUA_OK) {
                throw LuaLoadException(instance);
            }
        }

        // Note whether we're sandboxed
        lua_pushboolean(state, sandboxed);
        lua_setglobal(state, "sandboxed");

        lua_close(instance);
    }
    LuaScriptInstance::LuaScriptInstance(LuaScriptInstance &&move) noexcept {
        this->state = move.state;
        move.state = nullptr;
    }
}
