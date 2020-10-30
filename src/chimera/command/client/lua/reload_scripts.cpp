// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <memory>
#include "../../command.hpp"
#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../lua/lua_script.hpp"

namespace Chimera {
	bool reload_scripts_command(int, const char **) {
        // Reload everything
        unload_scripts();
        load_global_scripts();
        load_map_script();

        // Remove the output prefix
        extern const char *output_prefix;
        output_prefix = nullptr;
        
        ConsoleColor green = {1, 0.235, 0.82, 0};
        console_output(green, localize("chimera_lua_scripts_were_reloaded"));
        return true;
    }
}