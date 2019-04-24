#ifndef CHIMERA_MAP_HPP
#define CHIMERA_MAP_HPP

#include <cstdint>

#include "pad.hpp"

namespace Chimera {
    /** This is the type of map */
    enum MapGameType : uint16_t {
        MAP_SINGLE_PLAYER = 0,
        MAP_MULTIPLAYER,
        MAP_USER_INTERFACE
    };

    /**
     * This is the header located at the first 2048 bytes of each map file
     */
    struct MapHeader {
        /** Must be equal to 0x68656164 */
        std::uint32_t head = 0x68656164;

        /** 7 if retail; 0x261 if Custom Edition; 5 if Xbox */
        std::uint32_t engine_type;

        /** File size in bytes; Halo ignores map files that are >384 MiB */
        std::uint32_t file_size;

        PAD(0x4);

        /** File offset to tag data (which is loaded to 0x40440000 in Halo's memory) */
        std::uint32_t tag_data_offset;

        /** File size of tag data in bytes */
        std::uint32_t tag_data_size;

        PAD(0x8);

        /** File name of map excluding extension; typically matches scenario tag name, but not required */
        char name[32];

        /** Unused on PC version of Halo; can be used to extend map name another 32 characters */
        char build[32];

        /** Game type of map (e.g. multiplayer) */
        MapGameType game_type;

        PAD(0x2);

        /** Calculated with CRC32 of BSPs, models, and tag data */
        std::uint32_t crc32_unused;

        PAD(0x794);

        /** Must be equal to 0x666F6F74 */
        std::uint32_t foot = 0x666F6F74;
    };
    static_assert(sizeof(MapHeader) == 0x800);

    /**
     * Get the map header of the currently loaded map
     * @return reference to the map header of the currently loaded map
     */
    MapHeader &get_map_header() noexcept;

    /** This is an individual map index */
    struct MapIndex {
        /** Pointer to the file name, not including the extension */
        char *file_name;

        /** Map name index; 13 = Unknown Level, first 13 maps must be the stock maps in order */
        std::uint32_t map_name_index;

        /** 1 if loaded and valid */
        std::uint8_t loaded;

        PAD(0x3);

        /** CRC32 of the map */
        std::uint32_t crc32;
    };
    static_assert(sizeof(MapIndex) == 0x10);

    /**
     * Get a pointer to all of the map indices
     * @return pointer to all of the map indices
     */
    MapIndex *map_indices() noexcept;

    /**
     * Get the number of maps loaded
     * @return pointer to all of the maps
     */
    std::uint32_t maps_count() noexcept;

    /**
     * Load ui.map
     */
    void load_ui_map() noexcept;
}

#endif
