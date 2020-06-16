// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_LOADING_HPP
#define CHIMERA_MAP_LOADING_HPP

#include <cstdint>

namespace Chimera {
    #define BITMAPS_CUSTOM_MAP_NAME "custom_bitmaps"
    #define SOUNDS_CUSTOM_MAP_NAME "custom_sounds"
    #define LOC_CUSTOM_MAP_NAME "custom_loc"

    #define BITMAPS_CUSTOM_MAP "maps\\" BITMAPS_CUSTOM_MAP_NAME ".map"
    #define SOUNDS_CUSTOM_MAP "maps\\" SOUNDS_CUSTOM_MAP_NAME ".map"
    #define LOC_CUSTOM_MAP "maps\\" LOC_CUSTOM_MAP_NAME ".map"

    /**
     * Set up loading maps outside of the maps directory
     */
    void set_up_map_loading();

    /**
     * Get the path for the map
     * @param  map name of the map
     * @param  tmp get the tmp file path, instead, if available
     * @return     path to the map if found
     */
    const char *path_for_map(const char *map, bool tmp = false) noexcept;

    /**
     * Calculate the crc32 of the currently loaded map
     * @return crc32
     */
    std::uint32_t calculate_crc32_of_current_map_file() noexcept;
}
#endif
