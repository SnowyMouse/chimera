// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SCRIPT_HPP
#define CHIMERA_SCRIPT_HPP

#include <cstdint>

namespace Chimera {
    /**
     * Execute the script
     * @param script script to execute
     * @param lower  have Halo lowercase the string
     */
    void execute_script(const char *script, bool lower = false) noexcept;
}

#endif
