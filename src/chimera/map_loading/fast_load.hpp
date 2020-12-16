// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FAST_LOAD_HPP
#define CHIMERA_FAST_LOAD_HPP

#include <cstdio>
#include <cstdint>
#include <optional>
#include <string>
#include <filesystem>
#include "../halo_data/map.hpp"

namespace Chimera {
    struct MapEntry {
        bool multiplayer;
        std::optional<std::uint32_t> crc32;
        std::optional<CacheFileEngine> engine;
        std::string name;
        std::optional<std::uint32_t> index;
        
        std::filesystem::path get_file_path();
    };
    
    /**
     * Get the map entry for the map
     * @param  map_name map name
     * @return          a pointer to the map entry, if found
     */
    MapEntry *get_map_entry(const char *map_name);
    
    /**
     * Add a map to the map list if it does not exist
     * @param map_name   map to add
     * @return           reference to map in maps list
     */
    MapEntry &add_map_to_map_list(const char *map_name);
    
    /**
     * Resync the map list in the game with our own map list
     */
    void resync_map_list();
    
    
    /**
     * Initialize fast loading
     */
    void initialize_fast_load() noexcept;

    /**
     * Reload the map list
     */
    void reload_map_list_frame() noexcept;

    /**
     * Get the stock map CRC32
     * @param stock_map stock map CRC32 if present
     */
    std::optional<std::uint32_t> crc32_for_stock_map(const char *stock_map) noexcept;

    /**
     * Calculate CRC32 of a map file
     * @param  f      file descriptor
     * @param  header map header reference
     * @return        crc32
     */
    template <typename MapHeader> std::uint32_t calculate_crc32_of_map_file(std::FILE *f, const MapHeader &header) noexcept;
}

#endif
