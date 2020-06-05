// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CE_BRIDGE_MAP_SUPPORT_HPP
#define CHIMERA_CE_BRIDGE_MAP_SUPPORT_HPP

#include <cstddef>

namespace Chimera {
    /**
     * Set up using Custom Edition maps on retail
     * @return true if successful, false if not
     */
    bool set_up_custom_edition_map_support() noexcept;

    /**
     * Load Custom Edition resource map data
     * @param output      output buffer to load to
     * @param file_offset offset to load from
     * @param file_size   size to load
     * @param map_name    map name
     */
    void load_custom_edition_resource_data_in_retail(std::byte *output, std::size_t file_offset, std::size_t file_size, const char *map_name);
}

#endif
