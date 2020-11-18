// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <lua.hpp>
#include "../halo_data/map.hpp"
#include "../localization/localization.hpp"
#include "../output/output.hpp"
#include "../version.hpp"
#include "../chimera.hpp"
#include "../halo_data/game_engine.hpp"
#include "lua_filesystem.hpp"
#include "lua_game.hpp"
#include "lua_variables.hpp"
#include "lua_io.hpp"

namespace fs = std::filesystem;

namespace Chimera {
    std::vector<std::unique_ptr<LuaScript>> scripts;

    static std::string read_script_file(fs::path script_path) noexcept {
        std::ifstream file(script_path, std::ios::binary);
        if(file.is_open()) {
            std::stringstream file_content;
            file_content << file.rdbuf();
            file.close();
            return file_content.str();
        }
        return std::string();
    }

    static void load_lua_script(const char *script_name, const char *lua_script_data, size_t lua_script_data_size, bool sandbox, bool global) noexcept {
        // Create a new state for this script
        auto *state = luaL_newstate();

        luaL_openlibs(state);

        // Update Lua path
        auto new_lua_path = fs::path(get_chimera().get_path()) / "lua" / "modules";
        lua_getglobal(state, "package");
        lua_pushstring(state, (new_lua_path.string() + "\\?.lua").c_str());
        lua_setfield(state, -2, "path");
        lua_pop(state, 1);

        if(sandbox) {
            const char *sandbox_script = "io = nil\
                                        dofile = nil\
                                        getfenv = nil\
                                        load = nil\
                                        loadfile = nil\
                                        loadstring = nil\
                                        require = nil\
                                        os.execute = nil\
                                        os.exit = nil\
                                        os.remove = nil\
                                        os.rename = nil\
                                        os.tmpname = nil";

            auto load_result = luaL_loadbuffer(state, sandbox_script, std::strlen(sandbox_script), script_name);

            if(load_result != LUA_OK || lua_pcall(state, 0, 0, 0) != LUA_OK) {
                print_error(state);
                lua_close(state);
                return;
            }
        }

        // Set up functions
        set_fs_functions(state);
        set_io_functions(state);
        set_game_functions(state);

        // Refresh variables
        refresh_variables(state);

        // Set up script globals
        lua_pushstring(state, script_name);
        lua_setglobal(state, "script_name");

        lua_pushstring(state, global ? "global" : "map");
        lua_setglobal(state, "script_type");
        
        lua_pushboolean(state, sandbox);
        lua_setglobal(state, "sandboxed");

        lua_pushinteger(state, CHIMERA_GIT_COMMIT_COUNT);
        lua_setglobal(state, "build");

        lua_pushstring(state, CHIMERA_VERSION_STRING);
        lua_setglobal(state, "full_build");

        scripts.push_back(std::make_unique<LuaScript>(state, script_name, global, sandbox));

        // Load script into Lua state
        auto script_load_result = luaL_loadbuffer(state, lua_script_data, lua_script_data_size, script_name);

        // Set output prefix
        extern const char *output_prefix;
        auto prefix = ("[" + std::string(script_name) + "]");
        output_prefix = prefix.c_str();

        // Execute script
        if(script_load_result != LUA_OK || lua_pcall(state, 0, 0, 0) != LUA_OK) {
            // Clear the output prefix
            output_prefix = nullptr;

            // Print error messages
            console_error(localize("chimera_lua_error_failed_to_load_script"), script_name);
            print_error(state);

            // Close state and erase broken script
            lua_close(state);
            scripts.erase(scripts.begin() + scripts.size() - 1);
            return;
        }

        // Clear the output prefix
        output_prefix = nullptr;

        lua_getglobal(state, "clua_version");

        if(lua_isnumber(state, -1)) {
            double version = lua_tonumber(state, -1);
            if(version > CHIMERA_LUA_VERSION) {
                console_warning(localize("chimera_lua_warning_script_too_updated"), script_name);
                console_warning(localize("chimera_lua_warning_script_possibly_not_work"));
            }
            else if(static_cast<int>(version) < static_cast<int>(CHIMERA_LUA_VERSION)) {
                console_warning(localize("chimera_lua_warning_script_too_outdated"), script_name);
                console_warning(localize("chimera_lua_warning_script_possibly_not_work"));
            }
            else {
                script_from_state(state).version = version;
            }
        }
        else {
            console_warning(localize("chimera_lua_warning_script_undefined_api_version"), script_name);
            console_warning(localize("chimera_lua_warning_script_possibly_not_work"));
        }
        lua_pop(state, 1);

        script_from_state(state).loaded = true;
    }

    void load_map_script() noexcept {
        auto &map_header = get_map_header();
        auto lua_directory = fs::path(get_chimera().get_path()) / "lua";
        auto script_path = lua_directory / "scripts" / "map" / (std::string(map_header.name) + ".lua");
        if(fs::exists(script_path)) {
            auto script = read_script_file(script_path);
            if(!script.empty()) {
                auto script_name = script_path.filename().string();
                load_lua_script(script_name.c_str(), script.c_str(), script.size(), false, false);
            }
        }
        
        // TODO: Add a new method for handling Lua scripts in maps, since this gets destroyed by zstandard compression... and there may be invalid stuff in here 
        /* else if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            auto *script = reinterpret_cast<const char *>(map_header.lua_script_data);
            auto script_size = map_header.lua_script_size;
            if(script && script_size) {
                auto map_filename = std::string(map_header.name) + ".map";
                load_lua_script(map_filename.c_str(), script, script_size, true, false);
            }
        } */
    }

    void load_global_scripts() noexcept {
        auto lua_directory = fs::path(get_chimera().get_path()) / "lua";
        for(auto &entry : fs::directory_iterator(lua_directory / "scripts" / "global")) {
            auto file_path = entry.path();
            if(file_path.extension() == ".lua") {
                auto script = read_script_file(file_path);
                if(!script.empty()) {
                    auto script_name = file_path.filename().string();
                    load_lua_script(script_name.c_str(), script.c_str(), script.size(), false, true);
                }
            }
        }
    }

    void unload_scripts() noexcept {
        scripts.clear();
    }

    LuaScript &script_from_state(lua_State *state) noexcept {
        for(std::size_t i = 0; i < scripts.size(); i++) {
            auto &s = *scripts[i].get();
            if(s.state == state) {
                return s;
            }
        }
        // This shouldn't happen
        std::terminate();
    }

    void print_error(lua_State *state) noexcept {
        console_error(lua_tostring(state, -1));
        lua_pop(state, 1);
    }

    LuaScript::LuaScript(lua_State *state, const char *name, const bool &global, const bool &sandbox) noexcept : state(state), name(name), sandbox(sandbox), global(global) {}

    LuaScript::~LuaScript() noexcept {
        if(this->loaded) {
            lua_getglobal(this->state, this->c_unload.callback_function.data());
            if(!lua_isnil(this->state, -1) && lua_pcall(this->state, 0, 0, 0) != LUA_OK) {
                print_error(this->state);
            }
            lua_close(this->state);
        }
    }

    LuaAmbiguousTypeArgument LuaAmbiguousTypeArgument::check_argument(LuaScript &script, int arg, bool do_lua_error) {
        LuaAmbiguousTypeArgument argument;
        if(script.version < 2.03) { // BC
            argument.argument_type = LuaAmbiguousTypeArgument::ARGUMENT_STRING;
            argument.string_value = luaL_checkstring(script.state, arg);
        }
        else if(lua_isboolean(script.state, arg)) {
            argument.argument_type = LuaAmbiguousTypeArgument::ARGUMENT_BOOLEAN;
            argument.bool_value = lua_toboolean(script.state, arg);
        }
        else if(lua_isstring(script.state, arg)) {
            argument.argument_type = LuaAmbiguousTypeArgument::ARGUMENT_STRING;
            argument.string_value = lua_tostring(script.state, arg);
        }
        else if(lua_isnumber(script.state, arg)) {
            argument.argument_type = LuaAmbiguousTypeArgument::ARGUMENT_NUMBER;
            argument.number_value = lua_tonumber(script.state, arg);
        }
        else if(lua_isnil(script.state, arg)) {
            argument.argument_type = LuaAmbiguousTypeArgument::ARGUMENT_NIL;
        }
        else {
            if(do_lua_error) luaL_error(script.state, localize("chimera_lua_error_invalid_timer_argument"));
            else throw std::exception();
        }
        return argument;
    }

    void LuaAmbiguousTypeArgument::push_argument(LuaScript &script) noexcept {
        switch(this->argument_type) {
            case LuaAmbiguousTypeArgument::ARGUMENT_BOOLEAN:
                lua_pushboolean(script.state, this->bool_value);
                break;
            case LuaAmbiguousTypeArgument::ARGUMENT_STRING:
                lua_pushstring(script.state, this->string_value.data());
                break;
            case LuaAmbiguousTypeArgument::ARGUMENT_NUMBER:
                lua_pushnumber(script.state, this->number_value);
                break;
            case LuaAmbiguousTypeArgument::ARGUMENT_NIL:
                lua_pushnil(script.state);
                break;
        }
    }
}
