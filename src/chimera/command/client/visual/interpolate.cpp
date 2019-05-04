#include <cmath>

#include "../../command.hpp"
#include "../../../output/output.hpp"
#include "../../../fix/interpolate/interpolate.hpp"

namespace Chimera {
    bool interpolate_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {

                enabled = new_enabled;
            }
        }
        console_output(BOOL_TO_STR(enabled));
        return true;
    }
}
