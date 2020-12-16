// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ERROR_BOX_HPP
#define CHIMERA_ERROR_BOX_HPP

#include <cstdio>
#include <windows.h>

namespace Chimera {
    /**
     * Show an error box
     * @param header header to show
     * @param text   text to show
     */
    void show_error_box(const char *header, const char *text) noexcept;
}

#endif
