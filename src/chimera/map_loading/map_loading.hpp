// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_LOADING_HPP
#define CHIMERA_MAP_LOADING_HPP

namespace Chimera {
    #define BITMAPS_CUSTOM_MAP "custom_bitmaps"
    #define SOUNDS_CUSTOM_MAP "custom_sounds"
    #define LOC_CUSTOM_MAP "custom_loc"

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
}
#endif
