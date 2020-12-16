// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ERROR_BOX_HPP
#define CHIMERA_ERROR_BOX_HPP

#include <optional>

namespace Chimera {
    /**
     * Show an error box
     * @param header header to show
     * @param text   text to show
     * @param force  force showing a message box regardless of if it is there
     */
    void show_error_box(const char *header, const char *text, const std::optional<bool> &force = std::nullopt) noexcept;
}

#endif
