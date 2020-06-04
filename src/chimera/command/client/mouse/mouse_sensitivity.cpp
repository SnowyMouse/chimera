// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool mouse_sensitivity_command(int argc, const char **argv) {
        static bool active = false;
        static float horiz = 0;
        static float vert = 0;

        // This is the offset (-2.5 = 0 movement unless mouse acceleration is enabled, which then it may invert movement if moving too quickly)
        const float OFFSET = 2.5;

        // These are the signatures.
        auto &mouse_horiz_1_sig = get_chimera().get_signature("mouse_horiz_1_sig");
        auto &mouse_horiz_2_sig = get_chimera().get_signature("mouse_horiz_2_sig");
        auto &mouse_vert_1_sig = get_chimera().get_signature("mouse_vert_1_sig");
        auto &mouse_vert_2_sig = get_chimera().get_signature("mouse_vert_2_sig");

        if(argc == 2) {
            // Read the new values, subtracting the offset.
            horiz = strtof(argv[0], nullptr) - OFFSET;
            vert = strtof(argv[1], nullptr) - OFFSET;

            // Overwrite with the new values.
            overwrite(mouse_horiz_1_sig.data() + 2, &horiz);
            overwrite(mouse_horiz_2_sig.data() + 1, &horiz);
            overwrite(mouse_vert_1_sig.data() + 2, &vert);
            overwrite(mouse_vert_2_sig.data() + 2, &vert);

            active = true;
        }
        else if(argc == 1) {
            // Undo the thing.
            mouse_horiz_1_sig.rollback();
            mouse_horiz_2_sig.rollback();
            mouse_vert_1_sig.rollback();
            mouse_vert_2_sig.rollback();
            active = false;
        }
        if(active) {
            // Display the offset values.
            console_output(localize("chimera_mouse_sensitivity_command_setting"), horiz + OFFSET, vert + OFFSET);
        }
        else {
            console_output("off");
        }

        return true;
    }
}
