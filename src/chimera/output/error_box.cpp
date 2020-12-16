// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "error_box.hpp"

namespace Chimera {
    void show_error_box(const char *header, const char *text, const std::optional<bool> &force) noexcept {
        auto &chimario = get_chimera();
        std::fprintf(stderr, "\n\nError [%s]\n\n----------------\n\n%s\n\n----------------\n\n", header, text);
        if(chimario.feature_present("client") && force.value_or(!chimario.get_ini()->get_value_bool("error_handling.suppress_fatal_errors").value_or(false))) {
            MessageBox(nullptr, text, header, MB_OK | MB_ICONERROR);
        }
    }
}
