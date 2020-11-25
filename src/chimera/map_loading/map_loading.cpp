// SPDX-License-Identifier: GPL-3.0-only

#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <filesystem>
#include <sys/stat.h>
#include <windows.h>
#include "map_loading.hpp"
#include "laa.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"
#include "../localization/localization.hpp"
#include "../output/draw_text.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/script.hpp"
#include "../halo_data/map.hpp"
#include "../map_loading/fast_load.hpp"
#include "../halo_data/tag.hpp"
#include "../config/ini.hpp"
#include "../event/frame.hpp"
#include "../event/map_load.hpp"
#include "../../hac_map_downloader/hac_map_downloader.hpp"
#include "../bookmark/bookmark.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../fix/custom_edition_bridge/map_support.hpp"
#include "compression.hpp"
#include <chrono>

#define BYTES_TO_MiB(bytes) (bytes / 1024.0F / 1024.0F)

namespace Chimera {
    static bool allow_retail_maps = false;
    static bool do_maps_in_ram = false;
    static bool do_benchmark = false;

    std::byte *maps_in_ram_region = nullptr;
    static std::byte *ui_region = nullptr;

    std::size_t ui_map_file_size = 0;
    std::size_t map_file_size = 0;
    std::size_t compressed_ui_map_file_size = 0;
    std::size_t compressed_map_file_size = 0;

    std::size_t UI_OFFSET = 768 * 1024 * 1024;
    std::size_t UI_SIZE = 256 * 1024 * 1024;
    #define CHIMERA_MEMORY_ALLOCATION_SIZE (UI_OFFSET + UI_SIZE)

    std::size_t ui_buffer_used = 0;
    std::size_t map_buffer_used = 0;

    const char *latest_map_loaded_multiplayer = nullptr;

    extern "C" {
        std::byte *on_map_load_multiplayer_fail;
        char16_t download_text_string[64] = {};
        void on_map_load_multiplayer_asm() noexcept;
        void on_server_join_text_asm() noexcept;
    }

    extern "C" bool using_custom_map_on_retail() noexcept {
        return custom_edition_maps_supported_on_retail() && get_map_header().engine_type == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
    }

    struct CompressedMapIndex {
        char map_name[32];
        std::uint64_t date_modified;
    };

    static bool header_is_valid_for_this_game(const std::byte *header, bool *compressed = nullptr, const char *map_name = nullptr) {
        const auto &header_full_version = *reinterpret_cast<const MapHeader *>(header);
        const auto &header_demo_version = *reinterpret_cast<const MapHeaderDemo *>(header);

        // Copy everything lowercase
        char map_name_lowercase[sizeof(header_demo_version.name)] = {};
        std::size_t map_name_length = std::strlen(map_name);
        if(map_name_length >= sizeof(map_name_lowercase)) {
            return false; // the map is longer than 31 characters, thus it's a meme
        }
        for(std::size_t i = 0; i < map_name_length; i++) {
            map_name_lowercase[i] = std::tolower(map_name[i]);
        }

        // Set everything to lowercase
        char demo_name[sizeof(header_demo_version.name)] = {};
        char full_name[sizeof(header_full_version.name)] = {};
        for(std::size_t i = 0; i < sizeof(demo_name); i++) {
            demo_name[i] = std::tolower(header_demo_version.name[i]);
        }
        for(std::size_t i = 0; i < sizeof(full_name); i++) {
            full_name[i] = std::tolower(header_full_version.name[i]);
        }

        // Blorp
        demo_name[31] = 0;
        full_name[31] = 0;

        bool header_full_version_valid = header_full_version.head == MapHeader::HEAD_LITERAL && header_full_version.foot == MapHeader::FOOT_LITERAL && std::strcmp(full_name, map_name_lowercase) == 0;
        bool header_demo_version_valid = header_demo_version.head == MapHeaderDemo::HEAD_LITERAL && header_demo_version.foot == MapHeaderDemo::FOOT_LITERAL && std::strcmp(demo_name, map_name_lowercase) == 0;

        switch(game_engine()) {
            case GAME_ENGINE_DEMO:
                if(header_demo_version.engine_type == CacheFileEngine::CACHE_FILE_DEMO) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_demo_version_valid;
                }
                else if(header_full_version.engine_type == CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
                }
                else {
                    return false;
                }
            case GAME_ENGINE_CUSTOM_EDITION:
            ALSO_CHECK_IF_CUSTOM_EDITION:
                if(header_full_version.engine_type == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_full_version_valid;
                }
                else if(header_full_version.engine_type == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
                }
                else {
                    return false;
                }
            case GAME_ENGINE_RETAIL:
                if(header_full_version.engine_type == CacheFileEngine::CACHE_FILE_RETAIL) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_full_version_valid;
                }
                else if(header_full_version.engine_type == CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
                }
                else if(custom_edition_maps_supported_on_retail()) {
                    goto ALSO_CHECK_IF_CUSTOM_EDITION;
                }
                else {
                    return false;
                }
        }
        return false;
    }

    static bool header_is_valid_for_this_game(const char *path, bool *compressed = nullptr, const char *map_name = nullptr) {
        // Open the map
        std::FILE *f = std::fopen(path, "rb");
        if(!f) {
            return false;
        }

        // Read the header
        std::byte header[0x800];
        if(std::fread(header, sizeof(header), 1, f) != 1) {
            std::fclose(f);
            return false;
        }

        // Close
        std::fclose(f);

        return header_is_valid_for_this_game(header, compressed, map_name);
    }

    extern "C" void do_free_map_handle_bugfix(HANDLE &handle) {
        if(handle) {
            CloseHandle(handle);
            handle = 0;
        }
    }

    // Hold my compressed map...
    static CompressedMapIndex compressed_maps[3] = {};
    static std::size_t last_loaded_map = ~static_cast<std::size_t>(0);

    static void get_tmp_path(const CompressedMapIndex &compressed_map, char *buffer) {
        std::snprintf(buffer, MAX_PATH, "%s\\tmp_%zu.map", get_chimera().get_path(), &compressed_map - compressed_maps);
    }

    static std::size_t size_from_file(const char *path) noexcept;

    static void preload_assets_into_memory_buffer(std::byte *buffer, std::size_t &buffer_used, std::size_t buffer_size, const char *map_name) noexcept;
    static char currently_loaded_map[32] = {};

    extern "C" void do_map_loading_handling(char *map_path, const char *map_name) {
        static bool ui_was_loaded = false;

        // Halo PC might do this here. Just ignore it if it does
        if(std::strcmp(map_name, "bitmaps") == 0 || std::strcmp(map_name, "sounds") == 0 || std::strcmp(map_name, "loc") == 0 || std::strcmp(map_name, SOUNDS_CUSTOM_MAP_NAME) == 0 || std::strcmp(map_name, BITMAPS_CUSTOM_MAP_NAME) == 0 || std::strcmp(map_name, LOC_CUSTOM_MAP_NAME) == 0) {
            return;
        }

        bool ui_map = std::strcmp(map_name, "ui") == 0;

        // If the map is already loaded, go away
        bool do_not_reload = (do_maps_in_ram && ((ui_map && ui_was_loaded) || (std::strcmp(map_name, currently_loaded_map) == 0)));

        const char *new_path = path_for_map(map_name);
        if(new_path) {
            // Check if the map is valid. If not, don't worry about it
            bool compressed;
            bool valid = header_is_valid_for_this_game(new_path, &compressed, map_name);
            if(!valid) {
                char error_message[1024];
                std::snprintf(error_message, sizeof(error_message), "%s.map is not a valid cache file for this version of the game\n\nMap path: %s", map_name, new_path);
                MessageBox(nullptr, error_message, "Invalid map", MB_ICONERROR | MB_OK);
                std::exit(1);
            }

            std::size_t buffer_used = 0;
            std::size_t buffer_size = 0;
            std::byte *buffer = nullptr;

            if(!do_not_reload) {
                compressed_map_file_size = 0;

                if(compressed) {
                    // Get filesystem data
                    struct stat64 s;
                    stat64(new_path, &s);
                    std::uint64_t mtime = s.st_mtime;

                    char tmp_path[MAX_PATH] = {};

                    if(ui_map) {
                        compressed_ui_map_file_size = size_from_file(new_path);
                    }
                    else {
                        compressed_map_file_size = size_from_file(new_path);
                    }

                    // See if we can find it
                    std::size_t new_index = last_loaded_map + 1;
                    if(new_index >= (sizeof(compressed_maps) / sizeof(*compressed_maps))) {
                        new_index = 0;
                    }

                    if(!do_maps_in_ram) {
                        for(auto &map : compressed_maps) {
                            if(std::strcmp(map_name, map.map_name) == 0 && map.date_modified == mtime) {
                                get_tmp_path(map, tmp_path);
                                //console_output("Didn't need to decompress %s -> %s", new_path, tmp_path);
                                last_loaded_map = &map - compressed_maps;
                                if(map_path) {
                                    std::strncpy(map_path, tmp_path, MAX_PATH);
                                }
                                return;
                            }
                        }
                    }

                    // Attempt to decompress
                    auto &compressed_map_to_use = compressed_maps[new_index];
                    get_tmp_path(compressed_maps[new_index], tmp_path);
                    //std::printf("Trying to decompress %s @ %s -> %s...\n", map_name, new_path, tmp_path);
                    auto start = std::chrono::steady_clock::now();

                    // If we're doing maps in RAM, output directly to the region allowed
                    if(do_maps_in_ram) {
                        std::size_t offset;
                        if(ui_map) {
                            buffer_size = UI_SIZE;
                            offset = UI_OFFSET;
                        }
                        else {
                            buffer_size = UI_OFFSET;
                            offset = 0;
                        }

                        buffer = maps_in_ram_region + offset;
                        try {
                            buffer_used = decompress_map_file(new_path, buffer, buffer_size);
                        }
                        catch (std::exception &) {
                            char error_message[256];
                            std::snprintf(error_message, sizeof(error_message), "Failed to load %s: Map is invalid, or not enough space is allocated to load this map", new_path);
                            MessageBox(nullptr, error_message, "Decompression Error", MB_ICONERROR | MB_OK);
                            std::exit(1);
                            return;
                        }

                        if(ui_map) {
                            ui_map_file_size = buffer_used;
                        }
                        else {
                            map_file_size = buffer_used;
                        }
                    }

                    // Otherwise do a map file
                    else {
                        try {
                            decompress_map_file(new_path, tmp_path);
                        }
                        catch (std::exception &) {
                            char error_message[256];
                            std::snprintf(error_message, sizeof(error_message), "Failed to load %s: Map is invalid", new_path);
                            MessageBox(nullptr, error_message, "Decompression Error", MB_ICONERROR | MB_OK);
                            std::exit(1);
                            return;
                        }

                        if(ui_map) {
                            ui_map_file_size = size_from_file(tmp_path);
                        }
                        else {
                            map_file_size = size_from_file(tmp_path);
                        }
                    }
                    auto end = std::chrono::steady_clock::now();

                    // Benchmark
                    if(do_benchmark) {
                        console_output("Decompressed %s in %zu ms", map_name, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
                    }

                    // If we're not doing maps in RAM, change the path to the tmp file, increment loaded maps by 1
                    if(!do_maps_in_ram) {
                        //std::printf("Okay then!\n");
                        new_path = tmp_path;
                        last_loaded_map = new_index;
                        if(last_loaded_map > sizeof(compressed_maps) / sizeof(*compressed_maps)) {
                            last_loaded_map = 0;
                        }
                        compressed_map_to_use.date_modified = mtime;
                        std::strncpy(compressed_map_to_use.map_name, map_name, sizeof(compressed_map_to_use.map_name));
                    }
                }
                else if(do_maps_in_ram) {
                    std::size_t offset;
                    if(ui_map) {
                        buffer_size = UI_SIZE;
                        offset = UI_OFFSET;
                    }
                    else {
                        buffer_size = UI_OFFSET;
                        offset = 0;
                    }

                    std::FILE *f = std::fopen(new_path, "rb");
                    if(!f) {
                        return;
                    }
                    buffer = maps_in_ram_region + offset;
                    std::fseek(f, 0, SEEK_END);
                    buffer_used = std::ftell(f);

                    // Make sure we have enough space for it!
                    if(buffer_used > buffer_size) {
                        char error_message[256];
                        std::snprintf(error_message, sizeof(error_message), "Failed to load %s: Not enough space is allocated in memory to load this map", new_path);
                        MessageBox(nullptr, error_message, "Load Error", MB_ICONERROR | MB_OK);
                        std::exit(1);
                    }

                    std::fseek(f, 0, SEEK_SET);
                    std::fread(buffer, buffer_size, 1, f);
                    std::fclose(f);

                    if(ui_map) {
                        ui_map_file_size = buffer_used;
                    }
                    else {
                        map_file_size = buffer_used;
                    }
                }
                else {
                    if(ui_map) {
                        ui_map_file_size = size_from_file(new_path);
                    }
                    else {
                        map_file_size = size_from_file(new_path);
                    }
                }

                // Load everything from bitmaps.map and sounds.map that can fit
                if(do_maps_in_ram) {
                    preload_assets_into_memory_buffer(buffer, buffer_used, buffer_size, map_name);

                    if(ui_map) {
                        ui_buffer_used = buffer_used;
                        ui_was_loaded = true;
                    }
                    else {
                        std::fill(currently_loaded_map, currently_loaded_map + sizeof(currently_loaded_map), 0);
                        std::strncpy(currently_loaded_map, map_name, sizeof(currently_loaded_map) - 1);
                        map_buffer_used = buffer_used;
                    }
                }
            }

            if(map_path) {
                std::strcpy(map_path, new_path);
            }
        }
    }

    const char *path_for_map(const char *map, bool tmp) noexcept {
        static char path[MAX_PATH];
        if(tmp) {
            for(auto &compressed_map : compressed_maps) {
                if(std::strcmp(map,compressed_map.map_name) == 0) {
                    get_tmp_path(compressed_map, path);
                    return path;
                }
            }
        }

        #define RETURN_IF_FOUND(...) std::snprintf(path, sizeof(path), __VA_ARGS__, map); if(std::filesystem::exists(path)) return path;
        RETURN_IF_FOUND("maps\\%s.map");
        RETURN_IF_FOUND("%s\\maps\\%s.map", get_chimera().get_path());
        return nullptr;
    }

    static std::size_t size_from_file(const char *path) noexcept {
        std::FILE *f = std::fopen(path, "rb");
        std::fseek(f, 0, SEEK_END);
        std::size_t file_size = std::ftell(f);
        std::fclose(f);

        return file_size;
    }

    std::uint32_t maps_in_ram_crc32;

    /*std::uint32_t calculate_crc32_of_current_map_file() noexcept {
        // If we're doing maps in RAM, this was already calculated
        if(do_maps_in_ram) {
            do_map_loading_handling(nullptr, latest_map_loaded_multiplayer);
            return maps_in_ram_crc32;
        }
        // Otherwise do this
        else {
            // Read this
            std::FILE *f = std::fopen(path_for_map(latest_map_loaded_multiplayer, true), "rb");
            MapHeader header = {};
            std::fread(&header, sizeof(header), 1, f);
            std::fseek(f, 0, SEEK_SET);
            auto crc = calculate_crc32_of_map_file(f, header);
            std::fclose(f);
            return crc;
        }
    }*/

    static void preload_assets_into_memory_buffer(std::byte *buffer, std::size_t &buffer_used, std::size_t buffer_size, const char *map_name) noexcept {
        auto start = std::chrono::steady_clock::now();

        // Get tag data info
        std::uint32_t tag_data_address = reinterpret_cast<std::uint32_t>(get_tag_data_address());
        std::byte *tag_data;
        //std::uint32_t tag_data_size;
        auto engine = game_engine();
        int map_engine;

        if(engine == GameEngine::GAME_ENGINE_DEMO) {
            auto &header = *reinterpret_cast<MapHeaderDemo *>(buffer);
            tag_data = buffer + header.tag_data_offset;
            //tag_data_size = header.tag_data_size;
            map_engine = header.engine_type;
        }
        else {
            auto &header = *reinterpret_cast<MapHeader *>(buffer);
            tag_data = buffer + header.tag_data_offset;
            //tag_data_size = header.tag_data_size;
            map_engine = header.engine_type;

            // Calculate the CRC32 if we aren't a UI file
            if(buffer == maps_in_ram_region) {
                if(map_engine == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                    maps_in_ram_crc32 = ~calculate_crc32_of_map_file(nullptr, header);
                }
                else {
                    maps_in_ram_crc32 = crc32_for_stock_map(header.name).value_or(0xFFFFFFFF);
                }
            }
        }
        bool can_load_indexed_tags = map_engine == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;

        // Get the header
        #define TRANSLATE_POINTER(pointer, to_type) reinterpret_cast<to_type>(tag_data + reinterpret_cast<std::uintptr_t>(pointer) - tag_data_address)

        // Open bitmaps.map and sounds.map
        bool using_custom_rsc = (can_load_indexed_tags && engine != GameEngine::GAME_ENGINE_CUSTOM_EDITION);
        std::FILE *bitmaps_file = std::fopen(using_custom_rsc ? BITMAPS_CUSTOM_MAP : path_for_map("bitmaps"), "rb");
        std::FILE *sounds_file = std::fopen(using_custom_rsc ? SOUNDS_CUSTOM_MAP : path_for_map("sounds"), "rb");
        if(!bitmaps_file || !sounds_file) {
            return;
        }

        TagDataHeader &header = *reinterpret_cast<TagDataHeader *>(tag_data);
        auto *tag_array = TRANSLATE_POINTER(header.tag_array, Tag *);

        std::size_t missed_data = 0;

        struct ResourceHeader {
            std::uint32_t type;
            std::uint32_t path_offset;
            std::uint32_t resource_offset;
            std::uint32_t resource_count;
        };

        struct Resource {
            char path[MAX_PATH];
            std::uint32_t data_size;
            std::uint32_t data_offset;
        };

        auto load_resource_map = [](std::vector<Resource> &resources, std::FILE *file) {
            struct ResourceInMap {
                std::uint32_t path_offset;
                std::uint32_t data_size;
                std::uint32_t data_offset;
            };

            ResourceHeader header;
            if(std::fread(&header, sizeof(header), 1, file) != 1) {
                return;
            }
            resources.reserve(header.resource_count);

            // Load resources
            std::vector<ResourceInMap> resources_in_map(header.resource_count);
            std::fseek(file, header.resource_offset, SEEK_SET);
            std::fread(resources_in_map.data(), resources_in_map.size() * sizeof(*resources_in_map.data()), 1, file);

            // Load it all!
            for(std::size_t r = 0; r < header.resource_count; r++) {
                auto &resource = resources.emplace_back();
                resource.data_size = resources_in_map[r].data_size;
                resource.data_offset = resources_in_map[r].data_offset;
                std::fseek(file, header.path_offset, SEEK_SET);
                std::fseek(file, resources_in_map[r].path_offset, SEEK_CUR);
                std::fread(resource.path, sizeof(resource.path), 1, file);
            }
        };

        // Preload any indexed tags, if possible
        std::size_t old_used = buffer_used;
        if(can_load_indexed_tags) {
            auto *cache_file_header = reinterpret_cast<MapHeader *>(buffer);
            // If the tag data is NOT last, we need to make a copy
            std::size_t used_before_preloaded_tag_data = old_used;

            if(cache_file_header->tag_data_offset + cache_file_header->tag_data_size != old_used) {
                auto bytes_to_append = cache_file_header->tag_data_size;
                used_before_preloaded_tag_data = old_used + bytes_to_append;
                if(bytes_to_append > buffer_size || used_before_preloaded_tag_data > buffer_size) {
                    missed_data += bytes_to_append;
                    can_load_indexed_tags = false;
                }
                else {
                    std::memcpy(buffer + old_used, buffer + cache_file_header->tag_data_offset, bytes_to_append);
                    cache_file_header->tag_data_offset = old_used;
                    buffer_used = used_before_preloaded_tag_data;
                }
            }

            if(can_load_indexed_tags) {
                std::vector<Resource> bitmaps;
                std::vector<Resource> sounds;

                load_resource_map(bitmaps, bitmaps_file);
                load_resource_map(sounds, sounds_file);

                for(std::size_t t = 0; t < header.tag_count; t++) {
                    auto &tag = tag_array[t];
                    if(!tag.indexed) {
                        continue;
                    }
                    if(tag.primary_class == TagClassInt::TAG_CLASS_BITMAP) {
                        std::size_t resource_tag_index = reinterpret_cast<std::uint32_t>(tag.data);

                        // If this is screwed up, exit!
                        if(resource_tag_index >= bitmaps.size()) {
                            std::exit(1);
                        }

                        auto &resource = bitmaps[resource_tag_index];

                        // All right!
                        std::size_t bytes_to_read = resource.data_size;
                        std::size_t new_used = buffer_used + bytes_to_read;
                        if(bytes_to_read > buffer_size || new_used > buffer_size) {
                            missed_data += bytes_to_read;
                            continue;
                        }

                        auto *baseline = buffer + buffer_used;
                        std::uint32_t baseline_address = baseline - tag_data + tag_data_address;
                        tag.data = reinterpret_cast<std::byte *>(baseline_address);
                        std::fseek(bitmaps_file, resource.data_offset, SEEK_SET);
                        std::fread(baseline, bytes_to_read, 1, bitmaps_file);

                        // Now fix the pointers for sequence data
                        auto &sequences_count = *reinterpret_cast<std::uint32_t *>(baseline + 0x54);
                        if(sequences_count) {
                            auto &sequences_ptr = *reinterpret_cast<std::uint32_t *>(baseline + 0x58);
                            sequences_ptr += baseline_address;
                            auto *sequences = TRANSLATE_POINTER(sequences_ptr, std::byte *);
                            for(std::uint32_t s = 0; s < sequences_count; s++) {
                                auto *sequence = sequences + s * 0x40;
                                auto &sprites_count = *reinterpret_cast<std::uint32_t *>(sequence + 0x34);
                                if(sprites_count) {
                                    auto &sprites = *reinterpret_cast<std::uint32_t *>(sequence + 0x38);
                                    sprites += baseline_address;
                                }
                            }
                        }

                        // Fix bitmap data
                        auto &bitmap_data_count = *reinterpret_cast<std::uint32_t *>(baseline + 0x60);
                        if(bitmap_data_count) {
                            auto &bitmap_data_ptr = *reinterpret_cast<std::uint32_t *>(baseline + 0x64);
                            bitmap_data_ptr += baseline_address;
                            auto *bitmap_data = TRANSLATE_POINTER(bitmap_data_ptr, std::byte *);
                            for(std::size_t d = 0; d < bitmap_data_count; d++) {
                                auto *bitmap = bitmap_data + d * 0x30;
                                *reinterpret_cast<TagID *>(bitmap + 0x20) = tag.id;
                            }
                        }

                        buffer_used = new_used;
                        tag.indexed = 0;
                    }
                    else if(tag.primary_class == TagClassInt::TAG_CLASS_SOUND) {
                        std::optional<std::size_t> resource_tag_index;
                        const char *path = TRANSLATE_POINTER(tag.path, const char *);

                        for(auto &s : sounds) {
                            if(std::strcmp(path, s.path) == 0) {
                                resource_tag_index = &s - sounds.data();
                                break;
                            }
                        }

                        // If this is screwed up, exit!
                        if(!resource_tag_index.has_value()) {
                            std::exit(1);
                        }

                        // Load sounds
                        auto &resource = sounds[*resource_tag_index];
                        static constexpr std::size_t SOUND_HEADER_SIZE = 0xA4;
                        std::size_t bytes_to_read = resource.data_size - SOUND_HEADER_SIZE;
                        std::size_t new_used = buffer_used + bytes_to_read;
                        if(bytes_to_read > buffer_size || new_used > buffer_size) {
                            missed_data += bytes_to_read;
                            continue;
                        }

                        auto *baseline = buffer + buffer_used;
                        std::uint32_t baseline_address = baseline - tag_data + tag_data_address;
                        std::fseek(sounds_file, resource.data_offset, SEEK_SET);
                        std::byte base_sound_data[SOUND_HEADER_SIZE];
                        std::fread(base_sound_data, sizeof(base_sound_data), 1, sounds_file);
                        std::fread(baseline, bytes_to_read, 1, sounds_file);

                        auto *sound_data = TRANSLATE_POINTER(tag.data, std::byte *);
                        auto &pitch_range_count = *reinterpret_cast<std::uint32_t *>(sound_data + 0x98);

                        // Copy over channel count and format
                        *reinterpret_cast<std::uint16_t *>(sound_data + 0x6C) = *reinterpret_cast<std::uint16_t *>(base_sound_data + 0x6C);
                        *reinterpret_cast<std::uint16_t *>(sound_data + 0x6E) = *reinterpret_cast<std::uint16_t *>(base_sound_data + 0x6E);

                        // Copy over sample rate
                        *reinterpret_cast<std::uint16_t *>(sound_data + 0x6) = *reinterpret_cast<std::uint16_t *>(base_sound_data + 0x6);

                        // Copy over longest permutation length
                        *reinterpret_cast<std::uint32_t *>(sound_data + 0x84) = *reinterpret_cast<std::uint32_t *>(base_sound_data + 0x84);

                        if(pitch_range_count) {
                            *reinterpret_cast<std::uint32_t *>(sound_data + 0x9C) = baseline_address;

                            // Fix the pointers
                            for(std::size_t p = 0; p < pitch_range_count; p++) {
                                auto *pitch_range = baseline + p * 0x48;
                                auto &permutation_count = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x3C);
                                auto &permutation_ptr = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x40);
                                *reinterpret_cast<std::uint32_t *>(pitch_range + 0x34) = 0xFFFFFFFF;
                                *reinterpret_cast<std::uint32_t *>(pitch_range + 0x38) = 0xFFFFFFFF;

                                if(permutation_count) {
                                    permutation_ptr += baseline_address;
                                    auto *permutations = TRANSLATE_POINTER(permutation_ptr, std::byte *);
                                    for(std::size_t r = 0; r < permutation_count; r++) {
                                        auto *permutation = permutations + r * 0x7C;
                                        *reinterpret_cast<std::uint32_t *>(permutation + 0x2C) = 0xFFFFFFFF;
                                        *reinterpret_cast<std::uint32_t *>(permutation + 0x30) = 0;
                                        *reinterpret_cast<std::uint32_t *>(permutation + 0x34) = *reinterpret_cast<std::uint32_t *>(&tag.id);
                                        *reinterpret_cast<std::uint32_t *>(permutation + 0x3C) = *reinterpret_cast<std::uint32_t *>(&tag.id);

                                        auto &mouth_data = *reinterpret_cast<std::uint32_t *>(permutation + 0x54 + 0xC);
                                        auto &subtitle_data = *reinterpret_cast<std::uint32_t *>(permutation + 0x68 + 0xC);

                                        if(mouth_data) {
                                            mouth_data += baseline_address;
                                        }

                                        if(subtitle_data) {
                                            subtitle_data += baseline_address;
                                        }
                                    }
                                }
                            }
                        }

                        buffer_used = new_used;
                        tag.indexed = 0;
                    }
                }

                // Add up the difference
                std::size_t bytes_added = (buffer_used - used_before_preloaded_tag_data);
                cache_file_header->tag_data_size += bytes_added;
            }
        }

        // Preload all of the assets
        for(std::size_t t = 0; t < header.tag_count; t++) {
            auto &tag = tag_array[t];
            if(tag.indexed) {
                continue;
            }
            auto *data = TRANSLATE_POINTER(tag.data, std::byte *);
            if(tag.primary_class == TagClassInt::TAG_CLASS_BITMAP) {
                auto &bitmap_count = *reinterpret_cast<std::uint32_t *>(data + 0x60);
                auto *bitmap_data = TRANSLATE_POINTER(*reinterpret_cast<std::uint32_t *>(data + 0x64), std::byte *);
                for(std::uint32_t b = 0; b < bitmap_count; b++) {
                    auto *bitmap = bitmap_data + b * 48;
                    std::uint8_t &external = *reinterpret_cast<std::uint8_t *>(bitmap + 0xF);
                    if(!(external & 1)) {
                        continue;
                    }

                    // Get metadata
                    std::uint32_t bitmap_size = *reinterpret_cast<std::uint32_t *>(bitmap + 0x1C);
                    std::uint32_t &bitmap_offset = *reinterpret_cast<std::uint32_t *>(bitmap + 0x18);

                    // Don't add this bitmap if we can't fit it
                    std::size_t new_used = buffer_used + bitmap_size;
                    if(bitmap_size > buffer_size || new_used > buffer_size) {
                        missed_data += bitmap_size;
                        continue;
                    }

                    // Read the bitmap data and set the external flag to 0
                    std::fseek(bitmaps_file, bitmap_offset, SEEK_SET);
                    std::fread(buffer + buffer_used, bitmap_size, 1, bitmaps_file);
                    bitmap_offset = buffer_used;
                    buffer_used = new_used;
                    external ^= 1;
                }
            }
            else if(tag.primary_class == TagClassInt::TAG_CLASS_SOUND) {
                auto &pitch_range_count = *reinterpret_cast<std::uint32_t *>(data + 0x98);
                auto *pitch_range_data = TRANSLATE_POINTER(*reinterpret_cast<std::uint32_t *>(data + 0x9C), std::byte *);

                for(std::uint32_t r = 0; r < pitch_range_count; r++) {
                    auto *pitch_range = pitch_range_data + r * 0x48;
                    auto &permutation_count = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x3C);
                    auto *permutation_data = TRANSLATE_POINTER(*reinterpret_cast<std::uint32_t *>(pitch_range + 0x40), std::byte *);
                    for(std::uint32_t p = 0; p < permutation_count; p++) {
                        auto *permutation = permutation_data + 0x7C * p;
                        std::uint32_t &external = *reinterpret_cast<std::uint32_t *>(permutation + 0x44);
                        if(!(external & 1)) {
                            continue;
                        }

                        // Get metadata
                        std::uint32_t sound_size = *reinterpret_cast<std::uint32_t *>(permutation + 0x40);
                        std::uint32_t &sound_offset = *reinterpret_cast<std::uint32_t *>(permutation + 0x48);

                        // Don't add this sound if we can't fit it
                        std::size_t new_used = buffer_used + sound_size;
                        if(sound_size > buffer_size || new_used > buffer_size) {
                            missed_data += sound_size;
                            continue;
                        }

                        // Read the sound data and set the external flag to 0
                        std::fseek(sounds_file, sound_offset, SEEK_SET);
                        std::fread(buffer + buffer_used, sound_size, 1, sounds_file);
                        sound_offset = buffer_used;
                        buffer_used = new_used;
                        external ^= 1;
                    }
                }
            }
        }

        std::fclose(bitmaps_file);
        std::fclose(sounds_file);

        auto end = std::chrono::steady_clock::now();

        if(do_benchmark) {
            if(missed_data) {
                console_error("Failed to preload %.02f MiB due to insufficient capacity", BYTES_TO_MiB(missed_data));
            }

            std::size_t total_preloaded = buffer_used - old_used;
            std::size_t count = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            console_output("Preloaded %.02f MiB from %s in %u ms (%.02f MiB / %.02f MiB - %.01f%%)", BYTES_TO_MiB(total_preloaded), map_name, count, BYTES_TO_MiB(buffer_used), BYTES_TO_MiB(buffer_size), 100.0F * buffer_used / buffer_size);
        }
    }

    extern "C" void map_loading_asm();
    extern "C" void map_loading_server_asm();
    extern "C" void free_map_handle_bugfix_asm();
    extern "C" int on_check_if_map_is_bullshit_asm();

    extern "C" void on_read_map_file_data_asm();
    extern "C" int on_read_map_file_data(HANDLE file_descriptor, std::byte *output, std::size_t size, LPOVERLAPPED overlapped) {
        std::size_t file_offset = overlapped->Offset;
        char file_name[MAX_PATH + 1] = {};
        GetFinalPathNameByHandle(file_descriptor, file_name, sizeof(file_name) - 1, 0);

        char *last_backslash = nullptr;
        char *last_dot = nullptr;
        for(char &c : file_name) {
            if(c == '.') {
                last_dot = &c;
            }
            if(c == '\\' || c == '/') {
                last_backslash = &c;
            }
        }

        // Is the path bullshit?
        if(!last_backslash || !last_dot || last_dot < last_backslash) {
            return 0;
        }

        *last_dot = 0;
        const char *map_name = last_backslash + 1;

        if(std::strcmp(map_name, "bitmaps") == 0 || std::strcmp(map_name, "sounds") == 0 || std::strcmp(map_name, "loc") == 0) {
            // If we're on retail and we are loading from a custom edition map's resource map, handle that
            if(using_custom_map_on_retail()) {
                load_custom_edition_resource_data_in_retail(output, file_offset, size, map_name);
                return 1;
            }
            return 0;
        }

        else if(do_maps_in_ram) {
            if(std::strcmp(map_name, "ui") == 0) {
                std::copy(ui_region + file_offset, ui_region + file_offset + size, output);
                return 1;
            }
            else {
                if(std::strcmp(map_name, currently_loaded_map) != 0) {
                    static char fuck_you[8192] = {};
                    do_map_loading_handling(fuck_you, map_name);
                }
                std::copy(maps_in_ram_region + file_offset, maps_in_ram_region + file_offset + size, output);
                return 1;
            }
        }

        return 0;
    }

    static std::unique_ptr<HACMapDownloader> map_downloader;
    static char download_temp_file[MAX_PATH];

    static char connect_command[256];

    static void initiate_connection() {
        remove_preframe_event(initiate_connection);
        execute_script(connect_command);
    }

    static GenericFont font_to_use = GenericFont::FONT_SMALL;
    static bool retail_fallback = false;
    extern "C" int on_map_load_multiplayer(const char *map) noexcept;

    static void download_frame() {
        char output[128] = {};

        std::int16_t x = -320 + 20;
        std::int16_t width = ((640 / 2) - (640 / 2 + x)) * 2;
        std::int16_t y = 210;
        std::int16_t height = 240 - y;

        ColorARGB color { 1.0F, 1.0F, 1.0F, 1.0F };

        if(server_type() == SERVER_NONE) {
            map_downloader->cancel();
        }

        switch(map_downloader->get_status()) {
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_NOT_STARTED:
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_STARTING:
                std::snprintf(output, sizeof(output), "Connecting to repo...");
                break;
            case HACMapDownloader::DOWNLOAD_STAGE_DOWNLOADING: {
                auto dlnow = map_downloader->get_downloaded_size();
                auto dltotal = map_downloader->get_total_size();

                // Draw the progress
                apply_text("Transferred:", x, y, 100, height, color, font_to_use, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_CENTER);
                char progress_buffer[80];
                std::snprintf(progress_buffer, sizeof(progress_buffer), "%.02f ", dlnow / 1024.0F / 1024.0F);
                apply_text(std::string(progress_buffer), x + 100, y, 100, height, color, font_to_use, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                std::snprintf(progress_buffer, sizeof(progress_buffer), "/ %.02f MiB", dltotal / 1024.0F / 1024.0F);
                apply_text(std::string(progress_buffer), x + 200, y, 150, height, color, font_to_use, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_CENTER);

                std::snprintf(progress_buffer, sizeof(progress_buffer), "%0.02f %%", 100.0F * dlnow / dltotal);
                apply_text(std::string(progress_buffer), x + 350, y, 100, height, color, font_to_use, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                char download_speed_buffer[64];
                auto download_speed = map_downloader->get_download_speed();
                if(download_speed > 1000) {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%.01f MB/s", download_speed / 1000.0F);
                }
                else {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%zu kB/s", download_speed);
                }
                apply_text(download_speed_buffer, x + 450, y, 150, height, color, font_to_use, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                break;
            }
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_COMPLETE: {
                std::snprintf(output, sizeof(output), "Reconnecting...");
                console_output("Download complete. Reconnecting...");

                char to_path[MAX_PATH];
                std::snprintf(to_path, sizeof(to_path), "%s\\maps\\%s.map", get_chimera().get_path(), map_downloader->get_map().c_str());

                std::filesystem::rename(download_temp_file, to_path);

                add_map_to_map_list(map_downloader->get_map().c_str());
                resync_map_list();

                auto &latest_connection = get_latest_connection();
                std::snprintf(connect_command, sizeof(connect_command), "connect \"%s:%u\" \"%s\"", latest_connection.address, latest_connection.port, latest_connection.password);
                execute_script(connect_command);

                add_preframe_event(initiate_connection);
                break;
            }
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_CANCELING:
                std::snprintf(output, sizeof(output), "Canceling download...");
                break;
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_CANCELED:
                std::snprintf(output, sizeof(output), "Download canceled!");
                break;
            default: {
                if(retail_fallback || !custom_edition_maps_supported_on_retail()) {
                    std::snprintf(output, sizeof(output), "Download failed!");
                    console_output("Download failed!");
                    retail_fallback = false;
                    std::snprintf(connect_command, sizeof(connect_command), "connect \"256.256.256.256\" \"\"");
                    add_preframe_event(initiate_connection);
                }
                else {
                    std::snprintf(output, sizeof(output), "Retrying on retail Halo PC repo...");
                    std::string map_name_temp = map_downloader->get_map().c_str();
                    delete map_downloader.release();
                    retail_fallback = true;
                    on_map_load_multiplayer(map_name_temp.c_str());
                }
                break;
            }
        }

        // Draw the progress text
        if(*output) {
            apply_text(output, x, y, width, height, color, font_to_use, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
        }

        if(!map_downloader || map_downloader->is_finished()) {
            delete map_downloader.release();
            remove_preframe_event(download_frame);
            get_chimera().get_signature("server_join_progress_text_sig").rollback();
            get_chimera().get_signature("server_join_established_text_sig").rollback();
            get_chimera().get_signature("esrb_text_sig").rollback();
            retail_fallback = false;
        }
    }

    extern "C" int on_map_load_multiplayer(const char *map) noexcept {
        std::string name_lowercase_copy = map;
        latest_map_loaded_multiplayer = map;
        for(char &c : name_lowercase_copy) {
            c = std::tolower(c);
        }

        if(path_for_map(map) || std::strcmp(map, SOUNDS_CUSTOM_MAP_NAME) == 0 || std::strcmp(map, BITMAPS_CUSTOM_MAP_NAME) == 0 || std::strcmp(map, LOC_CUSTOM_MAP_NAME) == 0 || std::strcmp(map, "sounds") == 0 || std::strcmp(map, "bitmaps") == 0 || std::strcmp(map, "loc") == 0) {
            return 0;
        }

        // Determine what we're downloading from
        const char *game_engine_str;
        switch(game_engine()) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                game_engine_str = "halom";
                break;
            case GameEngine::GAME_ENGINE_RETAIL:
                game_engine_str = (custom_edition_maps_supported_on_retail() && !retail_fallback) ? "halom" : "halor";
                break;
            case GameEngine::GAME_ENGINE_DEMO:
                game_engine_str = "halod";
                break;
            default:
                game_engine_str = nullptr;
                return 1;
        }

        // Can we even do this?
        if(game_engine_str && std::strcmp(game_engine_str, "halor") == 0 && !allow_retail_maps) {
            console_error(localize("chimera_error_cannot_download_retail_maps_1"));
            console_error(localize("chimera_error_cannot_download_retail_maps_2"));
            std::snprintf(connect_command, sizeof(connect_command), "connect \"256.256.256.256\" \"\"");
            add_preframe_event(initiate_connection);
            return 1;
        }

        // Change the server status text
        static Hook hook1, hook2;
        char text_string8[sizeof(download_text_string) / sizeof(*download_text_string)] = {};
        std::snprintf(text_string8, sizeof(text_string8), "Downloading %s.map...", map);
        std::copy(text_string8, text_string8 + sizeof(text_string8), download_text_string);

        auto &server_join_progress_text_sig = get_chimera().get_signature("server_join_progress_text_sig");
        write_jmp_call(server_join_progress_text_sig.data() + 10, hook1, reinterpret_cast<const void *>(on_server_join_text_asm), nullptr, false);

        auto &server_join_established_text_sig = get_chimera().get_signature("server_join_established_text_sig");
        write_jmp_call(server_join_established_text_sig.data() + 5, hook2, reinterpret_cast<const void *>(on_server_join_text_asm), nullptr, false);

        auto &esrb_text_sig = get_chimera().get_signature("esrb_text_sig");
        overwrite(esrb_text_sig.data() + 5, static_cast<std::int16_t>(0x7FFF));
        overwrite(esrb_text_sig.data() + 5 + 7, static_cast<std::int16_t>(0x7FFF));

        // Start downloading (determine where to download to and start!)
        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s\\download.map", get_chimera().get_path());
        map_downloader = std::make_unique<HACMapDownloader>(name_lowercase_copy.c_str(), path, game_engine_str);
        map_downloader->set_preferred_server_node(get_chimera().get_ini()->get_value_long("memory.download_preferred_node"));
        map_downloader->dispatch();

        // Add callbacks so we can check every frame the status
        std::snprintf(download_temp_file, sizeof(download_temp_file), "%s\\download.map", get_chimera().get_path());
        add_preframe_event(download_frame);
        return 1;
    }

    void set_up_map_loading() {
        // Get settings
        auto is_enabled = [](const char *what) -> bool {
            return get_chimera().get_ini()->get_value_bool(what).value_or(false);
        };

        // Bump to 64 MiB
        auto &allocate_main_tag_data_sig = get_chimera().get_signature("memory_allocation_amount_sig");
        auto *allocate_memory_amount = reinterpret_cast<std::uint32_t *>(allocate_main_tag_data_sig.data() + 1);
        auto old_amount = *allocate_memory_amount;
        auto new_amount = old_amount - (23 * 1024 * 1024) + (64 * 1024 * 1024);
        overwrite(allocate_memory_amount, new_amount);

        static Hook hook;
        auto &map_load_path_sig = get_chimera().get_signature("map_load_path_sig");
        write_jmp_call(map_load_path_sig.data(), hook, nullptr, reinterpret_cast<const void *>(get_chimera().feature_present("client") ? map_loading_asm : map_loading_server_asm));

        static Hook hook2;
        auto &create_file_mov_sig = get_chimera().get_signature("create_file_mov_sig");
        write_jmp_call(create_file_mov_sig.data(), hook2, reinterpret_cast<const void *>(free_map_handle_bugfix_asm), nullptr);

        // Make Halo not check the maps if they're bullshit
        static Hook hook3;
        const void *fn;
        auto *map_check_data = get_chimera().get_signature("map_check_sig").data();
        write_function_override(map_check_data, hook3, reinterpret_cast<const void *>(on_check_if_map_is_bullshit_asm), &fn);

        do_benchmark = is_enabled("memory.benchmark");
        do_maps_in_ram = is_enabled("memory.enable_map_memory_buffer");

        // Read MiB
        auto read_mib = [](const char *what, std::size_t default_value) -> std::size_t {
            return get_chimera().get_ini()->get_value_size(what).value_or(default_value) * 1024 * 1024;
        };
        UI_OFFSET = read_mib("memory.map_size", 768);
        UI_SIZE = read_mib("memory.ui_size", 256);

        if(do_maps_in_ram) {
            if(!current_exe_is_laa_patched()) {
                MessageBox(nullptr, "Map memory buffers requires an large address aware-patched executable.", "Error", MB_ICONERROR | MB_OK);
                std::exit(1);
            }

            // Allocate memory, making sure to not do so after the 0x40000000 - 0x50000000 region used for tag data
            for(auto *m = reinterpret_cast<std::byte *>(0x80000000); m < reinterpret_cast<std::byte *>(0xF0000000) && !maps_in_ram_region; m += 0x10000000) {
                maps_in_ram_region = reinterpret_cast<std::byte *>(VirtualAlloc(m, CHIMERA_MEMORY_ALLOCATION_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            }

            if(!maps_in_ram_region) {
                char error_text[256] = {};
                std::snprintf(error_text, sizeof(error_text), "Failed to allocate %.02f GiB for map memory buffers.", BYTES_TO_MiB(CHIMERA_MEMORY_ALLOCATION_SIZE) / 1024.0F);
                MessageBox(nullptr, error_text, "Error", MB_ICONERROR | MB_OK);
                std::exit(1);
            }

            ui_region = maps_in_ram_region + UI_OFFSET;
        }

        // Handle this
        static Hook read_cache_file_data_hook;
        auto &read_map_file_data_sig = get_chimera().get_signature("read_map_file_data_sig");
        write_jmp_call(read_map_file_data_sig.data(), read_cache_file_data_hook, reinterpret_cast<const void *>(on_read_map_file_data_asm), nullptr);

        // Now do map downloading
        static Hook map_load_multiplayer_hook;
        auto &map_load_multiplayer_sig = get_chimera().get_signature("map_load_multiplayer_sig");
        write_jmp_call(map_load_multiplayer_sig.data(), map_load_multiplayer_hook, reinterpret_cast<const void *>(on_map_load_multiplayer_asm));
        on_map_load_multiplayer_fail = map_load_multiplayer_sig.data() + 0x5;

        // Make the meme go away
        if(game_engine() != GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            static Hook land_of_fun_hook;
            auto *preload_map_sig = get_chimera().get_signature("preload_map_sig").data();
            static constexpr SigByte mov_eax_1[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
            write_code_s(preload_map_sig, mov_eax_1);
        }

        // Support Cutdown Edition maps
        if(game_engine() == GameEngine::GAME_ENGINE_RETAIL) {
            set_up_custom_edition_map_support();
        }

        // Should we allow retail maps?
        allow_retail_maps = get_chimera().get_ini()->get_value_bool("memory.download_retail_maps").value_or(false);

        // What font should we use?
        auto *font_fam = get_chimera().get_ini()->get_value("chimera.download_font");
        if(font_fam) {
            font_to_use = generic_font_from_string(font_fam);
        }
    }
    
    std::byte *region_for_current_map() noexcept {
        return maps_in_ram_region;
    }
}
