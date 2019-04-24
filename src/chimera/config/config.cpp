#include <cstring>
#include <fstream>
#include "../localization/localization.hpp"
#include "../version.hpp"
#include "../command/command.hpp"
#include "../output/output.hpp"
#include "../chimera.hpp"
#include "config.hpp"

namespace Chimera {
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

        char saved_with_line[81];
        switch(pc.LowPart % 10) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            std::snprintf(saved_with_line, sizeof(saved_with_line), "#         Chimera version %-28s by Kavawuvi ^v^         #", CHIMERA_VERSION_STRING);
            break;

            case 7:
            case 8:
            std::snprintf(saved_with_line, sizeof(saved_with_line), "#         Chimera version %-28s Vap! Vap! Vap!~         #", CHIMERA_VERSION_STRING);
            break;

            case 9:
            std::snprintf(saved_with_line, sizeof(saved_with_line), "#         Chimera version %-17s HELP I'M TRAPPED IN A FILE         #", CHIMERA_VERSION_STRING);
            break;
        }
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
}
