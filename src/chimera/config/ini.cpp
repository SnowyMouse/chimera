// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include <windows.h>
#include <iostream>
#include <istream>
#include <locale>
#include <codecvt>
#include "ini.hpp"
#include "../command/command.hpp"
#include "../chimera.hpp"
#include "../output/error_box.hpp"
#include <cstring>

namespace Chimera {
    static std::locale locale;
    
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
            show_error_box("INI error", error);
            std::exit(136);
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
            show_error_box("INI error", error);
            std::exit(136);
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
            show_error_box("INI error", error);
            std::exit(136);
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

    /**
     * Convert a string encoded in UTF-8 to one encoded using the system's 8-bit encoding (ANSI)
     * @param str  UTF-8-encoded string
     * @param dflt default character to use if the conversion to ANSI fails
     * @return     false if the input string could not be decoded as UTF-8, true otherwise.
     */
    static bool utf8_to_ansi(std::string& str, char dflt){
        std::wstring wstr;
        try {
            wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(str.data());
        }
        catch (std::range_error&){
            return false;
        }
        std::string tmp(wstr.size(), '0');
        
        std::use_facet<std::ctype<wchar_t>>(locale).narrow(wstr.data(), wstr.data() + wstr.size(), dflt, &tmp[0]);
        str.swap(tmp);
        return true;
    }

    // Returns false on error, a key/value pair, or true to do nothing
    static std::variant<std::pair<std::string, std::string>, bool> digest_line(const char *data, const char **new_offset, std::string &current_group, std::size_t line_number) {
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

        auto show_error = [&line_number, &data]() {
            // We can't feasibly continue from this without causing undefined behavior. Abort the process after showing an error message.
            char error[1024];
            std::snprintf(error, sizeof(error), "chimera.INI error (line #%zu):\n\n%s\n\nThis line could not be parsed.\n", line_number, data);
            show_error_box("INI error", error);
            std::exit(136);
        };

        // Check if we're in a group
        if(*data == '[') {
            if(right_square_bracket_offset == 0) {
                show_error();
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
            std::string value;
            if(current_group.size() == 0) {
                key = std::string(data, equals_offset);
            }
            else {
                key = current_group + "." + std::string(data, equals_offset);
            }

            // Get the value and decode it
            value = std::string(data + equals_offset + 1, line_length - equals_offset - 1);
            // Use ACK (0x06) for replacing invalid chars since it will be rendered as a box character in the error message
            if (!utf8_to_ansi(value, '\x06')){
                show_error_box("INI error", (std::string() + "Failed to decode value of '" + key + "' in chimera.ini.\n\nMake sure the file is encoded using UTF-8.").data());
                std::exit(136);
            }
            else if (value.find('\x06') != std::string::npos){
                show_error_box("INI error", (std::string() + "Invalid character in the value of '" + key + "' in chimera.ini:\n\n" + value + "\n\nOnly characters your system can encode in ANSI are valid.").data());
                std::exit(136);
            }
            return std::pair(key, value);
        }

        show_error();
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
        try {
            locale = std::locale("");
        }
        catch(std::exception &e) {
            std::fprintf(stderr, "Failed to use default locale - %s\n", e.what());
        }
        
        std::string group;
        std::string line;
        std::size_t no = 0;

        if(!stream.good()) {
            show_error_box("INI error", "chimera.ini could not be opened.\n\nMake sure it exists and you have permission to it.");
            std::exit(1);
        }

        while(std::getline(stream, line)) {
            auto result = digest_line(line.data(), nullptr, group, ++no);
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
