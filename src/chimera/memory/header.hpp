/**
 * Copyright (c) 2018 Kavawuvi
 *
 * This code is licensed under the GNU GPL version 3 or any later version
 * See LICENSE for more information.
 */

#ifndef VAPMAP_HEADER_H
#define VAPMAP_HEADER_H

#include <cstdint>

namespace VapMap {
    struct VAPDate {
        /** Year (0-9999) */
        int year;

        /** Month (1-12) */
        int month;

        /** Day (1-28, 29, 30, or 31 depending on month and year) */
        int day;

        /** Hour (0-23) */
        int hour;

        /** Minute (0-59) */
        int minute;

        /** Second (0-59) */
        int second;

        /** Nanosecond (0-999999999) */
        int nanosecond;

        /**
         * Initialize a VAPDate from the given ISO date. Date must be formatted as YYYY-MM-DDThh:mm:ss.fffffffff.
         * @param  iso_date ISO date string
         * @return          true if successful
         */
        bool initialize_from_date(const char *iso_date) noexcept;

        /**
         * Write the ISO date string. iso_date must be valid for at least 30 characters and will be formatted as
         * YYYY-MM-DDThh:mm:ss.fffffffff.
         * @param iso_date ISO date output
         * @return         true if successful
         */
        bool write_date(char *iso_date) const noexcept;

        /**
         * Check if the date is valid
         * @return true if date is valid
         */
        bool valid() const noexcept;
    };

    /**
     * VAP map data
     */
    struct VAPStruct {
        /** Compression type used */
        enum VAPCompression : std::uint16_t {
            VAP_COMPRESSION_UNCOMPRESSED = 0,
            VAP_COMPRESSION_LZMA = 1
        };

        /** Compression type used */
        enum VAPVersion : std::uint16_t {
            VAP_VERSION_CHIMERA_1 = 0,
            VAP_VERSION_NEXT
        };

        /** Compression type used */
        enum VAPFeatureLevel : std::uint16_t {
            VAP_FEATURE_CHIMERA_1 = 0,
            VAP_FEATURE_NEXT
        };

        // 0x0080

        /** File size of the map when decompressed, include header size */
        std::uint32_t decompressed_file_size;

        /** Type of compression used. */
        VAPCompression compression_type;

        /** Version of the vap format used. Currently only VAP_VERSION_CHIMERA_1 is used. */
        VAPVersion vap_version;

        /** Number of blocks. If this is 0, the data is assumed to be one contiguous stream of data */
        std::uint32_t block_count;

        /** Compressed file size */
        std::uint32_t compressed_size;

        // 0x0090

        /** Feature level */
        std::uint32_t feature_level;

        /** Player limit (campaign); Set to 0 for multiplayer and user interface maps */
        std::uint16_t max_players;

        /** Reserved for future use; Leave it at 0 */
        std::uint16_t reserved1;

        /** Reserved for future use; Leave it at 0 */
        std::uint32_t reserved2[2];

        // 0x00A0

        /** Build date of the map in ISO 8601 (yyyy-mm-ddThh:mm:ss.fffffffff) format. Must be null terminated */
        char build_date[0x20];

        // 0x00C0

        /** Human-readable of the map. Must be null terminated */
        char name[0x20];

        // 0x00E0

        /** Human-readable description of the map. Must be null terminated */
        char description[0x80];

        // 0x0160

        /** Reserved for future use; Leave it at 0 */
        char reserved3[0x100];

        /**
         * Check if this struct is valid
         * @return true if the struct is valid
         */
        bool valid() const noexcept;
    };

    /**
     * Cache file header
     */
    struct VAPFileHeader {
        /** VAP engine */
        enum VAPEngine : std::uint32_t {
            /** Custom Edition format (not VAP) */
            VAP_ENGINE_CUSTOM_EDITION = 0x261,

            /** VAP format */
            VAP_ENGINE_VAP = 0x86
        };

        /** VAP map type */
        enum VAPMapType : std::uint16_t {
            /** Campaign */
            VAP_MAP_CAMPAIGN = 0,

            /** Multiplayer */
            VAP_MAP_MULTIPLAYER = 1,

            /** User Interface */
            VAP_MAP_UI = 2
        };

        // 0x0000

        /** Literal 0x68656164 */
        std::uint32_t head_literal;

        /** Engine; 0x261 for Halo Custom Edition, 0x86 for VAP */
        VAPEngine engine;

        /** Spawn of Satan. DO NOT TOUCH. JUST LEAVE THIS AT 0. */
        std::uint32_t file_size;

        /** Padding; must be zero'd out before doing anything with this tool */
        char pad1[0x4];

        // 0x0010

        /** Offset of the tag data when the map is decompressed */
        std::uint32_t tag_data_offset;

        /** Size of the tag data when the map is decompressed */
        std::uint32_t tag_data_size;

        /** Padding; must be zero'd out before doing anything with this tool */
        char pad2[0x8];

        // 0x0020

        /** Name of map. Must be null-terminated. */
        char name[0x20];

        // 0x0040

        /** Unused. Must be null-terminated. */
        char build[0x20];

        // 0x0060

        /** Type of map (0 = singleplayer, 1 = multiplayer, 2 = UI) */
        VAPMapType map_type;

        /** Padding; must be zero'd out before doing anything with this tool */
        char pad3[0x2];

        /** CRC32 - Unused by Halo */
        std::uint32_t crc32_unused;

        /** Padding; must be zero'd out before doing anything with this tool */
        char pad4[0x18];

        // 0x0080

        /** VAP struct */
        VAPStruct vap;

        /** Padding; must be zero'd out before doing anything with this tool */
        char pad5[0x59C];

        // 0x079C

        /** Literal 0x666F6F74 */
        std::uint32_t foot_literal;

        /**
         * Get whether or not the header is dirty. Dirty headers have extra data laying around.
         * @return true if the header is dirty
         */
        bool clean_header() const noexcept;

        /**
         * Check if the header is valid.
         * @return true if header is valid
         */
        bool valid() const noexcept;
    };

    static_assert(sizeof(VAPFileHeader) == 0x800);

    /**
     * Compressed block index. These always go after the file header in a vap.block_count-sized contiguous array.
     *
     * If block_count is 0, then the map will be assumed to be one stream of data rather than divided into blocks.
     */
    struct VAPBlockIndex {
        /** File offset of the compressed block */
        std::uint32_t file_offset;

        /** File size of the compressed block */
        std::uint32_t file_size;

        /** Decompressed size of the compressed block */
        std::uint32_t decompressed_size;

        /** Reserved for potential future usage */
        std::uint32_t reserved[1];
    };

    static_assert(sizeof(VAPBlockIndex) == 0x10);
}

#endif
