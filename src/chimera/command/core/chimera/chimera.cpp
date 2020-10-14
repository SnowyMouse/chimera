// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../lua/lua_script.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../version.hpp"
#include "../../../config/config.hpp"
#include "../../command.hpp"

namespace Chimera {
    bool Chimera::chimera_command(int argc, const char **argv) noexcept {
        if(argc == 0) {
            // Get all categories to show to the user
            std::vector<std::string> categories;
            auto &chimera = get_chimera();
            for(auto &cmd : chimera.p_commands) {
                // Ignore commands that cannot be used
                if(!chimera.feature_present(cmd.feature())) {
                    continue;
                }

                bool found = false;
                auto *cmd_cat = cmd.category();
                for(auto &category : categories) {
                    if(std::strcmp(category.data(),cmd_cat) == 0) {
                        found = true;
                        break;
                    }
                }

                // Add it alphabetically
                if(!found) {
                    bool added = false;
                    for(auto &category : categories) {
                        if(std::strcmp(category.data(),cmd_cat) > 0) {
                            added = true;
                            categories.insert(categories.begin() + (&category - categories.data()), cmd_cat);
                            break;
                        }
                    }
                    if(!added) {
                        categories.push_back(cmd_cat);
                    }
                }
            }

            ConsoleColor blue_color = { 1.0, 0.3, 0.6, 1 };

            // Show everything
            console_output(ConsoleColor::header_color(), localize("chimera_command_version"), CHIMERA_VERSION_STRING);
            console_output(blue_color, "Lua API version %.3f (based on version 2.042 from build -572)", CHIMERA_LUA_VERSION);
            console_output(ConsoleColor::header_color(), localize("chimera_command_categories"));
            for(auto &category : categories) {
                console_output(ConsoleColor::body_color(), "  - %s", category.data());
            }
        }
        else if(argc == 1) {
            auto &chimera = get_chimera();
            const char *category = argv[0];

            // Maybe it's a category? See if we can find at least one command with the category.
            for(auto &cmd : chimera.p_commands) {
                if(std::strcmp(cmd.category(), category) == 0) {
                    // Get all commands
                    std::vector<Command *> commands;
                    for(auto &cmd : chimera.p_commands) {
                        // Ignore commands that cannot be used
                        if(!chimera.feature_present(cmd.feature())) {
                            continue;
                        }

                        if(std::strcmp(cmd.category(), category) == 0) {
                            bool added = false;
                            const auto *cmd_name = cmd.name();
                            for(auto *&command : commands) {
                                if(std::strcmp(command->name(), cmd_name) > 0) {
                                    added = true;
                                    commands.insert(commands.begin() + (&command - commands.data()), &cmd);
                                    break;
                                }
                            }
                            if(!added) {
                                commands.push_back(&cmd);
                            }
                        }
                    }

                    // Show all commands
                    console_output(ConsoleColor::header_color(), localize("chimera_command_commands_in_category"), category);

                    if(commands.size() == 0) {
                        console_output(ConsoleColor::body_color(), localize("chimera_command_no_commands_available_in_category"));
                    }
                    else {
                        for(auto *cmd : commands) {
                            // See if we have something set for the command
                            const auto *settings_for_cmd = chimera.p_config ? chimera.p_config->get_settings_for_command(cmd->name()) : nullptr;
                            std::string settings_for_cmd_str;

                            if(settings_for_cmd && settings_for_cmd->size()) {
                                settings_for_cmd_str += " [";
                                settings_for_cmd_str += "\"" + (*settings_for_cmd)[0] + "\"";
                                for(std::size_t i = 1; i < settings_for_cmd->size(); i++) {
                                    settings_for_cmd_str += " \"" + (*settings_for_cmd)[i] + "\"";
                                }
                                settings_for_cmd_str += "]";
                            }

                            // Show it
                            console_output(ConsoleColor::body_color(), "  - %s%s", cmd->name(), settings_for_cmd_str.data());
                        }
                    }


                    return true;
                }
            }

            // Maybe we're looking for a command?
            char command[256];
            if(std::strncmp(category, "chimera_", 8) != 0) {
                std::snprintf(command, sizeof(command), "chimera_%s", category);
            }

            // Go through commands
            for(auto &cmd : chimera.p_commands) {
                if(std::strcmp(cmd.name(), command) == 0) {
                    char help[256] = {};
                    std::size_t help_i = 0;
                    const char *cmd_help = cmd.help();

                    console_output(ConsoleColor::header_color(), localize("chimera_command_info_for_command"), cmd.name());

                    while(*cmd_help) {
                        char hc = *(cmd_help++);
                        if(hc == '\n') {
                            help[help_i] = 0;
                            console_output(ConsoleColor::body_color(), "  > %s", help);
                            help_i = 0;
                        }
                        else {
                            help[help_i++] = hc;
                        }
                    }

                    if(help_i != 0) {
                        help[help_i] = 0;
                        console_output(ConsoleColor::body_color(), "  > %s", help);
                    }

                    return true;
                }
            }

            // Well, we tried
            console_error(localize("chimera_command_error_no_such_category_or_command"), category);
        }
        return true;
    }
}
