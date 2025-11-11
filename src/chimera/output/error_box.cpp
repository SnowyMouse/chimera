// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "error_box.hpp"

namespace Chimera {
    void show_error_box(const char *header, const char *text, const std::optional<bool> &force, const std::optional<bool> &is_client) noexcept {
        auto &chimario = get_chimera();
        std::fprintf(stderr, "\n\nError [%s]\n\n----------------\n\n%s\n\n----------------\n\n", header, text);
        if (
            is_client.value_or(chimario.feature_present("client")) &&
            (
                force.value_or(false) ||
                chimario.get_ini() == nullptr ||
                !chimario.get_ini()->get_value_bool("error_handling.suppress_fatal_errors").value_or(false)
            )
        ) {
            MessageBox(nullptr, text, header, MB_OK | MB_ICONERROR);
        }
    }

    void throw_error(bool condition, const char *message) noexcept {
        if(!condition) {
            show_error_box("Error", message);
            std::exit(1);
        }
    }

}
