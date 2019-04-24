#include <cmath>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool camo_fix_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                static std::uint32_t amazing_camo_fix_value = 0;
                auto &alpha_blend_camo_sig = get_chimera().get_signature("alpha_blend_camo_sig");
                auto &dart_1_sig = get_chimera().get_signature("dart_1_sig");
                auto &dart_2_sig = get_chimera().get_signature("dart_2_sig");
                if(new_enabled) {
                    overwrite(alpha_blend_camo_sig.data() + 4, static_cast<float>(1.0 - 0.9 / std::pow(1.34, 13.4)));
                    overwrite(dart_1_sig.data() + 2, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 9, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 16, &amazing_camo_fix_value);
                    overwrite(dart_1_sig.data() + 24, &amazing_camo_fix_value);

                    const SigByte skip_dart[] = { 0xEB, 0x10, 0x90, 0x90, 0x90, 0x90 };
                    write_code_s(dart_2_sig.data(), skip_dart);
                }
                else {
                    alpha_blend_camo_sig.rollback();
                    dart_1_sig.rollback();
                    dart_2_sig.rollback();
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
