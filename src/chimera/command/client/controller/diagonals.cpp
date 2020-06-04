// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool diagonals_command(int argc, const char **argv) {
        static float positive_diagonal = 0.0F;
        static float negative_diagonal = 0.0F;
        if(argc == 1) {
            positive_diagonal = std::stof(*argv);
            if(positive_diagonal < 0.0F) {
                positive_diagonal = 0.0F;
            }
            else if(positive_diagonal > 1.0F) {
                positive_diagonal = 1.0F;
            }
            negative_diagonal = positive_diagonal * -1.0F;
            auto diagonals_addr = get_chimera().get_signature("diagonals_sig").data();
            overwrite(diagonals_addr + 2, &positive_diagonal);
            overwrite(diagonals_addr + 27, &negative_diagonal);
        }
        console_output("%f", positive_diagonal);
        return true;
    }
}
