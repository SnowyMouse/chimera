// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_LOADING_HPP
#define CHIMERA_MAP_LOADING_HPP

#include <cstdint>
#include <cstddef>
#include <filesystem>

namespace Chimera {
    #define BITMAPS_CUSTOM_MAP_NAME "custom_bitmaps"
    #define SOUNDS_CUSTOM_MAP_NAME "custom_sounds"
    #define LOC_CUSTOM_MAP_NAME "custom_loc"

    #define BITMAPS_CUSTOM_MAP "maps\\" BITMAPS_CUSTOM_MAP_NAME ".map"
    #define SOUNDS_CUSTOM_MAP "maps\\" SOUNDS_CUSTOM_MAP_NAME ".map"
    #define LOC_CUSTOM_MAP "maps\\" LOC_CUSTOM_MAP_NAME ".map"
    
	struct LoadedMap {
		std::string name;
		std::filesystem::path path;
		std::filesystem::path absolute_path;
		std::optional<std::uint32_t> tmp_file;
		std::optional<std::byte *> memory_location;
        std::size_t buffer_size;
        std::size_t decompressed_size;
		std::size_t loaded_size;
        std::size_t file_size;
		std::filesystem::file_time_type timestamp; // time it was modified
	};
    
    /**
     * Get the loaded map
     * @param  name name of map
     * @return      reference to map if found, or nullptr
     */
    LoadedMap *get_loaded_map(const char *name) noexcept;

    /**
     * Set up loading maps outside of the maps directory
     */
    void set_up_map_loading();
    
    /**
     * Load the map
     * @param map_name map to load
     * @return         loaded map
     */
    LoadedMap *load_map(const char *map_name);
}
#endif
