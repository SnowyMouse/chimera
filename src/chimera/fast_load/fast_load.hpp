// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FAST_LOAD_HPP
#define CHIMERA_FAST_LOAD_HPP

#include <cstdio>
#include <cstdint>
#include <optional>

namespace Chimera {
    /**
     * Initialize fast loading
     */
    void initialize_fast_load() noexcept;

    /**
     * Reload the map list
     */
    void reload_map_list() noexcept;

    /**
     * Get the stock map CRC32
     * @param stock_map stock map CRC32 if present
     */
    std::optional<std::uint32_t> crc32_for_stock_map(const char *stock_map) noexcept;
}

#endif
