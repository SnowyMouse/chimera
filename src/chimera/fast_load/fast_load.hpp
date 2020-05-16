// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FAST_LOAD_HPP
#define CHIMERA_FAST_LOAD_HPP

#include <cstdio>
#include <cstdint>

namespace Chimera {
    /**
     * Initialize fast loading
     */
    void initialize_fast_load() noexcept;

    /**
     * Reload the map list
     */
    void reload_map_list() noexcept;
}

#endif
