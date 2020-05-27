#include <fstream>
#include <windows.h>
#include "ini.hpp"
#include "../command/command.hpp"
#include <cstring>

namespace Chimera {
    const char *Ini::get_value(const char *key) const noexcept {
        for(auto &i : this->p_values) {
            if(i.first == key) {
                return i.second.data();
            }
        }
        return nullptr;
    }

    std::optional<bool> Ini::get_value_bool(const char *key) const noexcept {
        auto *v = this->get_value(key);
        if(!v) {
            return std::nullopt;
        }
        return STR_TO_BOOL(v);
    }

    std::optional<double> Ini::get_value_float(const char *key) const noexcept {
        auto *v = this->get_value(key);
        if(!v) {
            return std::nullopt;
        }
        try {
            return std::stod(v);
        }
        catch(std::exception &) {
            char error[512];
            std::snprintf(error, sizeof(error), "%s (=> %s) is not a valid real number", key, v);
            MessageBox(nullptr, error, "Can't read INI value", 0);
            std::terminate();
        }
    }

    std::optional<long> Ini::get_value_long(const char *key) const noexcept {
        auto *v = this->get_value(key);
        if(!v) {
            return std::nullopt;
        }
        try {
            return std::stol(v);
        }
        catch(std::exception &) {
            char error[512];
            std::snprintf(error, sizeof(error), "%s (=> %s) is not a valid integer or is out of range (%li - %li)", key, v, LONG_MIN, LONG_MAX);
            MessageBox(nullptr, error, "Can't read INI value", 0);
            std::terminate();
        }
    }

    std::optional<unsigned long long> Ini::get_value_size(const char *key) const noexcept {
        auto *v = this->get_value(key);
        if(!v) {
            return std::nullopt;
        }
        try {
            return std::stoull(v);
        }
        catch(std::exception &) {
            char error[512];
            std::snprintf(error, sizeof(error), "%s (=> %s) is not a valid integer or is out of range (0 - %llu)", key, v, ULONG_LONG_MAX);
            MessageBox(nullptr, error, "Can't read INI value", 0);
            std::terminate();
        }
    }

    void Ini::set_value(const char *key, const char *value) noexcept {
        for(auto &i : this->p_values) {
            if(i.first == key) {
                i.second = value;
                break;
            }
        }
        this->p_values.emplace_back(key, value);
    }

    void Ini::set_value(std::pair<std::string, std::string> key_value) noexcept {
        for(auto &i : this->p_values) {
            if(i.first == key_value.first) {
                i.second = std::move(key_value.second);
                break;
            }
        }
        this->p_values.emplace_back(key_value);
    }

    void Ini::delete_value(const char *key) noexcept {
        for(auto &i : this->p_values) {
            if(i.first == key) {
                this->p_values.erase(this->p_values.begin() + (&i - this->p_values.data()));
                break;
            }
        }
    }

    // Returns false on error, a key/value pair, or true to do nothing
    static std::variant<std::pair<std::string, std::string>, bool> digest_line(const char *data, const char **new_offset, std::string &current_group) {
        // Determine how big the line is
        std::size_t line_length = 0;
        bool non_whitespace = false;
        std::size_t equals_offset = 0;
        std::size_t right_square_bracket_offset = 0;
        while(data[line_length] && data[line_length] != '\r' && data[line_length] != '\n') {
            if(!non_whitespace && data[line_length] != ' ' && data[line_length] != '\t') {
                non_whitespace = true;
            }
            if(!equals_offset && data[line_length] == '=') {
                equals_offset = line_length;
            }
            if(!right_square_bracket_offset && data[line_length] == ']') {
                right_square_bracket_offset = line_length;
            }
            line_length++;
        }

        // Set new_offset, making sure it doesn't start on a line break
        if(new_offset) {
            *new_offset = data + line_length;
            while(**new_offset == '\r' || **new_offset == '\n') {
                (*new_offset)++;
            }
        }

        // If there's nothing here, bail
        if(!non_whitespace) {
            return true;
        }

        // If we are starting on a comma, bail
        if(*data == ';') {
            return true;
        }

        // Check if we're in a group
        if(*data == '[') {
            if(right_square_bracket_offset == 0) {
                return false;
            }
            else {
                current_group = std::string(data + 1, right_square_bracket_offset - 1);
                return true;
            }
        }

        // Check if we have a key value
        if(equals_offset) {
            std::string key;
            if(current_group.size() == 0) {
                key = std::string(data, equals_offset);
            }
            else {
                key = current_group + "." + std::string(data, equals_offset);
            }
            return std::pair(key, std::string(data + equals_offset + 1, line_length - equals_offset - 1));
        }

        return false;
    }

    Ini::Ini(const char *path) {
        std::ifstream stream(path);
        this->load_from_stream(stream);
    }

    Ini::Ini(std::istream &stream) {
        this->load_from_stream(stream);
    }

    void Ini::load_from_stream(std::istream &stream) {
        std::string group;
        std::string line;
        while(std::getline(stream, line)) {
            auto result = digest_line(line.data(), nullptr, group);
            if(result.index() == 0) {
                this->set_value(std::move(std::get<0>(result)));
            }
            else if(std::get<1>(result) == false) {
                this->p_values.clear();
                return;
            }
        }
    }
}
