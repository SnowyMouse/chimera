#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

extern "C" {
    float deadzone_value = 0.0F;
    extern void handle_deadzones_asm();
}

extern "C" std::uint32_t handle_deadzones_cpp(std::uint32_t x) {
    std::int16_t value = static_cast<std::int16_t>(x);
    std::int16_t absolute_value;
    int sign;
    if(value < 0) {
        absolute_value = -value;
        sign = -1;
    }
    else {
        absolute_value = value;
        sign = 1;
    }

    constexpr std::int16_t MAX_INPUT = 0x1000;
    const std::int16_t MIN_INPUT = static_cast<std::int16_t>(MAX_INPUT * deadzone_value);
    absolute_value -= MIN_INPUT;

    // If we fall before the deadzone, return 0
    if(absolute_value < 0) {
        return 0;
    }

    // Otherwise, scale it
    else {
        const float INPUT_SCALE = static_cast<float>(MAX_INPUT) / (MAX_INPUT - MIN_INPUT);
        absolute_value *= INPUT_SCALE;
    }

    if(absolute_value >= 0x1000) {
        absolute_value = 0x1000;
    }

    return absolute_value * sign;
}

namespace Chimera {
    bool deadzones_command(int argc, const char **argv) {
        static Hook hook;
        auto analog_input_addr = get_chimera().get_signature("analog_input_sig").data();

        if(argc == 1) {
            deadzone_value = std::stof(*argv);
            if(deadzone_value <= 0.0F) {
                deadzone_value = 0.0F;
                hook.rollback();
            }
            else {
                if(deadzone_value >= 0.99F) {
                    deadzone_value = 0.99F;
                }
                write_jmp_call(analog_input_addr, hook, reinterpret_cast<const void *>(handle_deadzones_asm), nullptr);
            }
        }

        if(deadzone_value == 0.0F) {
            console_output("off");
        }
        else {
            console_output("%f", deadzone_value);
        }
        return true;
    }
}
