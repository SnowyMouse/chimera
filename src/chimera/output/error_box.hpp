// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ERROR_BOX_HPP
#define CHIMERA_ERROR_BOX_HPP

#include <optional>

namespace Chimera {
    /**
     * Show an error box
     * @param header    header to show
     * @param text      text to show
     * @param force     force showing a message box regardless of the suppress_fatal_errors option
     * @param is_client specify if Chimera is running as a client (default is to check for client signatures)
     */
    void show_error_box(const char *header, const char *text, const std::optional<bool> &force = std::nullopt, const std::optional<bool> &is_client = std::nullopt) noexcept;

    /**
     * Throw an error and exit the game
     * @param condition    if this value is false, throw the error
     * @param message      text to show
     */
    void throw_error(bool condition, const char *message) noexcept;

}

#endif
