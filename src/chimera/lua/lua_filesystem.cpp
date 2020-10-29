// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include "../localization/localization.hpp"
#include "lua_script.hpp"
#include "lua_filesystem.hpp"

namespace fs = std::filesystem;

namespace Chimera {
    /**
     * Get data directory of a given script state
     * @param state     Lua script state
     * @return          Path to data directory
     */
    fs::path get_script_data_path(lua_State *state) noexcept {
        static auto &chimera = get_chimera();
        static auto scripts_data_directory = fs::path(chimera.get_path()) / "lua" / "data";

        // Get script info
        lua_getglobal(state, "script_name");
        std::string script_filename = lua_tostring(state, -1);
        lua_getglobal(state, "script_type");
        std::string script_type = lua_tostring(state, -1);

        // Remove script file extension
        auto script_name = script_filename.substr(0, script_filename.size() - 4);

        // Create script data folder
        auto data_path = scripts_data_directory / script_type / script_name;

        if(!fs::exists(data_path)) {
            fs::create_directory(data_path);
        }

        return data_path;
    }

    /**
     * Check if a path is inside of the data folder of a given script
     * @param state     Lua script state
     * @param path      Path to validate
     * @return          true if the path valid, false if not
     */
    static bool check_path(lua_State *state, fs::path path) noexcept {
        // Get script data directory
        auto data_directory = get_script_data_path(state);

        auto absolute_path = fs::absolute(data_directory / path);
        if(absolute_path.string().find(data_directory.string()) == 0) {
            return true;
        }
        return false;
    }

    static int lua_create_directory(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                lua_pushboolean(state, fs::create_directories(get_script_data_path(state) / path));
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "create_directory");
        }
    }

    static int lua_remove_directory(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                auto data_directory = get_script_data_path(state);
                if(fs::is_directory(data_directory / path)) {
                    lua_pushboolean(state, fs::remove_all(data_directory / path));
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "remove_directory");
        }
    }

    static int lua_list_directory(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                auto data_directory = get_script_data_path(state);
                if(fs::is_directory(data_directory / path)) {
                    lua_newtable(state);
                    std::size_t table_index = 1;
                    for(auto &entry : fs::directory_iterator(data_directory / path)) {
                        auto filename = entry.path().filename().string();
                        if(fs::is_directory(entry.path())) {
                            filename += "\\";
                        }
                        lua_pushstring(state, filename.c_str());
                        lua_rawseti(state, -2, table_index++);
                    }
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "list_directory");
        }
    }

    static int lua_directory_exists(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                auto data_directory = get_script_data_path(state);
                if(fs::exists(data_directory / path)) {
                    lua_pushboolean(state, fs::is_directory(data_directory / path));
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "directory_exists");
        }
    }

    static int lua_write_file(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 2 || args == 3) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                std::string content = luaL_checkstring(state, 2);
                bool append_content = (args == 3 && (lua_isboolean(state, 3) && lua_toboolean(state, 3)));
                
                std::ofstream file;
                file.open(get_script_data_path(state) / path, (append_content ? std::ios::app : std::ios::trunc));
                if(file.is_open()) {
                    file << content;
                    lua_pushboolean(state, file.good());
                    file.close();
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "write_file");
        }
    }

    static int lua_read_file(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                std::ifstream file;
                file.open(get_script_data_path(state) / path);
                if(file.is_open()) {
                    std::stringstream file_content_stream;
                    std::string line_buffer;
                    while(file.good() && std::getline(file, line_buffer)) {
                        file_content_stream << line_buffer << std::endl;
                    }
                    auto file_content = file_content_stream.str();
                    if(!line_buffer.empty()) {
                        // Remove last newline character
                        file_content.pop_back();
                    }
                    lua_pushstring(state, file_content.c_str());
                    file.close();
                }
                else {
                    lua_pushnil(state);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "read_file");
        }
    }

    static int lua_delete_file(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                auto data_directory = get_script_data_path(state);
                if(fs::is_regular_file(data_directory / path)) {
                    lua_pushboolean(state, fs::remove(data_directory / path));
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "delete_file");
        }
    }

    static int lua_file_exists(lua_State *state) noexcept {
        int args = lua_gettop(state);
        if(args == 1) {
            const char *path = luaL_checkstring(state, 1);
            if(check_path(state, path)) {
                auto data_directory = get_script_data_path(state);
                if(fs::is_regular_file(data_directory / path)) {
                    lua_pushboolean(state, fs::exists(data_directory / path));
                }
                else {
                    lua_pushboolean(state, false);
                }
                return 1;
            }
            else {
                return luaL_error(state, localize("chimera_lua_error_scope_path"));
            }
        }
        else {
            return luaL_error(state, localize("chimera_lua_error_wrong_number_of_arguments"), "file_exists");
        }
    }

    void set_fs_functions(lua_State *state) noexcept {
        lua_register(state, "create_directory", lua_create_directory);
        lua_register(state, "remove_directory", lua_remove_directory);
        lua_register(state, "list_directory", lua_list_directory);
        lua_register(state, "directory_exists", lua_directory_exists);
        lua_register(state, "write_file", lua_write_file);
        lua_register(state, "read_file", lua_read_file);
        lua_register(state, "delete_file", lua_delete_file);
        lua_register(state, "file_exists", lua_file_exists);
    }
}