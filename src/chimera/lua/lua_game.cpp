// SPDX-License-Identifier: GPL-3.0-only

#include "../console/console.hpp"
#include "../halo_data/globals.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/resolution.hpp"
#include "../halo_data/script.hpp"
#include "../halo_data/object.hpp"
#include "../halo_data/table.hpp"
#include "../halo_data/tag_class.hpp"
#include "../halo_data/tag.hpp"
#include "../event/tick.hpp"
#include "../math_trig/math_trig.hpp"
#include "../output/output.hpp"
#include "../chimera.hpp"
#include "lua_game.hpp"
#include "lua_callback.hpp"

namespace Chimera {
    static int lua_console_is_open(lua_State *state) noexcept {
        lua_pushboolean(state, get_console_open());
        return 1;
    }

    static int lua_console_out(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1 || args == 4 || args == 5) {
            const char *message;
            if(lua_isboolean(state, 1)) {
                message = lua_toboolean(state, 1) ? "true" : "false";
            }
            else if(lua_isnil(state, 1)) {
                message = "(nil)";
            }
            else {
                message = luaL_checkstring(state, 1);
            }

            ConsoleColor argb;
            int offset = 0;
            if(args >= 4) {
                if(args >= 5) {
                    argb.a = luaL_checknumber(state, 2);
                    offset = 1;
                }
                else {
                    argb.a = 1;
                }
                argb.r = luaL_checknumber(state, offset + 2);
                argb.g = luaL_checknumber(state, offset + 3);
                argb.b = luaL_checknumber(state, offset + 4);
            }
            else {
                argb = {1, 1, 1, 1};
            }

            console_output(argb, message);

            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in console_out");
        }
    }

    static int lua_delete_object(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            ObjectID object_id;
            object_id.whole_id = luaL_checknumber(state, 1);
            if(object_id.is_null()) {
                return luaL_error(state, "invalid object id");
            }
            delete_object(object_id);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in delete_object");
        }
    }

    static int lua_execute_script(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1 || args == 4 || args == 5) {
            const char *script = luaL_checkstring(state, 1);
            execute_script(script, true);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in execute_script");
        }
    }

    static int lua_get_dynamic_player(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args <= 1) {
            auto &objects_table = ObjectTable::get_object_table();
            auto &players_table = PlayerTable::get_player_table();
            BaseDynamicObject *dynamic_object = nullptr;
            
            if(args == 0) {
                auto *player = players_table.get_client_player();
                if(player) {
                    dynamic_object = objects_table.get_dynamic_object(player->object_id);
                }
            }
            else {
                std::size_t index = luaL_checkinteger(state, 1);
                auto *player = players_table.get_player_by_rcon_id(index);
                if(player) {
                    dynamic_object = objects_table.get_dynamic_object(player->object_id);
                }
            }

            if(dynamic_object) {
                lua_pushinteger(state, reinterpret_cast<std::uint32_t>(dynamic_object));
            }
            else {
                lua_pushnil(state);
            }

            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in get_dynamic_player");
        }
    }

    static int lua_get_global(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *global_name = luaL_checkstring(state, 1);
            ScriptingGlobal s = read_global(global_name);
            switch(s.type) {
                case SCRIPTING_GLOBAL_NOT_FOUND:
                case SCRIPTING_GLOBAL_UNIMPLEMENTED:
                    return luaL_error(state, "global not found or not implemented");
                case SCRIPTING_GLOBAL_BOOLEAN:
                    lua_pushboolean(state, s.value.boolean);
                    return 1;
                case SCRIPTING_GLOBAL_REAL:
                    lua_pushnumber(state, s.value.real);
                    return 1;
                case SCRIPTING_GLOBAL_SHORT:
                    lua_pushinteger(state, s.value.short_int);
                    return 1;
                case SCRIPTING_GLOBAL_LONG:
                    lua_pushinteger(state, s.value.long_int);
                    return 1;
            }
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in set_global");
        }
    }

    static int lua_get_object(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto &objects_table = ObjectTable::get_object_table();
            auto id_int = luaL_checkinteger(state, 1);

            if(id_int < 0xFFFF) {
                auto *dynamic_object = objects_table.get_dynamic_object(id_int);
                if(dynamic_object) {
                    lua_pushinteger(state, reinterpret_cast<std::uint32_t>(dynamic_object));
                }
                else {
                    lua_pushnil(state);
                }
            }
            else {
                ObjectID object_id;
                object_id.whole_id = id_int;
                auto *dynamic_object = objects_table.get_dynamic_object(object_id);
                if(dynamic_object) {
                    lua_pushinteger(state, reinterpret_cast<std::uint32_t>(dynamic_object));
                }
                else {
                    lua_pushnil(state);
                }
            }
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in get_object");
        }
    }

    static int lua_get_player(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args <= 1) {
            auto &players_table = PlayerTable::get_player_table();
            Player *player = nullptr;
            
            if(args == 0) {
                player = players_table.get_client_player();
            }
            else {
                std::size_t index = luaL_checkinteger(state, 1);
                player = players_table.get_player_by_rcon_id(index);
            }
            
            if(player) {
                lua_pushinteger(state, reinterpret_cast<std::uint32_t>(player));
            }
            else {
                lua_pushnil(state);
            }

            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in get_player");
        }
    }

    static int lua_get_tag(lua_State *state) noexcept {
        int args = lua_gettop(state);
        Tag *tag = nullptr;

        if(args == 1) {
            TagID tag_id;
            tag_id.whole_id = luaL_checkinteger(state, 1);

            if(!tag_id.is_null()) {
                tag = get_tag(tag_id);
            }
            else {
                lua_pushnil(state);
            }
        }
        else if(args == 2) {
            const char *tag_class = luaL_checkstring(state, 1);
            const char *tag_path = luaL_checkstring(state, 2);

            auto tag_class_int = tag_class_from_string(tag_class);
            
            if(tag_class_int != TagClassInt::TAG_CLASS_NULL) {
                tag = get_tag(tag_path, tag_class_int);
            }
            else {
                tag = get_tag(tag_path, tag_class);
            }
        }
        else {
            return luaL_error(state, "wrong number of arguments in get_tag");
        }
        
        if(tag) {
            lua_pushinteger(state, reinterpret_cast<std::uint32_t>(tag));
        }
        else {
            lua_pushnil(state);
        }
        
        return 1;
    }

    static int lua_hud_message(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *message;
            if(lua_isboolean(state, 1)) {
                message = lua_toboolean(state, 1) ? "true" : "false";
            }
            else if(lua_isnil(state, 1)) {
                message = "(nil)";
            }
            else {
                message = luaL_checkstring(state, 1);
            }
            hud_output_raw(message);
            return 0;
        }
        return luaL_error(state, "wrong number of arguments in hud_message");
    }

    static int lua_set_global(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 2) {
            const char *global_name = luaL_checkstring(state, 1);
            ScriptingGlobal value = read_global(global_name);
            switch(value.type) {
                case SCRIPTING_GLOBAL_NOT_FOUND:
                case SCRIPTING_GLOBAL_UNIMPLEMENTED:
                    return luaL_error(state, "global not found or not implemented");
                case SCRIPTING_GLOBAL_BOOLEAN:
                    if(lua_isboolean(state, 2)) {
                        value.value.boolean = lua_toboolean(state, 2);
                    }
                    else if(lua_isnumber(state, 2)) {
                        value.value.boolean = lua_tointeger(state, 2) != 0;
                    }
                    else {
                        return luaL_error(state, "expected a number or boolean in argument 2");
                    }
                    break;
                case SCRIPTING_GLOBAL_REAL:
                    if(lua_isnumber(state, 2)) {
                        value.value.real = lua_tonumber(state, 2);
                    }
                    else {
                        return luaL_error(state, "expected a number in argument 2");
                    }
                    break;
                case SCRIPTING_GLOBAL_SHORT:
                    if(lua_isnumber(state, 2)) {
                        value.value.short_int = lua_tointeger(state, 2);
                    }
                    else {
                        return luaL_error(state, "expected a number in argument 2");
                    }
                    break;
                case SCRIPTING_GLOBAL_LONG:
                    if(lua_isnumber(state, 2)) {
                        value.value.long_int = lua_tointeger(state, 2);
                    }
                    else {
                        return luaL_error(state, "expected a number in argument 2");
                    }
                    break;
            }
            set_global(global_name, value.value);
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in set_global");
        }
    }

    static int lua_set_timer(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args >= 2) {
            auto &script = script_from_state(state);
            auto interval = luaL_checknumber(state, 1);
            if(interval < 0.1) {
                return luaL_error(state, "interval must be at least 0.1 millseconds");
            }
            auto *function = luaL_checkstring(state, 2);
            LuaScriptTimer timer;
            timer.timer_id = script.next_timer_id++;
            timer.interval_ms = interval;
            timer.function = function;
            for(int i = 3; i <= args; i++) {
                timer.arguments.push_back(LuaAmbiguousTypeArgument::check_argument(script, i, true));
            }
            lua_pushinteger(state, timer.timer_id);
            script.timers.emplace_back(timer);
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in set_timer");
        }
    }

    static int lua_stop_timer(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto &script = script_from_state(state);
            auto id = luaL_checkinteger(state, 1);
            for(std::size_t i = 0; i < script.timers.size(); i++) {
                if(script.timers[i].timer_id == id) {
                    script.timers.erase(script.timers.begin() + i);
                    return luaL_error(state, "timer with that ID does not exist");
                }
            }
            return 0;
        }
        else {
            return luaL_error(state, "wrong number of arguments in stop_timer");
        }
    }

    static int lua_spawn_object(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 5 || args == 4) {
            int args_offset = 0;
            TagID tag_id;

            if(lua_isnumber(state, 1)) {
                tag_id.whole_id = luaL_checknumber(state, 1);

                if(tag_id.is_null() || tag_id.index.index >= get_tag_data_header().tag_count) {
                    return luaL_error(state, "invalid tag ID specified in spawn_object");
                }

                args_offset = 2;
            }
            else {
                auto *tag = get_tag(luaL_checkstring(state, 2), luaL_checkstring(state, 1));
                if(!tag) {
                    return luaL_error(state,"invalid tag specified in spawn_object");
                }
                tag_id = tag->id;
                args_offset = 3;
            }

            float x = luaL_checknumber(state, args_offset);
            float y = luaL_checknumber(state, args_offset + 1);
            float z = luaL_checknumber(state, args_offset + 2);

            ObjectID parent;
            parent.whole_id = 0xFFFFFFFF;

            auto object_id = spawn_object(tag_id, x, y, z, parent);
            lua_pushinteger(state, object_id.whole_id);
            return 1;
        }
        else {
            return luaL_error(state, "wrong number of arguments in spawn_object");
        }
    }

    static int lua_tick_rate(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto value = luaL_checknumber(state, 1);
            if(value < 0.01) return luaL_error(state, "tick rate must be at least 0.01");
            set_tick_rate(luaL_checknumber(state, 1));
        }
        lua_pushnumber(state, tick_rate());
        return 1;
    }

    static int lua_ticks(lua_State *state) noexcept {
        auto progress = get_tick_progress();
        if(progress > 0.99999) progress = 0.99999;
        lua_pushnumber(state, static_cast<double>(get_tick_count()) + progress);
        return 1;
    }

    void set_game_functions(lua_State *state) noexcept {
        lua_register(state, "console_is_open", lua_console_is_open);
        lua_register(state, "console_out", lua_console_out);
        lua_register(state, "delete_object", lua_delete_object);
        lua_register(state, "execute_script", lua_execute_script);
        lua_register(state, "get_dynamic_player", lua_get_dynamic_player);
        lua_register(state, "get_global", lua_get_global);
        lua_register(state, "get_object", lua_get_object);
        lua_register(state, "get_player", lua_get_player);
        lua_register(state, "get_tag", lua_get_tag);
        lua_register(state, "hud_message", lua_hud_message);
        lua_register(state, "set_callback", lua_set_callback);
        lua_register(state, "set_global", lua_set_global);
        lua_register(state, "set_timer", lua_set_timer);
        lua_register(state, "stop_timer", lua_stop_timer);
        lua_register(state, "spawn_object", lua_spawn_object);
        lua_register(state, "tick_rate", lua_tick_rate);
        lua_register(state, "ticks", lua_ticks);
    }
}
