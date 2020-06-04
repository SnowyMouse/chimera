// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool uncap_cinematic_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc == 1) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &uncap_cinematic_sig = get_chimera().get_signature("uncap_cinematic_sig");
                if(new_enabled) {
                    const SigByte uncap_cinematic_data[] = {0xEB, 0x04, 0xB3, 0x01, 0xEB, 0x02, 0x32, 0xDB, 0x8B, 0x2D};
                    write_code_s(uncap_cinematic_sig.data(), uncap_cinematic_data);
                }
                else {
                    uncap_cinematic_sig.rollback();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
