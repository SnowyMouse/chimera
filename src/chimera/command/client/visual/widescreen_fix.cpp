#include "../../../fix/widescreen_fix.hpp"
#include "../../../output/output.hpp"
#include "../../command.hpp"

namespace Chimera {
    bool widescreen_fix_command(int argc, const char **argv) {
        static bool enabled = false;
        if(argc) {
            enabled = STR_TO_BOOL(*argv);
            set_widescreen_fix(enabled);
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
