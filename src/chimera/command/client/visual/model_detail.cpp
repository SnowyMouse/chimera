// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../fix/model_detail.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    extern "C" bool force_max_lod_level;

    bool model_detail_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(!new_enabled) {
                    force_max_lod_level = false;
                }
                else {
                    force_max_lod_level = true;
                }
                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
