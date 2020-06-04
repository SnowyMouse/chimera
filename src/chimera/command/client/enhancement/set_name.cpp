// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../event/map_load.hpp"

namespace Chimera {
    #define MAX_NAME_LEN 11

    static std::int16_t name[256] = {};
    static std::int16_t *old_name = nullptr;

    static void fix_color() {
        name[0] = old_name[0];
        std::copy(old_name + MAX_NAME_LEN + 1, old_name + sizeof(name) / sizeof(*name), name + MAX_NAME_LEN + 1);
    }

    bool set_name_command(int argc, const char **argv) {
        auto &player_name_sig = get_chimera().get_signature("player_name_sig");

        if(argc) {
            std::size_t name_length = std::strlen(*argv);

            if(!old_name) {
                old_name = *reinterpret_cast<std::int16_t **>(player_name_sig.data() + 1);
            }

            if(name_length == 0) {
                player_name_sig.rollback(); // roll it back
                remove_map_load_event(fix_color);
            }
            else {
                overwrite(player_name_sig.data() + 1, static_cast<std::int16_t *>(name));
                add_map_load_event(fix_color);
                fix_color();
            }

            // See if we can get a name
            wchar_t new_name[MAX_NAME_LEN + 1] = {};
            if(MultiByteToWideChar(CP_UTF8, 0, *argv, -1, new_name, sizeof(new_name) / sizeof(*new_name)) == 0) {
                console_error(localize("chimera_set_name_invalid_name_error"), *argv);
                return false;
            }
            else {
                std::copy(new_name, new_name + sizeof(new_name) / sizeof(*new_name), name + 1);
            }
        }

        console_output("%S", name + 1);

        return true;
    }
}
