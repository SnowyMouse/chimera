// SPDX-License-Identifier: GPL-3.0-only

#include "instance.hpp"
#include <lauxlib.h>
#include <lualib.h>

#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "io.hpp"

namespace Chimera {
    LuaScriptInstance::LuaLoadException::LuaLoadException(lua_State *state) noexcept : error(lua_tostring(state, -1)) {
        lua_pop(state, 1);
        lua_close(state);
    }

    LuaScriptInstance::LuaScriptInstance(const char *name, const char *lua_data, std::size_t lua_size, bool sandboxed) : name(name) {
        // Figure out the API version
        char ini_key[256];
        std::snprintf(ini_key, sizeof(ini_key), "lua.%s.backwards_compatibility", name);
        this->api = static_cast<unsigned int>(get_chimera().get_ini()->get_value_long(ini_key).value_or(3));

        std::snprintf(ini_key, sizeof(ini_key), "lua.%s.sandbox", name);
        this->sandboxed = static_cast<unsigned int>(get_chimera().get_ini()->get_value_bool(ini_key).value_or(sandboxed));

        // Set up the new instance
        auto *instance = luaL_newstate();
        luaL_openlibs(instance);

        // Sandbox if needed
        if(this->sandboxed) {
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
            if(luaL_loadbuffer(instance, sandbox, sizeof(sandbox), "chimera-internal-sandbox") != LUA_OK || lua_pcall(instance, 0, 0, 0) != LUA_OK) {
                throw LuaLoadException(instance);
            }
        }

        // Note whether we're sandboxed
        lua_pushboolean(instance, this->sandboxed);
        lua_setglobal(instance, "sandboxed");

        // Set up these functions
        set_up_io_functions(instance, this->api);

        // Execute stuff!
        bool old_sandbox = set_sandbox(this->sandboxed);
        bool error_load = luaL_loadbuffer(instance, lua_data, lua_size, name) != LUA_OK || lua_pcall(instance, 0, 0, 0) != LUA_OK;
        set_sandbox(old_sandbox);
        if(error_load) {
            throw LuaLoadException(instance);
        }

        // Since it's all set up, let's go
        this->state = instance;
    }
    LuaScriptInstance::LuaScriptInstance(LuaScriptInstance &&move) noexcept {
        this->state = move.state;
        move.state = nullptr;
    }
    LuaScriptInstance::~LuaScriptInstance() noexcept {
        if(this->state) {
            lua_close(this->state);
            this->state = nullptr;
        }
    }
}
