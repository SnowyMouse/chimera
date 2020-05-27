#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
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
            else if(name_length > MAX_NAME_LEN) {
                console_error("Invalid name %s. Name exceeds the maximum name size of %i characters.", *argv, MAX_NAME_LEN);
                return false;
            }
            else {
                overwrite(player_name_sig.data() + 1, static_cast<std::int16_t *>(name));
                add_map_load_event(fix_color);
                fix_color();
            }

            // Zero out the name and then copy the new name over
            std::fill(name + 1, name + 1 + MAX_NAME_LEN, 0);
            MultiByteToWideChar(CP_UTF8, 0, *argv, static_cast<int>(name_length), reinterpret_cast<LPWSTR>(name + 1), MAX_NAME_LEN - 1);
        }

        console_output("%S", name + 1);

        return true;
    }
}
