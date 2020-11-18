// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include "../chimera.hpp"
#include "lua_callback.hpp"
#include "lua_script.hpp"

namespace fs = std::filesystem;

namespace Chimera {
    static void setup_lua_folder() {
        auto lua_directory = fs::path(get_chimera().get_path()) / "lua";

        fs::create_directories(lua_directory / "scripts" / "global");
        fs::create_directories(lua_directory / "scripts" / "map");
        fs::create_directories(lua_directory / "data" / "global");
        fs::create_directories(lua_directory / "data" / "map");
        fs::create_directories(lua_directory / "modules");

        //std::ofstream(lua_directory / "loaded-scripts.txt", std::ios::app).close();
        //std::ofstream(lua_directory / "trusted-scripts.txt", std::ios::app).close();

        // Move scripts from old directories
        auto move_scripts = [](fs::path origin, fs::path destination) {
            if(fs::exists(origin)) {
                for(auto &entry : fs::directory_iterator(origin)) {
                    fs::rename(entry.path(), destination / entry.path().filename());
                }
                fs::remove(origin);
            }
        };
        move_scripts(lua_directory / "global", lua_directory / "scripts" / "global");
        move_scripts(lua_directory / "map", lua_directory / "scripts" / "map");
    }

    void setup_lua_scripting() {
        static bool already_setup = false;
        if(already_setup) {
            return;
        }

        setup_lua_folder();
        setup_callbacks();
        load_global_scripts();
        load_map_script();

        already_setup = true;
    }

    void destroy_lua_scripting() {
        unload_scripts();
    }
}