// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "menu.hpp"

namespace Chimera {
    extern "C" void load_main_menu_asm() noexcept;

    void load_main_menu() noexcept {
        load_main_menu_asm();
    }
}