// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <filesystem>
#include <fstream>
#include "../localization/localization.hpp"
#include "../version.hpp"
#include "../command/command.hpp"
#include "../output/output.hpp"
#include "../chimera.hpp"
#include "config.hpp"

namespace Chimera {
    static int egg_count = 0;
    void disable_easter_eggs() {
        egg_count--;
    }
    void enable_easter_eggs() {
        egg_count++;
    }

    const std::vector<std::string> *Config::get_settings_for_command(const char *command) const {
        for(auto &c : this->p_settings) {
            if(std::strcmp(c.first.data(), command) == 0) {
                return &c.second;
            }
        }
        return nullptr;
    }

    void Config::set_settings_for_command(const char *command, const std::vector<std::string> &settings) {
        for(auto &c : this->p_settings) {
            if(std::strcmp(c.first.data(), command) == 0) {
                if(c.second == settings) {
                    return;
                }
                this->p_settings.erase(this->p_settings.begin() + (&c - this->p_settings.data()));
                break;
            }
        }
        this->p_settings.emplace_back(command, settings);

        if(this->p_saving) {
            this->save();
        }
    }

    void Config::save() {
        // Open
        std::ofstream config(this->p_path, std::ios_base::out | std::ios_base::trunc);

        // Set header
        config << "################################################################################\n";
        config << "#                                                                              #\n";

        // Here's a thing
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);

        // Make our line thingy
        static constexpr const std::size_t LEN = 81;
        char saved_with_line[LEN];
        std::memset(saved_with_line, ' ', sizeof(saved_with_line));
        char chimera_version[] = "#   Chimera version " CHIMERA_VERSION_STRING;
        static_assert(sizeof(chimera_version) < LEN / 2, "out-of-bounds for chimera_version size");
        std::memcpy(saved_with_line, chimera_version, sizeof(chimera_version) - 1);

        // Randomly select a line
        const char *random_text;
        auto meme = pc.LowPart % 20;

        if(egg_count != 0) {
            random_text = "it's broken now";
        }
        else {
            if(meme == 9 || meme == 2) {
                random_text = "Chu! Chu! Chu!!!!";
            }
            else if(meme == 10 || meme == 3) {
                random_text = "Vap! Vap! Vap!~";
            }
            else if(meme == 11) {
                random_text = "HELP I'M TRAPPED IN A FILE";
            }
            else {
                random_text = "by Snowy ^.^";
            }
        }


        std::snprintf(saved_with_line + LEN/2, sizeof(saved_with_line) - LEN/2, "%36s   #", random_text);

        config << saved_with_line << "\n";
        config << "# ---------------------------------------------------------------------------- #\n";
        config << localize("chimera_config_note") << "\n";
        config << "#                                                                              #\n";
        config << "################################################################################\n\n";

        // Set settings
        for(auto &c : this->p_settings) {
            config << unsplit_arguments(split_arguments(c.first.data())) << " " << unsplit_arguments(c.second) << "\n";
        }

        // Flush
        config.flush();
        config.close();
    }

    void Config::load() {
        // Open
        std::ifstream config(this->p_path, std::ios_base::in);
        std::string line;
        std::size_t line_count = 0;
        while(std::getline(config, line)) {
            line_count++;
            #define SHOW_ERROR(...) console_error(localize("chimera_error_config"), this->p_path.data()); \
                                    console_error("    %zu: %s", line_count, line.data()); \
                                    console_error(__VA_ARGS__);

            const Command *command;
            switch(get_chimera().execute_command(line.data(), &command, true)) {
                case CommandResult::COMMAND_RESULT_SUCCESS:
                    break;
                case CommandResult::COMMAND_RESULT_FAILED_ERROR_NOT_FOUND:
                    if(split_arguments(line.data()).size()) {
                        SHOW_ERROR(localize("chimera_error_command_not_found"));
                    }
                    break;
                case CommandResult::COMMAND_RESULT_FAILED_ERROR:
                    SHOW_ERROR(localize("chimera_error_command_error"), command->name());
                    break;
                case CommandResult::COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE:
                    SHOW_ERROR(localize("chimera_error_command_unavailable"), command->name(), command->feature());
                    break;
                case CommandResult::COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS:
                    SHOW_ERROR(localize("chimera_error_not_enough_arguments"), command->name(), command->min_args());
                    break;
                case CommandResult::COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS:
                    SHOW_ERROR(localize("chimera_error_too_many_arguments"), command->name(), command->max_args());
                    break;
            }
        }
        this->p_saving = true;
    }

    Config::Config(const char *path) : p_path(path) {}
    Config::Config(const std::filesystem::path path) : p_path(path.string()) {}
}
