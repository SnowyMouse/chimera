// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"

namespace Chimera {
    bool language_command(int argc, const char **argv) {
        if(argc) {
            bool found = false;
            for(Language l = static_cast<Language>(0); l < Language::CHIMERA_LANGUAGE_COUNT; l = static_cast<Language>(l + 1)) {
                if(std::strcmp(localize("chimera_language_shortened_name", l), argv[0]) == 0) {
                    get_chimera().set_language(l);
                    found = true;
                }
            }

            if(!found) {
                console_error(localize("chimera_language_command_error_invalid_language"), argv[0]);
                return false;
            }
        }
        else {
            extern const char *output_prefix;
            const char *old_prefix = output_prefix;
            output_prefix = nullptr;

            console_output(ConsoleColor::header_color(), localize("chimera_language_command_available_languages"));
            for(Language l = static_cast<Language>(0); l < Language::CHIMERA_LANGUAGE_COUNT; l = static_cast<Language>(l + 1)) {
                if(l == Language::CHIMERA_LANGUAGE_VAP) {
                    continue;
                }
                console_output(ConsoleColor::body_color(), "  - %s", localize("chimera_language_shortened_name", l));
            }

            output_prefix = old_prefix;
        }

        console_output(localize("chimera_language_shortened_name"));

        return true;
    }
}
