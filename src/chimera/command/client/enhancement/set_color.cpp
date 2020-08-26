// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <chrono>

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../event/map_load.hpp"

namespace Chimera {
    enum PlayerColor {
        PLAYER_COLOR_WHITE = 0,
        PLAYER_COLOR_BLACK,
        PLAYER_COLOR_RED,
        PLAYER_COLOR_BLUE,
        PLAYER_COLOR_GRAY,
        PLAYER_COLOR_YELLOW,
        PLAYER_COLOR_GREEN,
        PLAYER_COLOR_PINK,
        PLAYER_COLOR_PURPLE,
        PLAYER_COLOR_CYAN,
        PLAYER_COLOR_COBALT,
        PLAYER_COLOR_ORANGE,
        PLAYER_COLOR_TEAL,
        PLAYER_COLOR_SAGE,
        PLAYER_COLOR_BROWN,
        PLAYER_COLOR_TAN,
        PLAYER_COLOR_MAROON,
        PLAYER_COLOR_SALMON,
        PLAYER_COLOR_COUNT,
        PLAYER_COLOR_RANDOM = 0xFFFF
    };

    static std::uint32_t chimera_set_color_override = 0xFFFFFFFF;

    extern "C" {
        void get_set_color_setting_asm() noexcept;
    }

    extern "C" std::uint32_t get_set_color_setting() {
        if(chimera_set_color_override == PlayerColor::PLAYER_COLOR_RANDOM) {
            std::srand(std::chrono::steady_clock::now().time_since_epoch().count());
            return std::rand() % PlayerColor::PLAYER_COLOR_COUNT;
        }
        else {
            return chimera_set_color_override;
        }
    }

    bool set_color_command(int argc, const char **argv) {
        auto &chimera = get_chimera();

        if(argc) {
            if(std::strcmp(argv[0], localize("chimera_color_white")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_WHITE;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_black")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_BLACK;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_red")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_RED;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_blue")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_BLUE;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_gray")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_GRAY;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_yellow")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_YELLOW;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_green")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_GREEN;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_pink")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_PINK;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_purple")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_PURPLE;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_cyan")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_CYAN;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_cobalt")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_COBALT;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_orange")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_ORANGE;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_teal")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_TEAL;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_sage")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_SAGE;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_brown")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_BROWN;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_tan")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_TAN;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_maroon")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_MAROON;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_salmon")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_SALMON;
            }
            else if(std::strcmp(argv[0], localize("chimera_color_random")) == 0) {
                chimera_set_color_override = PlayerColor::PLAYER_COLOR_RANDOM;
            }
            else if(std::strcmp(argv[0], localize("off")) == 0) {
                chimera_set_color_override = 0xFFFFFFFF;
            }
            else {
                console_error(localize("chimera_set_color_command_invalid_color"), argv[0]);
                return false;
            }

            auto &sig = chimera.get_signature(chimera.feature_present("client_demo") ? "player_color_trial_sig" : "player_color_sig");

            if(chimera_set_color_override == 0xFFFFFFFF) {
                sig.rollback();
            }
            else {
                static Hook hook;
                static const SigByte nop[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
                write_code_s(sig.data(), nop);
                write_jmp_call(sig.data(), hook, reinterpret_cast<const void *>(get_set_color_setting_asm), nullptr, false);
            }
        }

        const char *color;
        switch(chimera_set_color_override) {
            case PLAYER_COLOR_WHITE:
                color = localize("chimera_color_white");
                break;
            case PLAYER_COLOR_BLACK:
                color = localize("chimera_color_black");
                break;
            case PLAYER_COLOR_RED:
                color = localize("chimera_color_red");
                break;
            case PLAYER_COLOR_BLUE:
                color = localize("chimera_color_blue");
                break;
            case PLAYER_COLOR_GRAY:
                color = localize("chimera_color_gray");
                break;
            case PLAYER_COLOR_YELLOW:
                color = localize("chimera_color_yellow");
                break;
            case PLAYER_COLOR_GREEN:
                color = localize("chimera_color_green");
                break;
            case PLAYER_COLOR_PINK:
                color = localize("chimera_color_pink");
                break;
            case PLAYER_COLOR_PURPLE:
                color = localize("chimera_color_purple");
                break;
            case PLAYER_COLOR_CYAN:
                color = localize("chimera_color_cyan");
                break;
            case PLAYER_COLOR_COBALT:
                color = localize("chimera_color_cobalt");
                break;
            case PLAYER_COLOR_ORANGE:
                color = localize("chimera_color_orange");
                break;
            case PLAYER_COLOR_TEAL:
                color = localize("chimera_color_teal");
                break;
            case PLAYER_COLOR_SAGE:
                color = localize("chimera_color_sage");
                break;
            case PLAYER_COLOR_BROWN:
                color = localize("chimera_color_brown");
                break;
            case PLAYER_COLOR_TAN:
                color = localize("chimera_color_tan");
                break;
            case PLAYER_COLOR_MAROON:
                color = localize("chimera_color_maroon");
                break;
            case PLAYER_COLOR_SALMON:
                color = localize("chimera_color_salmon");
                break;
            case PLAYER_COLOR_RANDOM:
                color = localize("chimera_color_random");
                break;
            default:
                color = "off";
                break;
        }
        console_output("%s", color);

        return true;
    }
}
