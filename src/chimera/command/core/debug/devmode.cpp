#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool devmode_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &devmode_sig = get_chimera().get_signature("devmode_sig");
                if(new_enabled) {
                    const SigByte force_devmode[] = { 0x90, 0x90, 0x85, 0xC0, 0x90, 0x90, 0x83, 0xF8, 0x06, 0x7E, 0x6E };
                    write_code(devmode_sig.data(), force_devmode, sizeof(force_devmode)/sizeof(force_devmode[0]));
                }
                else {
                    devmode_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
