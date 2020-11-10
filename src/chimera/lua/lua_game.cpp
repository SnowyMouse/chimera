// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include "../console/console.hpp"
#include "../halo_data/globals.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/menu.hpp"
#include "../halo_data/player.hpp"
#include "../halo_data/resolution.hpp"
#include "../halo_data/script.hpp"
#include "../halo_data/object.hpp"
#include "../halo_data/table.hpp"
#include "../halo_data/tag_class.hpp"
#include "../halo_data/tag.hpp"
#include "../event/tick.hpp"
#include "../localization/localization.hpp"
#include "../math_trig/math_trig.hpp"
#include "../output/draw_text.hpp"
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
                message = localize(lua_toboolean(state, 1) ? "common_true" : "common_false");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "console_out");
        }
    }

    static int lua_delete_object(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            ObjectID object_id;
            object_id.whole_id = luaL_checknumber(state, 1);
            if(object_id.is_null()) {
                return luaL_error(state, localize("chimera_lua_error_invalid_object_id"));
            }
            delete_object(object_id);
            return 0;
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "delete_object");
        }
    }

    static int lua_draw_text(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 11) {
            // Text
            const char *text = luaL_checkstring(state, 1);

            // Get the width scale
            auto &resolution = get_resolution();
            float aspect_ratio = static_cast<float>(resolution.width) / resolution.height;
            float width_scale = (aspect_ratio * 480.0f) / 640.0f;
            
            // Frame bounds
            std::int16_t offset_left = round(luaL_checknumber(state, 2) * width_scale);
            std::int16_t offset_top = luaL_checknumber(state, 3);
            std::int16_t offset_right = round(luaL_checknumber(state, 4) * width_scale);
            std::int16_t offset_bottom = luaL_checknumber(state, 5);

            // Font to use (Font tag id or generic font)
            TagID font_tag_id;
            if(lua_isnumber(state, 6)) {
                font_tag_id.whole_id = luaL_checknumber(state, 6);
                if(!get_tag(font_tag_id)) {
                    return luaL_error(state, localize("chimera_lua_error_draw_text_invalid_font_id"));
                }
            }
            else {
                auto generic_font = generic_font_from_string(luaL_checkstring(state, 6));
                font_tag_id = get_generic_font(generic_font);
            }

            // Text alignment
            FontAlignment font_alignment;
            const char *align = luaL_checkstring(state, 7);
            if(std::strcmp(align, "left") == 0) {
                font_alignment = ALIGN_LEFT;
            }
            else if (std::strcmp(align, "right") == 0) {
                font_alignment = ALIGN_RIGHT;
            }
            else if (std::strcmp(align, "center") == 0) {
                font_alignment = ALIGN_CENTER;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_draw_text_invalid_alignment"));
            }

            // Text color
            ColorARGB color;
            color.alpha = luaL_checknumber(state, 8);
            color.red = luaL_checknumber(state, 9);
            color.green = luaL_checknumber(state, 10);
            color.blue = luaL_checknumber(state, 11);

            // Draw!
            apply_text(text, offset_left, offset_top, offset_right, offset_bottom, color, font_tag_id, font_alignment, TextAnchor::ANCHOR_TOP_LEFT);

            return 0;
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "draw_text");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "execute_script");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "get_dynamic_player");
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
                    return luaL_error(state, localize("chimera_lua_error_global_not_found"));
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "get_global");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "get_object");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "get_player");
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "get_tag");
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
                message = localize(lua_toboolean(state, 1) ? "common_true" : "common_false");
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
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "hud_message");
        }
    }

    static int lua_load_ui_widget(lua_State *state) noexcept {
        auto args = lua_gettop(state);
        if(args == 1) {
            if(lua_isstring(state, 1)) {
                const char *path = luaL_checkstring(state, 1);
                lua_pushboolean(state, load_ui_widget(path));
                return 1;
            }
            else if(lua_isnumber(state, 1)) {
                auto *tag = get_tag(luaL_checkinteger(state, 1));
                if(tag) {
                    lua_pushboolean(state, load_ui_widget(tag->path));
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_invalid_function_argument"), 1, "load_ui_widget");
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "load_ui_widget");
        }
    }

    static int lua_set_global(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 2) {
            const char *global_name = luaL_checkstring(state, 1);
            ScriptingGlobal value = read_global(global_name);
            switch(value.type) {
                case SCRIPTING_GLOBAL_NOT_FOUND:
                case SCRIPTING_GLOBAL_UNIMPLEMENTED:
                    return luaL_error(state, localize("chimera_lua_error_global_not_found"));
                case SCRIPTING_GLOBAL_BOOLEAN:
                    if(lua_isboolean(state, 2)) {
                        value.value.boolean = lua_toboolean(state, 2);
                    }
                    else if(lua_isnumber(state, 2)) {
                        value.value.boolean = lua_tointeger(state, 2) != 0;
                    }
                    else {
                        return luaL_error(state, localize("chimera_lua_error_expected_number_or_boolean"), 2);
                    }
                    break;
                case SCRIPTING_GLOBAL_REAL:
                    if(lua_isnumber(state, 2)) {
                        value.value.real = lua_tonumber(state, 2);
                    }
                    else {
                        return luaL_error(state, localize("chimera_lua_error_expected_number"), 2);
                    }
                    break;
                case SCRIPTING_GLOBAL_SHORT:
                    if(lua_isnumber(state, 2)) {
                        value.value.short_int = lua_tointeger(state, 2);
                    }
                    else {
                        return luaL_error(state, localize("chimera_lua_error_expected_number"), 2);
                    }
                    break;
                case SCRIPTING_GLOBAL_LONG:
                    if(lua_isnumber(state, 2)) {
                        value.value.long_int = lua_tointeger(state, 2);
                    }
                    else {
                        return luaL_error(state, localize("chimera_lua_error_expected_number"), 2);
                    }
                    break;
            }
            set_global(global_name, value.value);
            return 0;
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "set_global");
        }
    }

    static int lua_set_timer(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args >= 2) {
            auto &script = script_from_state(state);
            auto interval = luaL_checknumber(state, 1);
            if(interval < 0.1) {
                return luaL_error(state, localize("chimera_lua_error_minimum_timer_interval"));
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "set_timer");
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
                    return 0;
                }
            }
            return luaL_error(state, localize("chimera_lua_error_timer_does_not_exists"));
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "stop_timer");
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
                    return luaL_error(state, localize("chimera_lua_error_spawn_object_invalid_id"));
                }

                args_offset = 2;
            }
            else {
                auto *tag = get_tag(luaL_checkstring(state, 2), luaL_checkstring(state, 1));
                if(!tag) {
                    return luaL_error(state, localize("chimera_lua_error_spawn_object_invalid_path"));
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
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "spawn_object");
        }
    }

    static int lua_tick_rate(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            auto value = luaL_checknumber(state, 1);
            if(value < 0.01) return luaL_error(state, localize("chimera_lua_error_minimum_tick_rate"));
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
        lua_register(state, "draw_text", lua_draw_text);
        lua_register(state, "execute_script", lua_execute_script);
        lua_register(state, "get_dynamic_player", lua_get_dynamic_player);
        lua_register(state, "get_global", lua_get_global);
        lua_register(state, "get_object", lua_get_object);
        lua_register(state, "get_player", lua_get_player);
        lua_register(state, "get_tag", lua_get_tag);
        lua_register(state, "hud_message", lua_hud_message);
        lua_register(state, "load_ui_widget", lua_load_ui_widget);
        lua_register(state, "set_callback", lua_set_callback);
        lua_register(state, "set_global", lua_set_global);
        lua_register(state, "set_timer", lua_set_timer);
        lua_register(state, "stop_timer", lua_stop_timer);
        lua_register(state, "spawn_object", lua_spawn_object);
        lua_register(state, "tick_rate", lua_tick_rate);
        lua_register(state, "ticks", lua_ticks);
    }
}
