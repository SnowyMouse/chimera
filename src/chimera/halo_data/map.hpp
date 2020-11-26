// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_HPP
#define CHIMERA_MAP_HPP

#include <cstdint>
#include <cstddef>

#include "pad.hpp"

namespace Chimera {
    /** This is the type of map */
    enum MapGameType : uint16_t {
        MAP_SINGLE_PLAYER = 0,
        MAP_MULTIPLAYER,
        MAP_USER_INTERFACE
    };

    enum CacheFileEngine : std::uint32_t {
        CACHE_FILE_XBOX = 0x5,
        CACHE_FILE_DEMO = 0x6,
        CACHE_FILE_RETAIL = 0x7,
        CACHE_FILE_CUSTOM_EDITION = 0x261,
        CACHE_FILE_INVADER = 0x1A86,

        CACHE_FILE_DEMO_COMPRESSED = 0x861A0006,
        CACHE_FILE_RETAIL_COMPRESSED = 0x861A0007,
        CACHE_FILE_CUSTOM_EDITION_COMPRESSED = 0x861A0261
    };

    /**
     * This is the header located at the first 2048 bytes of each map file
     */
    struct MapHeader {
        static const std::uint32_t HEAD_LITERAL = 0x68656164;
        static const std::uint32_t FOOT_LITERAL = 0x666F6F74;

        /** Must be equal to 0x68656164 */
        std::uint32_t head;

        /** 7 if retail; 0x261 if Custom Edition; 5 if Xbox */
        CacheFileEngine engine_type;

        /** File size in bytes; Halo ignores map files that are >384 MiB */
        std::uint32_t file_size;

        PAD(0x4);

        /** File offset to tag data */
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
        std::uint32_t crc32;

        PAD(0x2B0);

        PAD(0x4E4);

        /** Must be equal to 0x666F6F74 */
        std::uint32_t foot;
        
        bool is_valid() const noexcept;
    };
    static_assert(sizeof(MapHeader) == 0x800);

    struct MapHeaderDemo {
        static const std::uint32_t HEAD_LITERAL = 0x45686564;
        static const std::uint32_t FOOT_LITERAL = 0x47666F74;

        PAD(0x2);

        /** Game type of map (e.g. multiplayer) */
        MapGameType game_type;

        PAD(0x2BC);

        /** Must be equal to 0x45686564 */
        std::uint32_t head;

        /** File size of tag data in bytes */
        std::uint32_t tag_data_size;

        /** Unused on PC version of Halo */
        char build[32];

        PAD(0x2A0);

        /** 6 */
        CacheFileEngine engine_type;

        /** File name of map excluding extension; typically matches scenario tag name, but not required */
        char name[32];

        PAD(0x4);

        /** Calculated with CRC32 of BSPs, models, and tag data */
        std::uint32_t crc32;

        PAD(0x34);

        /** File size in bytes; Halo ignores map files that are >384 MiB */
        std::uint32_t file_size;

        /** File offset to tag data */
        std::uint32_t tag_data_offset;

        /** Must be equal to 0x47666F74 */
        std::uint32_t foot;

        PAD(0x20C);
        
        bool is_valid() const noexcept;
    };
    static_assert(sizeof(MapHeaderDemo) == 0x800);

    /**
     * Get the map header of the currently loaded map
     * @return reference to the map header of the currently loaded map
     */
    MapHeader &get_map_header() noexcept;
    MapHeaderDemo &get_demo_map_header() noexcept;

    /** This is an individual map index */
    struct MapIndex {
        /** Pointer to the file name, not including the extension */
        const char *file_name;

        /** Map name index; 13 = Unknown Level, first 13 maps must be the stock maps in order */
        std::uint32_t map_name_index;
    };
    static_assert(sizeof(MapIndex) == 0x8);

    struct MapIndexRetail : MapIndex {
        /** 1 if loaded and valid */
        std::uint8_t loaded;

        PAD(0x3);
    };
    static_assert(sizeof(MapIndexRetail) == 0xC);

    /** This is an individual map index */
    struct MapIndexCustomEdition : MapIndexRetail {
        /** CRC32 checksum for joining/hosting servers */
        std::uint32_t crc32;
    };
    static_assert(sizeof(MapIndexCustomEdition) == 0x10);

    struct MapList {
        MapIndex *map_list;
        std::uint32_t map_count;
    };

    /**
     * Get the map list
     * @return map list
     */
    MapList &get_map_list() noexcept;

    /**
     * Get whether the map is protected
     */
    bool map_is_protected() noexcept;

    /**
     * Load ui.map
     */
    void load_ui_map() noexcept;
    
    /**
     * Get the name of the currently loaded map
     * @return name of currently loaded map
     */
    const char *get_map_name() noexcept;
}

#endif
