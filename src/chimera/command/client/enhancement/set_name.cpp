#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool set_name_command(int argc, const char **argv) {
        static wchar_t *name = nullptr;
        if(!name) {
            name = *reinterpret_cast<wchar_t **>(get_chimera().get_signature("player_name_sig").data() + 1) + 1;
        }
        if(argc) {
            std::size_t name_length = std::strlen(*argv);
            if(name_length > 11) {
                console_error("Invalid name %s. Name exceeds the maximum name size of 11 characters.", *argv);
                return false;
            }

            // Copy the name and the null terminator
            std::copy(*argv, *argv + name_length + 1, name);
        }
        console_output("%S", name);
        return true;
    }
}
