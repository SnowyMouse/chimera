// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INI_HPP
#define CHIMERA_INI_HPP

#include <variant>
#include <string>
#include <vector>
#include <utility>
#include <istream>
#include <optional>

namespace Chimera {
    class Ini {
    public:
        /**
         * Get the value by name
         * @param  key name of the value
         * @return     pointer to the value if found, or nullptr if not
         */
        const char *get_value(const char *key) const noexcept;

        /**
         * Get the value by name
         * @param  key name of the value
         * @return     boolean value of the key or nullopt if not set
         */
        std::optional<bool> get_value_bool(const char *key) const noexcept;

        /**
         * Get the value by name
         * @param  key name of the value
         * @return     boolean value of the key or nullopt if not set
         */
        std::optional<double> get_value_float(const char *key) const noexcept;

        /**
         * Get the value by name
         * @param  key name of the value
         * @return     boolean value of the key or nullopt if not set
         */
        std::optional<long> get_value_long(const char *key) const noexcept;

        /**
         * Get the value by name
         * @param  key name of the value
         * @return     boolean value of the key or nullopt if not set
         */
        std::optional<unsigned long long> get_value_size(const char *key) const noexcept;

        /**
         * Set the value
         * @param key        name of the value
         * @param new_value  new value to set to
         */
        void set_value(const char *key, const char *new_value) noexcept;

        /**
         * Set the value
         * @param key        name of the value
         * @param new_value  new value to set to
         */
        void set_value(std::pair<std::string, std::string> key_value) noexcept;

        /**
         * Delete the value
         * @param key name of the value
         */
        void delete_value(const char *key) noexcept;

        /**
         * Initialize an Ini from a path
         * @param path path to initialize from
         */
        Ini(const char *path);

        /**
         * Initialize an Ini from a stream
         * @param data data to initialize from
         */
        Ini(std::istream &stream);

        /**
         * Initialize an empty Ini.
         */
        Ini() = default;

        /**
         * Copy an Ini
         * @param copy ini file to copy
         */
        Ini(const Ini &copy) = default;

        /**
         * Move an Ini
         * @param move ini file to move from
         */
        Ini(Ini &&move) = default;

    private:
        /** Values of the Ini */
        std::vector<std::pair<std::string, std::string>> p_values;

        /**
         * Load from the stream
         * @param stream stream to load from
         */
        void load_from_stream(std::istream &stream);
    };
}

#endif
