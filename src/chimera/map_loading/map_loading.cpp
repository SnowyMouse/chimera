#include <filesystem>
#include <sys/stat.h>
#include "map_loading.hpp"
#include "laa.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/map.hpp"
#include "../config/ini.hpp"
#include <chrono>

namespace Invader::Compression {
    void decompress_map_file(const char *input, const char *output);
    void decompress_map_file(const char *input, std::byte *output, std::size_t output_size);
}

namespace Chimera {
    static bool do_maps_in_ram = false;
    static bool do_benchmark = false;

    static std::byte *maps_in_ram_region = nullptr;
    static std::byte *ui_region = nullptr;

    static constexpr std::size_t UI_OFFSET = 1024 * 1024 * 1024;
    static constexpr std::size_t UI_SIZE = 256 * 1024 * 1024;
    static constexpr std::size_t CHIMERA_MEMORY_ALLOCATION_SIZE = (UI_OFFSET + UI_SIZE);

    enum CacheFileEngine : std::uint32_t {
        CACHE_FILE_XBOX = 0x5,
        CACHE_FILE_DEMO = 0x6,
        CACHE_FILE_RETAIL = 0x7,
        CACHE_FILE_CUSTOM_EDITION = 0x261,
        CACHE_FILE_DARK_CIRCLET = 0x1A86,

        CACHE_FILE_DEMO_COMPRESSED = 0x861A0006,
        CACHE_FILE_RETAIL_COMPRESSED = 0x861A0007,
        CACHE_FILE_CUSTOM_EDITION_COMPRESSED = 0x861A0261
    };

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
                if(header_demo_version.engine_type == CACHE_FILE_DEMO) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_demo_version_valid;
                }
                else if(header_full_version.engine_type == CACHE_FILE_DEMO_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
                }
                else {
                    return false;
                }
            case GAME_ENGINE_CUSTOM_EDITION:
                if(header_full_version.engine_type == CACHE_FILE_CUSTOM_EDITION) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_full_version_valid;
                }
                else if(header_full_version.engine_type == CACHE_FILE_CUSTOM_EDITION_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
                }
                else {
                    return false;
                }
            case GAME_ENGINE_RETAIL:
                if(header_full_version.engine_type == CACHE_FILE_RETAIL) {
                    if(compressed) {
                        *compressed = false;
                    }
                    return header_full_version_valid;
                }
                else if(header_full_version.engine_type == CACHE_FILE_RETAIL_COMPRESSED) {
                    if(compressed) {
                        *compressed = true;
                    }
                    return header_full_version_valid;
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
    static CompressedMapIndex compressed_maps[2] = {};
    static std::size_t last_loaded_map = 0;

    static void get_tmp_path(const CompressedMapIndex &compressed_map, char *buffer) {
        std::snprintf(buffer, MAX_PATH, "%s\\tmp_%zu.map", get_chimera().get_path(), &compressed_map - compressed_maps);
    }

    extern "C" void do_map_loading_handling(char *map_path, const char *map_name) {
        const char *new_path = path_for_map(map_name);
        if(new_path) {
            // Check if the map is valid. If not, don't worry about it
            bool compressed;
            bool valid = header_is_valid_for_this_game(new_path, &compressed, map_name);
            if(!valid) {
                std::exit(1);
            }

            if(compressed) {
                // Get filesystem data
                struct stat64 s;
                stat64(new_path, &s);
                std::uint64_t mtime = s.st_mtime;

                char tmp_path[MAX_PATH] = {};

                // See if we can find it
                if(!do_maps_in_ram) {
                    for(auto &map : compressed_maps) {
                        if(std::strcmp(map_name, map.map_name) == 0 && map.date_modified == mtime) {
                            get_tmp_path(map, tmp_path);
                            //console_output("Didn't need to decompress %s -> %s", new_path, tmp_path);
                            last_loaded_map = &map - compressed_maps;
                            std::strncpy(map_path, tmp_path, MAX_PATH);
                            return;
                        }
                    }
                }

                // Attempt to decompress
                std::size_t new_index = !last_loaded_map;
                auto &compressed_map_to_use = compressed_maps[new_index];
                try {
                    get_tmp_path(compressed_maps[new_index], tmp_path);
                    //console_output("Trying to compress %s @ %s -> %s...", map_name, new_path, tmp_path);
                    auto start = std::chrono::steady_clock::now();

                    // If we're doing maps in RAM, output directly to the region allowed
                    if(do_maps_in_ram) {
                        std::size_t size;
                        std::size_t offset;
                        if(std::strcmp(map_name, "ui") == 0) {
                            size = UI_SIZE;
                            offset = UI_OFFSET;
                        }
                        else {
                            size = UI_OFFSET;
                            offset = 0;
                        }
                        Invader::Compression::decompress_map_file(new_path, maps_in_ram_region + offset, size);
                    }

                    // Otherwise do a map file
                    else {
                        Invader::Compression::decompress_map_file(new_path, tmp_path);
                    }
                    auto end = std::chrono::steady_clock::now();

                    // Benchmark
                    if(do_benchmark) {
                        console_output("Decompressed %s in %zu milliseconds\n", map_name, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
                    }

                    // If we're not doing maps in RAM, change the path to the tmp file, increment loaded maps by 1
                    if(!do_maps_in_ram) {
                        std::strcpy(map_path, tmp_path);
                        last_loaded_map++;
                        if(last_loaded_map > sizeof(compressed_maps) / sizeof(*compressed_maps)) {
                            last_loaded_map = 0;
                        }
                        compressed_map_to_use.date_modified = mtime;
                        std::strncpy(compressed_map_to_use.map_name, map_name, sizeof(compressed_map_to_use.map_name));
                    }
                }
                catch (std::exception &e) {
                    compressed_map_to_use = {};
                    //console_output("Failed to decompress %s @ %s: %s", map_name, new_path, e.what());
                    return;
                }
            }

            else if(!do_maps_in_ram) {
                std::size_t size;
                std::size_t offset;
                if(std::strcmp(map_name, "ui") == 0) {
                    size = UI_SIZE;
                    offset = UI_OFFSET;
                }
                else {
                    size = UI_OFFSET;
                    offset = 0;
                }

                std::FILE *f = std::fopen(new_path, "rb");
                if(!f) {
                    return;
                }
                std::fread(maps_in_ram_region + offset, size, 1, f);
                std::fclose(f);
            }

            std::strcpy(map_path, new_path);
        }
    }

    extern "C" void map_loading_asm();
    extern "C" void free_map_handle_bugfix_asm();

    void set_up_map_loading() {
        static Hook hook;
        auto &map_load_path_sig = get_chimera().get_signature("map_load_path_sig");
        write_jmp_call(map_load_path_sig.data(), hook, nullptr, reinterpret_cast<const void *>(map_loading_asm));
        static Hook hook2;
        auto &create_file_mov_sig = get_chimera().get_signature("create_file_mov_sig");
        write_jmp_call(create_file_mov_sig.data(), hook2, reinterpret_cast<const void *>(free_map_handle_bugfix_asm), nullptr);

        // Make Halo not check the maps if they're bullshit
        static unsigned char return_1[6] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 };
        auto *map_check_sig = get_chimera().get_signature("map_check_sig").data();
        overwrite(map_check_sig, return_1, sizeof(return_1));

        // Get settings
        auto is_enabled = [](const char *what) -> bool {
            const char *value = get_chimera().get_ini()->get_value(what);
            return !(!value || std::strcmp(value, "1") != 0);
        };

        do_maps_in_ram = is_enabled("memory.enable_map_memory_buffer");
        do_benchmark = is_enabled("memory.benchmark");

        if(do_maps_in_ram) {
            if(!current_exe_is_laa_patched()) {
                MessageBox(0, "Map memory buffers requires an large address aware-patched executable.", "Error", 0);
                std::exit(1);
            }

            // Allocate memory, making sure to not do so after the 0x40000000 - 0x50000000 region used for tag data
            for(auto *m = reinterpret_cast<std::byte *>(0x80000000); m < reinterpret_cast<std::byte *>(0xF0000000) && !maps_in_ram_region; m += 0x10000000) {
                maps_in_ram_region = reinterpret_cast<std::byte *>(VirtualAlloc(m, CHIMERA_MEMORY_ALLOCATION_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            }

            if(!maps_in_ram_region) {
                MessageBox(0, "Failed to allocate 1.25 GiB for map memory buffers.", "Error", 0);
                std::exit(1);
            }

            ui_region = maps_in_ram_region + UI_OFFSET;
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
}
