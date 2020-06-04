// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../fix/model_detail.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool model_detail_command(int argc, const char **argv) {
        if(argc) {
            set_model_detail(std::atof(*argv));
        }
        console_output("%f", get_model_detail());
        return true;
    }
}
