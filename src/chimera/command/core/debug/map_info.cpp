// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../../config/ini.hpp"
#include "../../../halo_data/game_engine.hpp"
#include "../../../halo_data/map.hpp"
#include "../../../halo_data/tag.hpp"
#include "../../../map_loading/fast_load.hpp"
#include "../../../map_loading/map_loading.hpp"
#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"

namespace Chimera {
    #define MIB_SIZE 1048576
    #define MAX_TAG_DATA_SIZE_MIB 64.00
    #define MAX_TAG_COUNT 65535
    #define OUTPUT_WITH_COLOR(...) console_output(body_color, __VA_ARGS__)
    #define SIZE_IN_MIB(value) (static_cast<float>(value) / MIB_SIZE)

    bool map_info_command(int, const char **) {
        // Remove output prefix
        extern const char *output_prefix;
        auto *prefix = output_prefix;
        output_prefix = nullptr;

        // Map info
        const char *map_name = nullptr;
        const char *map_build = nullptr;
        float tag_data_size = 0;
        std::uint32_t expected_crc32;
        CacheFileEngine engine;
        MapEntry *entry = nullptr;
        LoadedMap *loaded_map = nullptr;

        #define INFO_FROM_HEADER(map_header) { \
            map_name = map_header.name; \
            map_build = map_header.build; \
            tag_data_size = map_header.tag_data_size; \
            expected_crc32 = map_header.crc32; \
            engine = map_header.engine_type; \
        }

        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            auto &demo_map_header = get_demo_map_header();
            INFO_FROM_HEADER(demo_map_header);
        }
        else {
            auto &map_header = get_map_header();
            INFO_FROM_HEADER(map_header);
        }
        
        entry = get_map_entry(map_name);
        loaded_map = get_loaded_map(map_name);

        // is the map compressed?
        bool compressed = loaded_map->decompressed_size != loaded_map->file_size;

        // Map size
        std::size_t map_size = loaded_map->decompressed_size;

        // Tag count
        auto tag_count = get_tag_data_header().tag_count;

        // Output colors
        auto header_color = ConsoleColor::header_color();
        auto body_color = ConsoleColor::body_color();

        // Print header
        console_output(header_color, "%s", localize("chimera_map_info_command_current_map_info"));

        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_name"), map_name);
        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_build"), map_build);

        const char *target_engine;
        switch(engine) {
            case CacheFileEngine::CACHE_FILE_DEMO:
                target_engine = "Halo Demo / Trial";
                break;
            case CacheFileEngine::CACHE_FILE_RETAIL:
                target_engine = "Halo: Combat Evolved (PC)";
                break;
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                target_engine = "Halo: Custom Edition";
                break;
            // case CacheFileEngine::CACHE_FILE_INVADER:
            //     target_engine = "Invader (Native)";
            //     break;
            // case CacheFileEngine::CACHE_FILE_XBOX:
            //     target_engine = "Halo: Combat Evolved (Xbox)";
            //     break;
            default:
                target_engine = "Bullshit (what did you think was going to be here? lol)";
                break;
        }
        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_target_engine"), target_engine);

        try {
            if(expected_crc32 == entry->crc32) {
                OUTPUT_WITH_COLOR("CRC32: 0x%.08X", entry->crc32.value());
            }
            else {
                console_error("CRC32: 0x%.08X (%s)", entry->crc32.value(), localize("chimera_map_info_command_mismatched"));
            }
        }
        catch (std::exception &) {
            console_error("CRC32: ?");
        }

        const char *map_is_compressed = compressed ? localize("common_yes") : localize("common_no");
        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_compressed"), map_is_compressed);

        if(compressed) {
            std::size_t compressed_map_size = loaded_map->file_size;
            OUTPUT_WITH_COLOR("%s: %.2f MiB", localize("chimera_map_info_command_map_size"), SIZE_IN_MIB(compressed_map_size));
            OUTPUT_WITH_COLOR("%s: %.2f MiB", localize("chimera_map_info_command_uncompressed_map_size"), SIZE_IN_MIB(map_size));
        }
        else {
            OUTPUT_WITH_COLOR("%s: %.2f MiB", localize("chimera_map_info_command_map_size"), SIZE_IN_MIB(map_size));
        }

        if(get_chimera().get_ini()->get_value_bool("memory.enable_map_memory_buffer").value_or(false)) {
            std::size_t buffer_used = loaded_map->loaded_size;
            std::size_t buffer_size = loaded_map->buffer_size;
            
            float buffer_used_percentage = (static_cast<float>(buffer_used) / buffer_size) * 100;

            const char *key_string = localize("chimera_map_info_command_ram_buffer");
            OUTPUT_WITH_COLOR("%s: %.2f MiB / %.2f MiB (%.2f%%)", key_string, SIZE_IN_MIB(buffer_used), SIZE_IN_MIB(buffer_size), buffer_used_percentage);
        }

        OUTPUT_WITH_COLOR("%s: %d / %d", localize("chimera_map_info_command_map_tag_count"), tag_count, MAX_TAG_COUNT);
        OUTPUT_WITH_COLOR("%s: %.2f MiB / %.2f MiB", localize("chimera_map_info_command_map_tag_data_size"), SIZE_IN_MIB(tag_data_size), MAX_TAG_DATA_SIZE_MIB);

        bool map_protected_bool = map_is_protected();
        const char *map_protected = map_protected_bool ? localize("common_yes") : localize("common_no");
        if(!map_protected_bool) {
            OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_protected"), map_protected);
        }
        else {
            console_error("%s: %s", localize("chimera_map_info_command_map_protected"), map_protected);
        }

        // Restore the output prefix
        output_prefix = prefix;

        return true;
    }
}
