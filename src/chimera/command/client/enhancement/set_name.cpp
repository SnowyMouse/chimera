#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static wchar_t name[13] = {};

    bool set_name_command(int argc, const char **argv) {
        auto &player_name_sig = get_chimera().get_signature("player_name_sig");

        if(argc) {
            std::size_t name_length = std::strlen(*argv);

            if(name_length == 0) {
                player_name_sig.rollback(); // roll it back
            }
            else if(name_length > 11) {
                console_error("Invalid name %s. Name exceeds the maximum name size of 11 characters.", *argv);
                return false;
            }
            else {
                overwrite(player_name_sig.data() + 1, static_cast<wchar_t *>(name));
            }

            // Copy the name (with null terminator)
            std::copy(*argv, *argv + name_length + 1, name + 1);
        }

        console_output("%S", name + 1);

        return true;
    }
}
