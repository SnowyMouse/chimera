// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_COMMAND_HPP
#define CHIMERA_COMMAND_HPP

#include <cstring>
#include <vector>
#include <string>

namespace Chimera {
    #define BOOL_TO_STR(boolean) (boolean ? "true" : "false")
    #define STR_TO_BOOL(str) (std::strcmp(str, "1") == 0 || std::strcmp(str, "true") == 0)

    /**
     * Result of a command
     */
    enum CommandResult {
        /** Command was a success. If invoked by console and the command can save, save here. */
        COMMAND_RESULT_SUCCESS = 0,

        /** Command failed. Do not save. */
        COMMAND_RESULT_FAILED_ERROR,

        /** Command not found. Do not save. */
        COMMAND_RESULT_FAILED_ERROR_NOT_FOUND,

        /** Command requires more arguments than given. Do not save. */
        COMMAND_RESULT_FAILED_NOT_ENOUGH_ARGUMENTS,

        /** Command requires fewer arguments than given. Do not save. */
        COMMAND_RESULT_FAILED_TOO_MANY_ARGUMENTS,

        /** Command feature is not available on this version of Halo CE. Do not save. */
        COMMAND_RESULT_FAILED_FEATURE_NOT_AVAILABLE
    };

    /**
     * Function type for a command
     * @param arg_count Number of arguments
     * @param args      Arguments given; not valid if arg_count is 0
     * @return          true if success; false if failure
     */
    using CommandFunction = bool (*)(int arg_count, const char **args);

    /**
     * Command the user can call to execute functions
     */
    class Command {
    public:
        /**
         * Get the name of the command
         * @return pointer to the name of the command
         */
        const char *name() const noexcept {
            return this->p_name;
        }

        /**
         * Get the feature of the command
         * @return pointer to the feature of the command
         */
        const char *feature() const noexcept {
            return this->p_feature;
        }

        /**
         * Get the category of the command
         * @return pointer to the category of the command
         */
        const char *category() const noexcept {
            return this->p_category;
        }

        /**
         * Get the help of the command
         * @return pointer to the help of the command
         */
        const char *help() const noexcept {
            return this->p_help;
        }

        /**
         * Get the minimum arguments of the command
         * @return minimum arguments
         */
        std::size_t min_args() const noexcept {
            return this->p_min_args;
        }

        /**
         * Get the maximum arguments of the command
         * @return maximum arguments
         */
        std::size_t max_args() const noexcept {
            return this->p_max_args;
        }

        /**
         * Return whether the command automatically saves
         * @return true if command should automatically save
         */
        bool autosave() const noexcept {
            return this->p_autosave;
        }

        /**
         * Call the function with the given arguments
         * @param  arg_count number of arguments to pass
         * @param  args      array of arguments
         * @return           result of command
         */
        CommandResult call(std::size_t arg_count, const char **args) const noexcept;

        /**
         * Call the function with the given arguments
         * @param  args array of arguments
         * @return      result of command
         */
        CommandResult call(const std::vector<std::string> &arguments) const noexcept;

        /**
         * Instantiate a command
         * @param name     name of the command
         * @param category category of the command
         * @param feature  feature of the command
         * @param help     help info of the command
         * @param function function pointer of the command
         * @param autosave auto saves if successful and at least 1 arg was passed
         * @param min_args minimum arguments
         * @param max_args maximum arguments
         */
        Command(const char *name, const char *category, const char *feature, const char *help, CommandFunction function, bool autosave, std::size_t min_args, std::size_t max_args);

        /**
         * Instantiate a command
         * @param name     name of the command
         * @param category category of the command
         * @param feature  feature of the command
         * @param help     help info of the command
         * @param function function pointer of the command
         * @param autosave auto saves if successful and at least 1 arg was passed
         * @param args     required number of arguments
         */
        Command(const char *name, const char *category, const char *feature, const char *help, CommandFunction function, bool autosave, std::size_t args = 0);

    private:
        /** Name of the command */
        const char *p_name;

        /** Category of the command */
        const char *p_category;

        /** Feature required for the command */
        const char *p_feature;

        /** Help of the command */
        const char *p_help;

        /** Function to call for the command */
        CommandFunction p_function;

        /** Command automatically saves */
        bool p_autosave;

        /** Minimum required arguments for the command */
        std::size_t p_min_args;

        /** Maximum required arguments for the command */
        std::size_t p_max_args;
    };

    /**
     * Split the arguments
     * @param command arguments to split
     * @return        vector of split arguments
     */
    std::vector<std::string> split_arguments(const char *command) noexcept;

    /**
     * Unsplit the arguments
     * @param  arguments arguments to unsplit
     * @return           combined arguments
     */
    std::string unsplit_arguments(const std::vector<std::string> &arguments) noexcept;
}

#endif
