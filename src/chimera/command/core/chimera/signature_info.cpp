// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>

#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../halo_data/path.hpp"
#include "../../command.hpp"

namespace Chimera {
    bool Chimera::signature_info_command(int, const char **argv) noexcept {
        // Check if we want to dump signatures
        if(std::strcmp(*argv, "dump") == 0) {
            // Get the process name
            char b[256];
            GetModuleFileNameA(nullptr, b, 256);
            const char *name = b;
            for(const char *i = b; *i; i++) {
                if(*i == '/' || *i == '\\') {
                    name = i + 1;
                }
            }

            // Get signature info
            auto &chimera = get_chimera();

            // Make a path
            char path[MAX_PATH];
            std::snprintf(path, sizeof(path), "%schimera_signature_dump.csv", chimera.get_path());

            // Open the file
            std::ofstream o(path, std::ios_base::out | std::ios_base::trunc);
            char line[256];

            #define DUMP_FMT "%s,%s,%s,%s\n"
            std::snprintf(line, sizeof(line), DUMP_FMT, "Signature", "Address", "Relative Address", "Feature");
            o << line;

            // Go through all signatures
            for(auto &sig : chimera.p_signatures) {
                auto data_ptr = reinterpret_cast<std::uintptr_t>(sig.data());
                char address[256] = "NULL";
                char address_rel[256] = "";

                // Copy the address data
                if(data_ptr) {
                    std::snprintf(address, sizeof(address), "0x%.08X", data_ptr);
                    std::snprintf(address_rel, sizeof(address_rel), "%s+%X", name, data_ptr - 0x400000);
                }

                // Print it
                std::snprintf(line, sizeof(line), DUMP_FMT, sig.name(), address, address_rel, sig.feature());
                o << line;
            }

            // Close
            o.flush();
            o.close();

            // Done
            console_output(localize("chimera_signature_info_command_dumped"), path);

            return true;
        }
        for(auto &sig : get_chimera().p_signatures) {
            if(std::strcmp(sig.name(), *argv) == 0) {
                extern const char *output_prefix;
                auto *old_prefix = output_prefix;
                output_prefix = nullptr;

                console_output(ConsoleColor::header_color(), localize("chimera_signature_info_command_signature_info"), *argv);
                console_output(ConsoleColor::body_color(), "    %s: %s", localize("chimera_signature_info_command_signature_feature"), sig.feature());

                auto *ptr = sig.data();
                if(ptr) {
                    console_output(ConsoleColor::body_color(), "    %s: 0x%.08X", localize("chimera_signature_info_command_signature_address"), sig.data());
                }
                else {
                    console_output(ConsoleColor::body_color(), "    %s: NULL", localize("chimera_signature_info_command_signature_address"));
                }

                output_prefix = old_prefix;
                return true;
            }
        }

        console_error(localize("chimera_signature_info_command_error"), *argv);
        return false;
    }
}
