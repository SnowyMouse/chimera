// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../version.hpp"

namespace Chimera {
    static void print_section(const char *header, const char *text) noexcept;

    bool about_command(int, const char **) {
        extern const char *output_prefix;
        auto *old_prefix = output_prefix;
        output_prefix = nullptr;

        // Print each line
        print_section("Version", "Chimera " CHIMERA_VERSION_STRING);
        print_section(localize("chimera_about_command_credits_header"), localize("chimera_about_command_credits"));
        print_section(localize("chimera_about_command_license_header"), localize("chimera_about_command_license"));

        output_prefix = old_prefix;
        return true;
    }

    static void print_section(const char *header, const char *text) noexcept {
        console_output(ConsoleColor::header_color(), header);
        char line[256];
        std::size_t line_length = 0;
        const char *format = "    %s";
        while(*text) {
            if(*text == '\n') {
                line[line_length] = 0;
                console_output(ConsoleColor::body_color(), format, line);
                line_length = 0;
                text++;
                continue;
            }
            line[line_length++] = *text;
            text++;
        }
        line[line_length] = 0;
        if(line_length) {
            console_output(ConsoleColor::body_color(), format, line);
        }
    }
}
