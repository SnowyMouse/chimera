// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LUA_SCRIPT_HPP
#define CHIMERA_LUA_SCRIPT_HPP

#include <string>
#include <vector>
#include <lua.hpp>
#include "../output/output.hpp"
#include "../event/event.hpp"

namespace Chimera {
    #define CHIMERA_LUA_INTERPRETER 2.042

    struct LuaScriptCallback {
        std::string callback_function;
        EventPriority priority;
    };

    struct LuaScript;

    struct LuaAmbiguousTypeArgument {
        enum {
            ARGUMENT_BOOLEAN,
            ARGUMENT_STRING,
            ARGUMENT_NUMBER,
            ARGUMENT_NIL
        } argument_type;
        std::string string_value;
        double number_value;
        bool bool_value;
        static LuaAmbiguousTypeArgument check_argument(LuaScript &script, int arg, bool do_lua_error);
        void push_argument(LuaScript &script) noexcept;
    };

    struct LuaScriptTimer {
        double interval_ms;
        double time_passed = 0;
        std::string function;
        size_t timer_id;
        std::vector<LuaAmbiguousTypeArgument> arguments;
    };

    struct LuaScript {
        lua_State *state = nullptr;

        std::vector<LuaScriptTimer> timers;

        double version = CHIMERA_LUA_INTERPRETER;

        std::string name;
        bool loaded = false;
        bool sandbox;
        bool global;

        LuaScriptCallback c_command;
        LuaScriptCallback c_frame;
        LuaScriptCallback c_preframe;
        LuaScriptCallback c_map_load;
        LuaScriptCallback c_map_preload;
        LuaScriptCallback c_precamera;
        LuaScriptCallback c_rcon_message;
        LuaScriptCallback c_spawn;
        LuaScriptCallback c_prespawn;
        LuaScriptCallback c_tick;
        LuaScriptCallback c_pretick;
        LuaScriptCallback c_unload;

        size_t next_timer_id = 0;

        LuaScript(lua_State *state, const char *name, const bool &global, const bool &unlocked) noexcept;
        ~LuaScript() noexcept;
    };

    LuaScript &script_from_state(lua_State *state) noexcept;
    void refresh_client_index(lua_State *state) noexcept;
    void refresh_variables(lua_State *state) noexcept;
    void print_error(lua_State *state) noexcept;
}

#endif
