#include "../../../localization/localization.hpp"
#include "../../../console/console.hpp"
#include "../../command.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool enable_console_command(int argc, const char **argv) {
        static bool command_used = false;
        static bool command_disabled = false;
        static bool current_setting = true;

        // The first time this is run, check if the console was already enabled (with -console probably)
        if(!command_used) {
            command_used = true;
            command_disabled = get_console_enabled();

            if(command_disabled) {
                return true;
            }
        }

        // If the command is open, we can't disable it until the next load OR if the command was disabled due to -console being used, we can't do anything
        if(argc) {
            bool setting = STR_TO_BOOL(*argv);
            if(!setting && command_disabled) {
                console_warning(localize("chimera_enable_console_command_warning"));
            }
            else if(!setting && get_console_open()) {
                console_warning(localize("chimera_warning_command_will_take_effect_on_next_load"));
            }
            else {
                set_console_enabled(setting);
            }
            current_setting = setting;
        }
        console_output(BOOL_TO_STR(current_setting));
        return true;
    }
}
