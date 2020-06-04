// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../event/frame.hpp"
#include "../halo_data/keyboard.hpp"
#include "../output/output.hpp"
#include "../localization/localization.hpp"
#include "../halo_data/script.hpp"
#include "hotkey.hpp"

namespace Chimera {
    struct Hotkey {
        bool alt_held = false;
        bool ctrl_held = false;
        bool shift_held = false;
        std::string key;
        std::string command;
    };

    static std::vector<Hotkey> hotkeys;
    static const char *key = nullptr;

    static void on_frame() noexcept {
        const char *current_key = nullptr;
        #define KEY_STR(key) # key
        #define SET_KEY_IF_HELD(key) if(keys.key) { current_key = KEY_STR(key); }
        auto &keys = get_keyboard_keys();

        SET_KEY_IF_HELD(f1)
        else SET_KEY_IF_HELD(f2)
        else SET_KEY_IF_HELD(f3)
        else SET_KEY_IF_HELD(f4)
        else SET_KEY_IF_HELD(f5)
        else SET_KEY_IF_HELD(f5)
        else SET_KEY_IF_HELD(f6)
        else SET_KEY_IF_HELD(f7)
        else SET_KEY_IF_HELD(f8)
        else SET_KEY_IF_HELD(f9)
        else SET_KEY_IF_HELD(f10)
        else SET_KEY_IF_HELD(f11)
        else SET_KEY_IF_HELD(f12)

        #undef SET_KEY_IF_HELD
        #define SET_KEY_IF_HELD(key, str) if(keys.key) { current_key = str; }

        else SET_KEY_IF_HELD(top_0, "0")
        else SET_KEY_IF_HELD(top_1, "1")
        else SET_KEY_IF_HELD(top_2, "2")
        else SET_KEY_IF_HELD(top_3, "3")
        else SET_KEY_IF_HELD(top_4, "4")
        else SET_KEY_IF_HELD(top_5, "5")
        else SET_KEY_IF_HELD(top_6, "6")
        else SET_KEY_IF_HELD(top_7, "7")
        else SET_KEY_IF_HELD(top_8, "8")
        else SET_KEY_IF_HELD(top_9, "9")

        else SET_KEY_IF_HELD(num_0, "num_0")
        else SET_KEY_IF_HELD(num_1, "num_1")
        else SET_KEY_IF_HELD(num_2, "num_2")
        else SET_KEY_IF_HELD(num_3, "num_3")
        else SET_KEY_IF_HELD(num_4, "num_4")
        else SET_KEY_IF_HELD(num_5, "num_5")
        else SET_KEY_IF_HELD(num_6, "num_6")
        else SET_KEY_IF_HELD(num_7, "num_7")
        else SET_KEY_IF_HELD(num_8, "num_8")
        else SET_KEY_IF_HELD(num_9, "num_9")

        #undef SET_KEY_IF_HELD
        #undef KEY_STR

        // If we let go of a key?
        if(key && !current_key) {
            key = nullptr;
        }
        // If we're holding a key?
        else if(!key && current_key) {
            key = current_key;
            bool ctrl = keys.left_control || keys.right_control;
            bool alt = keys.left_alt || keys.right_alt;
            bool shift = keys.left_shift || keys.right_shift;
            for(auto &h : hotkeys) {
                if(h.key == key && h.ctrl_held == ctrl && h.alt_held == alt && h.shift_held == shift) {
                    if(std::strncmp(h.command.data(), "chimera", strlen("chimera")) == 0) {
                        const Command *found_command;
                        switch(get_chimera().execute_command(h.command.data(), &found_command)) {
                            case COMMAND_RESULT_SUCCESS:
                            case COMMAND_RESULT_FAILED_ERROR:
                                break;
                            case COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE:
                                console_error(localize("chimera_error_command_unavailable"), found_command->name(), found_command->feature());
                                break;
                            case COMMAND_RESULT_FAILED_ERROR_NOT_FOUND:
                                console_error(localize("chimera_error_command_not_found"));
                                break;
                            case COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS:
                                console_error(localize("chimera_error_not_enough_arguments"), found_command->name(), found_command->min_args());
                                break;
                            case COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS:
                                console_error(localize("chimera_error_too_many_arguments"), found_command->name(), found_command->max_args());
                                break;
                        }
                    }
                    else {
                        execute_script(h.command.data());
                    }
                    break;
                }
            }
        }
    }

    void set_up_hotkeys() noexcept {
        auto *ini = get_chimera().get_ini();
        auto *enable_hotkeys = ini->get_value("hotkey.enabled");
        if(!enable_hotkeys || *enable_hotkeys != '1') {
            return;
        }
        hotkeys.clear();

        add_preframe_event(on_frame);

        #define ADD_HOTKEY(ctrl, alt, shift, key_str, setting_str) { \
            auto *key_ini = ini->get_value("hotkey." setting_str); \
            if(key_ini && *key_ini) { \
                auto &key_adding = hotkeys.emplace_back(); \
                key_adding.ctrl_held = ctrl; \
                key_adding.shift_held = shift; \
                key_adding.alt_held = alt; \
                key_adding.key = key_str; \
                key_adding.command = key_ini; \
            } \
        }

        #define ADD_BASIC_HOTKEY(key) ADD_HOTKEY(false, false, false, key, key)
        #define ADD_ALT_HOTKEY(key) ADD_HOTKEY(false, true, false, key, "alt_" key)
        #define ADD_CTRL_HOTKEY(key) ADD_HOTKEY(true, false, false, key, "ctrl_" key)
        #define ADD_ALT_SHIFT_HOTKEY(key) ADD_HOTKEY(false, true, true, key, "alt_shift_" key)
        #define ADD_CTRL_ALT_SHIFT_HOTKEY(key) ADD_HOTKEY(true, true, true, key, "ctrl_alt_shift_" key)

        ADD_BASIC_HOTKEY("f1")
        ADD_BASIC_HOTKEY("f2")
        ADD_BASIC_HOTKEY("f3")
        ADD_BASIC_HOTKEY("f4")
        ADD_BASIC_HOTKEY("f5")
        ADD_BASIC_HOTKEY("f6")
        ADD_BASIC_HOTKEY("f7")
        ADD_BASIC_HOTKEY("f8")
        ADD_BASIC_HOTKEY("f9")
        ADD_BASIC_HOTKEY("f10")
        ADD_BASIC_HOTKEY("f11")
        ADD_BASIC_HOTKEY("f12")

        ADD_ALT_HOTKEY("1")
        ADD_ALT_HOTKEY("2")
        ADD_ALT_HOTKEY("3")
        ADD_ALT_HOTKEY("4")
        ADD_ALT_HOTKEY("5")
        ADD_ALT_HOTKEY("6")
        ADD_ALT_HOTKEY("7")
        ADD_ALT_HOTKEY("8")
        ADD_ALT_HOTKEY("9")
        ADD_ALT_HOTKEY("0")

        ADD_ALT_SHIFT_HOTKEY("1")
        ADD_ALT_SHIFT_HOTKEY("2")
        ADD_ALT_SHIFT_HOTKEY("3")
        ADD_ALT_SHIFT_HOTKEY("4")
        ADD_ALT_SHIFT_HOTKEY("5")
        ADD_ALT_SHIFT_HOTKEY("6")
        ADD_ALT_SHIFT_HOTKEY("7")
        ADD_ALT_SHIFT_HOTKEY("8")
        ADD_ALT_SHIFT_HOTKEY("9")
        ADD_ALT_SHIFT_HOTKEY("0")

        ADD_CTRL_HOTKEY("1")
        ADD_CTRL_HOTKEY("2")
        ADD_CTRL_HOTKEY("3")
        ADD_CTRL_HOTKEY("4")
        ADD_CTRL_HOTKEY("5")
        ADD_CTRL_HOTKEY("6")
        ADD_CTRL_HOTKEY("7")
        ADD_CTRL_HOTKEY("8")
        ADD_CTRL_HOTKEY("9")
        ADD_CTRL_HOTKEY("0")

        ADD_CTRL_ALT_SHIFT_HOTKEY("1")
        ADD_CTRL_ALT_SHIFT_HOTKEY("2")
        ADD_CTRL_ALT_SHIFT_HOTKEY("3")
        ADD_CTRL_ALT_SHIFT_HOTKEY("4")
        ADD_CTRL_ALT_SHIFT_HOTKEY("5")
        ADD_CTRL_ALT_SHIFT_HOTKEY("6")
        ADD_CTRL_ALT_SHIFT_HOTKEY("7")
        ADD_CTRL_ALT_SHIFT_HOTKEY("8")
        ADD_CTRL_ALT_SHIFT_HOTKEY("9")
        ADD_CTRL_ALT_SHIFT_HOTKEY("0")

        ADD_ALT_HOTKEY("num_1")
        ADD_ALT_HOTKEY("num_2")
        ADD_ALT_HOTKEY("num_3")
        ADD_ALT_HOTKEY("num_4")
        ADD_ALT_HOTKEY("num_5")
        ADD_ALT_HOTKEY("num_6")
        ADD_ALT_HOTKEY("num_7")
        ADD_ALT_HOTKEY("num_8")
        ADD_ALT_HOTKEY("num_9")
        ADD_ALT_HOTKEY("num_0")

        ADD_ALT_SHIFT_HOTKEY("num_1")
        ADD_ALT_SHIFT_HOTKEY("num_2")
        ADD_ALT_SHIFT_HOTKEY("num_3")
        ADD_ALT_SHIFT_HOTKEY("num_4")
        ADD_ALT_SHIFT_HOTKEY("num_5")
        ADD_ALT_SHIFT_HOTKEY("num_6")
        ADD_ALT_SHIFT_HOTKEY("num_7")
        ADD_ALT_SHIFT_HOTKEY("num_8")
        ADD_ALT_SHIFT_HOTKEY("num_9")
        ADD_ALT_SHIFT_HOTKEY("num_0")

        ADD_CTRL_HOTKEY("num_1")
        ADD_CTRL_HOTKEY("num_2")
        ADD_CTRL_HOTKEY("num_3")
        ADD_CTRL_HOTKEY("num_4")
        ADD_CTRL_HOTKEY("num_5")
        ADD_CTRL_HOTKEY("num_6")
        ADD_CTRL_HOTKEY("num_7")
        ADD_CTRL_HOTKEY("num_8")
        ADD_CTRL_HOTKEY("num_9")
        ADD_CTRL_HOTKEY("num_0")

        ADD_CTRL_ALT_SHIFT_HOTKEY("num_1")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_2")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_3")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_4")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_5")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_6")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_7")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_8")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_9")
        ADD_CTRL_ALT_SHIFT_HOTKEY("num_0")
    }
}
