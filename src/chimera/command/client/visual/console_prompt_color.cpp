#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"

namespace Chimera {
    bool console_prompt_color_command(int argc, const char **argv) {
        // Set the prompt color
        static ConsoleColor *color = nullptr;
        if(!color) {
            color = *reinterpret_cast<ConsoleColor **>(get_chimera().get_signature("console_prompt_color_sig").data() + 1);
        }

        // If we have 3 arguments, try to get the color
        if(argc == 3) {
            color->r = std::strtof(argv[0], nullptr);
            if(color->r < 0.0) {
                color->r = 0.0;
            }
            else if(color->r > 1.0) {
                color->r = 1.0;
            }

            color->g = std::strtof(argv[1], nullptr);
            if(color->g < 0.0) {
                color->g = 0.0;
            }
            else if(color->g > 1.0) {
                color->g = 1.0;
            }

            color->b = std::strtof(argv[2], nullptr);
            if(color->b < 0.0) {
                color->b = 0.0;
            }
            else if(color->b > 1.0) {
                color->b = 1.0;
            }

            if(color->r == 0.0 && color->g == 1.0 && color->b == 1.0) {
                console_output(*color, "Vap!~");
            }
        }

        // If we don't, turn off the mod
        else if(argc > 0) {
            color->r = 1.0;
            color->g = 0.3;
            color->b = 1.0;
        }

        console_output(localize("chimera_console_prompt_color_command_output"), color->r, color->g, color->b);

        return true;
    }
}
