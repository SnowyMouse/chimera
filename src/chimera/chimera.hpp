// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HPP
#define CHIMERA_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include "command/command.hpp"
#include "event/event.hpp"

#define CHIMERA_EXTERN extern "C" __declspec(dllexport)

namespace Chimera {
    /** Language used */
    enum Language {
        /** English language */
        CHIMERA_LANGUAGE_ENGLISH,

        /** Spanish language */
        CHIMERA_LANGUAGE_SPANISH,

        /** The secret language of our ancestors */
        CHIMERA_LANGUAGE_VAP,

        /** Number of languages */
        CHIMERA_LANGUAGE_COUNT
    };

    #define MAX_CHIMERA_MAP_SIZE static_cast<std::size_t>(512 * 1024 * 1024)

    class Signature;
    class Config;
    class Ini;

    class Chimera {
    public:
        /**
         * Check if the given feature is present
         * @param  feature feature to check
         * @return         true if feature is present
         */
        bool feature_present(const char *feature);

        /**
         * Get the missing signatures for a feature
         * @param feature feature that is missing signatures
         */
        std::vector<const char *> missing_signatures_for_feature(const char *feature);

        /**
         * Get the signature, calling std::terminate on failure
         * @param  signature signature name
         * @return           reference to the signature
         */
        Signature &get_signature(const char *signature);

        /**
         * Execute the command
         * @param  command       command name and arguments
         * @param  found_command command if found (unchanged if not)
         * @return               result of command
         */
        CommandResult execute_command(const char *command, const Command **found_command = nullptr, bool saves = false);

        /**
         * Get the config
         * @return config
         */
        Config &get_config() noexcept {
            return *this->p_config;
        }

        /**
         * Get the path to the Chimera profile folder
         * @return path to the Chimera profile folder
         */
        const char *get_path() noexcept;

        /**
         * Reload the configuration
         */
        void reload_config();

        /**
         * Reload the ini
         */
        void reload_ini();

        /**
         * Get the ini data
         */
        const Ini *get_ini() const noexcept;

        /**
         * Get the language used
         * @return language
         */
        Language get_language() const noexcept;

        /**
         * Set the language used
         * @param language new language
         */
        void set_language(Language language) noexcept;

        /**
         * Get a vector of all of the commands
         * @return all of the commands
         */
        const std::vector<Command> &get_commands() const noexcept;

        /**
         * Get a vector of all of the commands
         * @return all of the commands
         */
        std::vector<Command> &get_commands() noexcept;

        /** Construct a Chimera */
        Chimera();

        /** Deconstruct a Chimera */
        ~Chimera() = default;

    private:
        /** Chimera folder path */
        std::string p_path;

        /** Signatures loaded into Chimera */
        std::vector<Signature> p_signatures;

        /** Commands in Chimera */
        std::vector<Command> p_commands;

        /** Config file for Chimera */
        std::unique_ptr<Config> p_config;

        /** Ini file for Chimera */
        std::unique_ptr<Ini> p_ini;

        /** Currently loaded map (if map is compressed) */
        std::byte *p_map_data;

        /** Language used */
        Language p_language = CHIMERA_LANGUAGE_ENGLISH;

        /**
         * Chimera command
         * @param  argc arg count
         * @param  argv arg values
         * @return      true
         */
        static bool chimera_command(int argc, const char **argv) noexcept;

        /**
         * Block unwanted things
         * @param  argc arg count
         * @param  argv arg values
         * @return      true
         */
        static bool block_all_bullshit_command(int argc, const char **argv) noexcept;

        /**
         * Show information for a signature
         * @param  argc arg count
         * @param  argv arg values
         * @return      true
         */
        static bool signature_info_command(int argc, const char **argv) noexcept;

        /**
         * Get all commands
         */
        void get_all_commands() noexcept;
    };

    /**
     * Instantiate the Chimera mod
     */
    extern "C" void instantiate_chimera();

    /**
     * Destroy the Chimera mod
     */
    extern "C" void destroy_chimera();

    /**
     * Check if all features are valid.
     * @return 0 on failure, 1 on success
     */
    extern "C" int find_signatures();

    /**
     * List all missing signatures
     * @return NULL if there are no missing signatures or an error message with all signatures missing
     */
    extern "C" const char *signature_errors();

    /**
     * Determine the type of Halo executable being used
     * @return 0 if not Halo or unknown, 1 if client, 2 if server
     */
    int halo_type();

    #define DEDICATED_SERVER (halo_type() == 2)

    /**
     * Get a reference to the currently loaded Chimera mod
     * @return reference to the currently loaded Chimera mod
     */
    Chimera &get_chimera();

    /**
     * Get the address for a signature. This is intended to be used for assembly.
     * @param signature pointer to location of signature or nullptr
     */
    extern "C" const std::byte *address_for_signature(const char *signature);
}

#endif
