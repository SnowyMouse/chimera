// SPDX-License-Identifier: GPL-3.0-only

#ifndef LUA__INSTANCE__HPP
#define LUA__INSTANCE__HPP

struct lua_State;
#include <cstddef>
#include <string>
#include <exception>

namespace Chimera {
    class LuaEventHandler;

    class LuaScriptInstance {
    public:
        /**
         * Exception thrown when loading has failed
         */
        class LuaLoadException : public std::exception {
            friend class LuaScriptInstance;
        public:
            const char *what() const noexcept override;
        private:
            LuaLoadException(lua_State *state) noexcept;
            std::string error;
        };

        /**
         * Get the script name
         * @return script name
         */
        const char *get_name() noexcept { return this->name.c_str(); }

        /**
         * Instantiate a Lua script instance
         * @param lua_data  pointer to the lua script
         * @param lua_size  length of the lua script
         * @param sandboxed disable certain features in the Lua script
         * @throws          LuaScriptInstance if failed
         */
        LuaScriptInstance(const char *lua_data, std::size_t lua_size, bool sandboxed);
        LuaScriptInstance(LuaScriptInstance &&move) noexcept;
    private:
        /** Lua state */
        lua_State *state = nullptr;

        /** Script name (no extension) */
        std::string name;
    };
}

#endif
