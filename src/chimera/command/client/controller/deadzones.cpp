// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

static constexpr std::int16_t MAX_INPUT = 0x1000;
static std::int16_t min_input = 0x0;
static std::int16_t input_range = 0x0;

extern "C" {
    extern void handle_deadzones_asm();
}

extern "C" std::uint32_t handle_deadzones_cpp(std::uint32_t x) {
    std::int16_t value = static_cast<std::int16_t>(x);
    std::int16_t absolute_value, sign;
    if(value < 0) {
        absolute_value = -value;
        sign = -1;
    }
    else {
        absolute_value = value;
        sign = 1;
    }

    // If we fall before the deadzone, return 0
    absolute_value -= min_input;
    if(absolute_value < 0) {
        return 0;
    }

    // Multiply by the max input, divide by input range
    absolute_value = static_cast<std::int16_t>(static_cast<std::int32_t>(MAX_INPUT) * absolute_value / input_range);
    if(absolute_value >= MAX_INPUT) {
        absolute_value = MAX_INPUT;
    }

    return absolute_value * sign;
}

namespace Chimera {
    bool deadzones_command(int argc, const char **argv) {
        static Hook hook;
        static float deadzone_value = 0.0F;
        static bool enabled = false;
        auto analog_input_addr = get_chimera().get_signature("analog_input_sig").data();

        if(argc == 1) {
            deadzone_value = std::stof(*argv);
            if(deadzone_value <= 0.0F) {
                deadzone_value = 0.0F;
                if(enabled) {
                    hook.rollback();
                    enabled = false;
                }
            }
            else {
                if(deadzone_value >= 1.0F) {
                    deadzone_value = 1.0F;
                }

                min_input = MAX_INPUT * deadzone_value;
                if(min_input >= MAX_INPUT) {
                    min_input = MAX_INPUT - 1;
                }

                input_range = MAX_INPUT - min_input;

                if(!enabled) {
                    write_jmp_call(analog_input_addr, hook, reinterpret_cast<const void *>(handle_deadzones_asm), nullptr);
                    enabled = true;
                }
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
