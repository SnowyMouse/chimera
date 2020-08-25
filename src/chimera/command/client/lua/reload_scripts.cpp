// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <memory>
#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../lua/lua_script.hpp"
#include "../../../lua/lua.hpp"

namespace Chimera {
	extern std::vector<std::unique_ptr<LuaScript>> scripts;
	extern void open_lua_scripts();

	bool reload_scripts_command(int argc, const char **argv) {
        // Clear current loaded scripts
        scripts.clear();
        open_lua_scripts();

        // Remove the output prefix
        extern const char *output_prefix;
        output_prefix = nullptr;
        
        console_output({1, 0.235, 0.82, 0}, "Lua scripts were reloaded.");
        return true;
    }
}