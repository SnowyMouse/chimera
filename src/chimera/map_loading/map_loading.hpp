// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_LOADING_HPP
#define CHIMERA_MAP_LOADING_HPP

#include <cstdint>
#include <cstddef>
#include <filesystem>

namespace Chimera {
    struct LoadedMap {
        std::string name;
        std::filesystem::path path;
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
