// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <memory>
#include "../localization/localization.hpp"
#include "../chimera.hpp"
#include "command.hpp"

namespace Chimera {
    CommandResult Command::call(std::size_t arg_count, const char **args) const noexcept {
        if(!get_chimera().feature_present(this->feature())) {
            return CommandResult::COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE;
        }
        else if(arg_count > this->max_args()) {
            return CommandResult::COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS;
        }
        else if(arg_count < this->min_args()) {
            return CommandResult::COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS;
        }
        else {
            return this->p_function(arg_count, args) ? CommandResult::COMMAND_RESULT_SUCCESS : CommandResult::COMMAND_RESULT_FAILED_ERROR;
        }
    }

    CommandResult Command::call(const std::vector<std::string> &arguments) const noexcept {
        // Get argument count
        std::size_t arg_count = arguments.size();

        // If no arguments were passed, just call it.
        if(arg_count == 0) {
            return this->call(0, nullptr);
        }

        // Make our array
        auto arguments_alloc(std::make_unique<const char *[]>(arg_count));
        for(std::size_t i = 0; i < arg_count; i++) {
            arguments_alloc[i] = arguments[i].data();
        }

        // Do it!
        return this->call(arg_count, arguments_alloc.get());
    }

    std::vector<std::string> split_arguments(const char *command) noexcept {
        // This is the vector to return.
        std::vector<std::string> arguments;

        // This value will be true if we are inside quotes, during which the word will not separate into arguments.
        bool in_quotes = false;

        // If using a backslash, add the next character to the string regardless of what it is.
        bool escape_character = false;

        // Regardless of if there were any characters, there was an argument.
        bool allow_empty_argument = false;

        // Get the command
        std::size_t command_size = std::strlen(command);

        // Get the argument
        std::string argument;
        for(std::size_t i = 0; i < command_size; i++) {
            if(escape_character) {
                escape_character = false;
            }
            // Escape character - this will be used to include the next character regardless of what it is
            else if(command[i] == '\\') {
                escape_character = true;
                continue;
            }
            // If a whitespace or octotothorpe is in quotations in the argument, then it is considered part of the argument.
            else if(command[i] == '"') {
                in_quotes = !in_quotes;
                allow_empty_argument = true;
                continue;
            }
            else if((command[i] == ' ' || command[i] == '\r' || command[i] == '\n' || command[i] == '#') && !in_quotes) {
                // Add argument if not empty.
                if(argument != "" || allow_empty_argument) {
                    arguments.push_back(argument);
                    argument = "";
                    allow_empty_argument = false;
                }

                // Terminate if beginning a comment.
                if(command[i] == '#') {
                    break;
                }
                continue;
            }
            argument += command[i];
        }

        // Add the last argument.
        if(argument != "" || allow_empty_argument) {
            arguments.push_back(argument);
        }

        return arguments;
    }

    std::string unsplit_arguments(const std::vector<std::string> &arguments) noexcept {
        // This is the string to return.
        std::string unsplit;

        for(std::size_t i = 0; i < arguments.size(); i++) {
            // This is a reference to the argument we're dealing with.
            const std::string &argument = arguments[i];

            // This will be the final string we append to the unsplit string.
            std::string argument_final;

            // Set this to true if we need to surround this argument with quotes.
            bool surround_with_quotes = false;

            // Go through each character and add them one-by-one to argument_final.
            for(const char &c : argument) {
                switch(c) {
                    // Backslashes and quotation marks should be escaped.
                    case '\\':
                    case '"':
                        argument_final += '\\';
                        break;

                    // If we're using spaces or octothorpes, the argument should be surrounded with quotation marks. We could escape those, but this is more readable.
                    case '#':
                    case ' ':
                        surround_with_quotes = true;
                        break;

                    default:
                        break;
                }
                argument_final += c;
            }

            if(surround_with_quotes) {
                argument_final = std::string("\"") + argument_final + "\"";
            }

            unsplit += argument_final;

            // Add the space to separate the next argument.
            if(i + 1 < arguments.size()) {
                unsplit += " ";
            }
        }

        return unsplit;
    }

    Command::Command(const char *name, const char *category, const char *feature, const char *help, CommandFunction function, bool autosave, std::size_t min_args, std::size_t max_args) :
        p_name(name), p_category(category), p_feature(feature), p_help(help), p_function(function), p_autosave(autosave), p_min_args(min_args), p_max_args(max_args) {}

    Command::Command(const char *name, const char *category, const char *feature, const char *help, CommandFunction function, bool autosave, std::size_t args) : Command(name, category, feature, help, function, autosave, args, args) {}

    void Chimera::get_all_commands() noexcept {
        this->p_commands.clear();

        #define ADD_COMMAND(name, category, feature, command_fn, autosave, ...) \
            extern bool command_fn(int, const char **); \
            static_assert(autosave == false || autosave == true, "autosave value is not a boolean"); \
            this->p_commands.emplace_back(name, localize(category), feature, localize(name "_command_help"), command_fn, autosave, __VA_ARGS__);

        // Chimera-specific commands
        this->p_commands.emplace_back("chimera", localize("chimera_category_core"), "core", localize("chimera_command_help"), Chimera::chimera_command, false, 0, 1);
        this->p_commands.emplace_back("chimera_signature_info", localize("chimera_category_core"), "core", localize("chimera_signature_info_command_help"), Chimera::signature_info_command, false, 1, 1);
        ADD_COMMAND("chimera_about", "chimera_category_core", "core", about_command, true, 0, 0);
        ADD_COMMAND("chimera_language", "chimera_category_core", "core", language_command, true, 0, 1);
        ADD_COMMAND("chimera_chat_color_help", "chimera_category_custom_chat", "client_custom_chat", chat_color_help_command, true, 0, 1);
        ADD_COMMAND("chimera_chat_block_server_messages", "chimera_category_custom_chat", "client_custom_chat", chat_block_server_messages_command, true, 0, 1);
        ADD_COMMAND("chimera_chat_block_ips", "chimera_category_custom_chat", "client_custom_chat", chat_block_ips_command, true, 0, 1);

        // Debug
        ADD_COMMAND("chimera_budget", "chimera_category_debug", "client", budget_command, true, 0, 1);

        if(this->feature_present("core_devmode_retail")) {
            ADD_COMMAND("chimera_devmode", "chimera_category_debug", "core_devmode_retail", devmode_retail_command, true, 0, 1);
        }
        else {
            ADD_COMMAND("chimera_devmode", "chimera_category_debug", "core_devmode", devmode_command, true, 0, 1);
        }
        ADD_COMMAND("chimera_load_ui_map", "chimera_category_debug", "client", load_ui_map_command, false, 0, 0);
        ADD_COMMAND("chimera_player_info", "chimera_category_debug", "core", player_info_command, false, 0, 1);
        ADD_COMMAND("chimera_apply_damage", "chimera_category_debug", "core", apply_damage_command, false, 2, 5);
        ADD_COMMAND("chimera_block_damage", "chimera_category_debug", "core", block_damage_command, false, 0, 1);
        ADD_COMMAND("chimera_show_coordinates", "chimera_category_debug", "client", show_coordinates_command, true, 0, 1);
        ADD_COMMAND("chimera_show_fps", "chimera_category_debug", "client", show_fps_command, true, 0, 1);
        ADD_COMMAND("chimera_tps", "chimera_category_debug", "core", tps_command, false, 0, 1);
        ADD_COMMAND("chimera_teleport", "chimera_category_debug", "core", teleport_command, false, 1, 4);
        ADD_COMMAND("chimera_script_command_dump", "chimera_category_debug", "core", script_command_dump_command, false, 0, 0);
        ADD_COMMAND("chimera_send_chat_message", "chimera_category_debug", "client", send_chat_message_command, false, 2, 2);
        ADD_COMMAND("chimera_map_info", "chimera_category_debug", "client", map_info_command, false, 0, 0);

        // Enhancements
        this->p_commands.emplace_back("chimera_block_all_bullshit", localize("chimera_category_enhancement"), "client", localize("chimera_block_all_bullshit_help"), Chimera::block_all_bullshit_command, false, 0, 0);
        ADD_COMMAND("chimera_block_buffering", "chimera_category_enhancement", "client_disable_buffering", block_buffering_command, true, 0, 1);
        ADD_COMMAND("chimera_block_extra_weapon", "chimera_category_enhancement", "client_block_extra_weapon", block_extra_weapon_command, false, 0, 0);
        ADD_COMMAND("chimera_unblock_all_extra_weapons", "chimera_category_enhancement", "client_block_extra_weapon", unblock_all_extra_weapons_command, false, 0, 0);
        ADD_COMMAND("chimera_set_name", "chimera_category_enhancement", "client", set_name_command, true, 0, 1);
        ADD_COMMAND("chimera_set_color", "chimera_category_enhancement", "client", set_color_command, true, 0, 1);
        ADD_COMMAND("chimera_throttle_fps", "chimera_category_enhancement", "client", throttle_fps_command, true, 0, 1);
        ADD_COMMAND("chimera_fp_reverb", "chimera_category_enhancement", "client_fp_reverb", fp_reverb_command, true, 0, 1);

        // Server
        ADD_COMMAND("chimera_spectate", "chimera_category_server", "client_spectate", spectate_command, false, 1, 1);
        ADD_COMMAND("chimera_spectate_next", "chimera_category_server", "client_spectate", spectate_next_command, false, 0, 0);
        ADD_COMMAND("chimera_spectate_previous", "chimera_category_server", "client_spectate", spectate_previous_command, false, 0, 0);
        ADD_COMMAND("chimera_spam_to_join", "chimera_category_server", "client", spam_to_join_command, true, 0, 1);
        ADD_COMMAND("chimera_spectate_team_only", "chimera_category_server", "client_spectate", spectate_team_only_command, true, 0, 1);
        ADD_COMMAND("chimera_delete_empty_weapons", "chimera_category_server", "core", delete_empty_weapons_command, true, 0, 1);
        ADD_COMMAND("chimera_player_list", "chimera_category_server", "core", player_list_command, false, 0, 0);
        ADD_COMMAND("chimera_block_equipment_rotation", "chimera_category_server", "core_null_rotation", block_equipment_rotation_command, true, 0, 1);
        ADD_COMMAND("chimera_allow_all_passengers", "chimera_category_server", "core_mtv", allow_all_passengers_command, true, 0, 1);
        ADD_COMMAND("chimera_custom_edition_netcode", "chimera_category_core" /* "chimera_category_server" */, "client_retail", custom_edition_netcode_command, true, 0, 1);

        // Visuals
        ADD_COMMAND("chimera_af", "chimera_category_visual", "client_af", af_command, true, 0, 1);
        ADD_COMMAND("chimera_block_auto_center", "chimera_category_visual", "client", block_auto_center_command, true, 0, 1);
        ADD_COMMAND("chimera_block_gametype_indicator", "chimera_category_visual", "client_gametype_indicator", block_gametype_indicator_command, true, 0, 1);
        ADD_COMMAND("chimera_block_gametype_rules", "chimera_category_visual", "client_gametype_rules", block_gametype_rules_command, true, 0, 1);
        ADD_COMMAND("chimera_block_hold_f1", "chimera_category_visual", "client_hold_f1", block_hold_f1_command, true, 0, 1);
        ADD_COMMAND("chimera_block_letterbox", "chimera_category_visual", "client_letterbox", block_letterbox_command, true, 0, 1);
        ADD_COMMAND("chimera_block_loading_screen", "chimera_category_visual", "client_loading_screen", block_loading_screen_command, true, 0, 1);
        ADD_COMMAND("chimera_block_server_ip", "chimera_category_visual", "client_server_ip", block_server_ip_command, true, 0, 1);
        ADD_COMMAND("chimera_block_zoom_blur", "chimera_category_visual", "client_zoom_blur", block_zoom_blur_command, true, 0, 1);
        ADD_COMMAND("chimera_console_prompt_color", "chimera_category_visual", "client_console_prompt_color", console_prompt_color_command, true, 0, 3);
        ADD_COMMAND("chimera_fov", "chimera_category_visual", "client", fov_command, true, 0, 1);
        ADD_COMMAND("chimera_fov_vehicle", "chimera_category_visual", "client", fov_vehicle_command, true, 0, 1);
        ADD_COMMAND("chimera_fov_cinematic", "chimera_category_visual", "client", fov_cinematic_command, true, 0, 1);
        ADD_COMMAND("chimera_interpolate", "chimera_category_visual", "client", interpolate_command, true, 0, 1);
        ADD_COMMAND("chimera_model_detail", "chimera_category_visual", "client_lod", model_detail_command, true, 0, 1);
        ADD_COMMAND("chimera_shrink_empty_weapons", "chimera_category_visual", "client", shrink_empty_weapons_command, true, 0, 1);
        ADD_COMMAND("chimera_simple_score_screen", "chimera_category_visual", "client_score_screen", simple_score_screen_command, true, 0, 1);
        ADD_COMMAND("chimera_split_screen_hud", "chimera_category_visual", "client_split_screen_hud", split_screen_hud_command, true, 0, 1);
        ADD_COMMAND("chimera_widescreen_fix", "chimera_category_visual", "client_widescreen", widescreen_fix_command, true, 0, 1);
        ADD_COMMAND("chimera_uncap_cinematic", "chimera_category_visual", "client_interpolate", uncap_cinematic_command, true, 0, 1);
        // ADD_COMMAND("chimera_meme_zone", "chimera_category_visual", "client_widescreen", meme_zone_command, true, 0, 1);

        // Lua
        ADD_COMMAND("chimera_reload_scripts", "chimera_category_lua", "client", reload_scripts_command, false, 0, 0);

        // Mouse
        ADD_COMMAND("chimera_block_mouse_acceleration", "chimera_category_mouse", "client_mouse_acceleration", block_mouse_acceleration_command, true, 0, 1);
        ADD_COMMAND("chimera_mouse_sensitivity", "chimera_category_mouse", "client_mouse_sensitivity", mouse_sensitivity_command, true, 0, 2);

        // Controller
        ADD_COMMAND("chimera_aim_assist", "chimera_category_controller", "client", aim_assist_command, true, 0, 1);
        ADD_COMMAND("chimera_auto_uncrouch", "chimera_category_controller", "client_auto_uncrouch", auto_uncrouch_command, true, 0, 1);
        ADD_COMMAND("chimera_diagonals", "chimera_category_controller", "client_diagonals", diagonals_command, true, 0, 1);
        ADD_COMMAND("chimera_deadzones", "chimera_category_controller", "client_deadzones", deadzones_command, true, 0, 1);
        ADD_COMMAND("chimera_block_button_quotes", "chimera_category_controller", "client_quote_prompt", block_button_quotes_command, true, 0, 1);

        // Bookmark
        ADD_COMMAND("chimera_bookmark_list", "chimera_category_bookmark", "client", bookmark_list_command, false, 0, 0);
        ADD_COMMAND("chimera_bookmark_add", "chimera_category_bookmark", "client", bookmark_add_command, false, 0, 2);
        ADD_COMMAND("chimera_bookmark_connect", "chimera_category_bookmark", "client", bookmark_connect_command, false, 1, 1);
        ADD_COMMAND("chimera_bookmark_delete", "chimera_category_bookmark", "client", bookmark_delete_command, false, 0, 1);
        ADD_COMMAND("chimera_history_list", "chimera_category_bookmark", "client", history_list_command, false, 0, 0);
        ADD_COMMAND("chimera_history_connect", "chimera_category_bookmark", "client", history_connect_command, false, 1, 1);
    }
}
