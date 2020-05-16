// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CONFIG_HPP
#define CHIMERA_CONFIG_HPP

#include <vector>
#include <string>

namespace Chimera {
    class Config {
    public:
        /**
         * Get a pointer to the settings for the specified command
         * @param command command to retrieve settings
         * @return        pointer to settings or nullptr if no settings were found
         */
        const std::vector<std::string> *get_settings_for_command(const char *command) const;

        /**
         * Set the settings for the specified command
         * @param command  command to set settings
         * @param settings settings to set to
         */
        void set_settings_for_command(const char *command, const std::vector<std::string> &settings);

        /**
         * Set whether or not to enable saving
         * @param saving set saving
         */
        void set_saving(bool saving) noexcept {
            this->p_saving = saving;
        }

        /** Save */
        void save();

        /** Load */
        void load();

        /**
         * Open the config file
         * @param path path to config file
         */
        Config(const char *path);

    private:
        /** Path of the config file */
        std::string p_path;

        /** Settings (command, settings pair) */
        std::vector<std::pair<std::string, std::vector<std::string>>> p_settings;

        /** Saving is enabled */
        bool p_saving = false;
    };
}

#endif
