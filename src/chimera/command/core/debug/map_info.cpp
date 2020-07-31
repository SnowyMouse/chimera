// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../../config/ini.hpp"
#include "../../../halo_data/game_engine.hpp"
#include "../../../halo_data/map.hpp"
#include "../../../halo_data/tag.hpp"
#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../chimera.hpp"

namespace Chimera {
    #define MIB_SIZE 1048576
    #define MAX_TAG_DATA_SIZE_MIB 23.00
    #define MAX_TAG_COUNT 65535
    #define OUTPUT_WITH_COLOR(...) console_output(body_color, __VA_ARGS__)

    bool map_info_command(int, const char **) {
        // Remove output prefix
        extern const char *output_prefix;
        auto *prefix = output_prefix;
        output_prefix = nullptr;

        // Map info
        const char* map_name = nullptr;
        const char* map_build = nullptr;
        float map_size = 0;
        float tag_data_size = 0;
        std::uint32_t crc32 = 0;

        #define INFO_FROM_HEADER(map_header) { \
            map_name = map_header.name; \
            map_build = map_header.build; \
            map_size = map_header.file_size; \
            tag_data_size = map_header.tag_data_size; \
            crc32 = map_header.crc32_unused; \
        }

        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            auto &demo_map_header = get_demo_map_header();
            INFO_FROM_HEADER(demo_map_header);
        }
        else {
            auto &map_header = get_map_header();
            INFO_FROM_HEADER(map_header);
        }

        // size in MiB pls
        map_size /= MIB_SIZE;
        tag_data_size /= MIB_SIZE;

        // Output colors
        auto header_color = ConsoleColor::header_color();
        auto body_color = ConsoleColor::body_color();

        // Print everything!!
        console_output(header_color, "%s", localize("chimera_map_info_command_current_map_info"));

        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_name"), map_name);
        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_build"), map_build);
        OUTPUT_WITH_COLOR("CRC32: 0x%.08X", crc32);

        if(get_chimera().get_ini()->get_value_bool("memory.enable_map_memory_buffer").value_or(false)) {
            float buffer_size = 0;
            float buffer_used = 0;

            auto read_mib = [](const char *what, std::size_t default_value) -> std::size_t {
                return get_chimera().get_ini()->get_value_size(what).value_or(default_value) * 1024 * 1024;
            };

            if(std::strcmp(map_name, "ui") == 0) {
                size_t ui_buffer_size = read_mib("memory.ui_size", 256);
                buffer_size = ui_buffer_size / MIB_SIZE;
            }
            else {
                size_t map_buffer_size = read_mib("memory.map_size", 768);
                buffer_size = map_buffer_size / MIB_SIZE;
            }
            
            buffer_used = (map_size / buffer_size) * 100;

            const char* key_string = localize("chimera_map_info_command_map_size");
            const char* format = "%s: %.2f MiB / %.2f MiB (%.2f%%)";

            OUTPUT_WITH_COLOR(format, key_string, map_size, buffer_size, buffer_used);
        }
        else {
            OUTPUT_WITH_COLOR("%s: %.2f MiB", localize("chimera_map_info_command_map_size"), map_size);
        }

        OUTPUT_WITH_COLOR("%s: %.2f MiB / %.2f MiB", localize("chimera_map_info_command_map_tag_data_size"), tag_data_size, MAX_TAG_DATA_SIZE_MIB);

        auto tag_count = get_tag_data_header().tag_count;
        OUTPUT_WITH_COLOR("%s: %d / %d", localize("chimera_map_info_command_map_tag_count"), tag_count, MAX_TAG_COUNT);

        const char* map_protected = map_is_protected() ? localize("common_yes") : localize("common_no");
        OUTPUT_WITH_COLOR("%s: %s", localize("chimera_map_info_command_map_protected"), map_protected);

        // Restore the output prefix
        output_prefix = prefix;

        return true;
    }
}
