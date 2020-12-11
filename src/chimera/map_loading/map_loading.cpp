// SPDX-License-Identifier: GPL-3.0-only

#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <windows.h>
#include <filesystem>
#include <vector>
#include <deque>
#include <cstring>
#include <memory>

#include "map_loading.hpp"
#include "compression.hpp"
#include "crc32.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/tag.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../halo_data/server.hpp"
#include "../signature/signature.hpp"
#include "../config/ini.hpp"
#include "fast_load.hpp"
#include "../chimera.hpp"
#include "../localization/localization.hpp"
#include "../../hac_map_downloader/hac_map_downloader.hpp"
#include "../output/output.hpp"
#include "../output/draw_text.hpp"
#include "../bookmark/bookmark.hpp"
#include "../halo_data/script.hpp"
#include "../event/frame.hpp"
#include "laa.hpp"

using charmander = char; // charmander charrrr!
using charmeleon = char16_t;

namespace Chimera {
    static bool fix_tag(std::vector<std::byte> &tag_data, TagClassInt primary_class, Tag *tag = nullptr) noexcept;
    
    static std::deque<LoadedMap> loaded_maps;
    static std::byte *buffer;
    static std::size_t total_buffer_size = 0;
    static std::size_t max_temp_files = 3;
    static bool do_benchmark = false;
    static bool download_retail_maps = false;
    static bool custom_edition_maps_supported = false;
    static GenericFont download_font = GenericFont::FONT_CONSOLE;
    
    static const char *bitmaps_file = "bitmaps.map";
    static const char *sounds_file = "sounds.map";
    static const char *loc_file = "loc.map";
    
    static const char *custom_bitmaps_file = "custom_bitmaps.map";
    static const char *custom_sounds_file = "custom_sounds.map";
    static const char *custom_loc_file = "custom_loc.map";
    
    enum ResourceOrigin {
        RESOURCE_ORIGIN_CUSTOM_BIT     = 0b0100,
        
        RESOURCE_ORIGIN_BITMAPS        = 0b0000,
        RESOURCE_ORIGIN_SOUNDS         = 0b0001,
        RESOURCE_ORIGIN_LOC            = 0b0010,
        
        RESOURCE_ORIGIN_CUSTOM_BITMAPS = RESOURCE_ORIGIN_BITMAPS | RESOURCE_ORIGIN_CUSTOM_BIT,
        RESOURCE_ORIGIN_CUSTOM_SOUNDS  = RESOURCE_ORIGIN_SOUNDS  | RESOURCE_ORIGIN_CUSTOM_BIT,
        RESOURCE_ORIGIN_CUSTOM_LOC     = RESOURCE_ORIGIN_LOC     | RESOURCE_ORIGIN_CUSTOM_BIT
    };
    
    struct ResourceMetadata {
        ResourceOrigin origin;
        std::uint32_t offset;
        std::byte *data;
        std::size_t size;
    };
    
    static std::vector<ResourceMetadata> metadata;
    
    // Resource maps' tag data
    static std::vector<std::vector<std::byte>> custom_edition_bitmaps_tag_data;
    static std::vector<std::vector<std::byte>> custom_edition_sounds_tag_data;
    static std::vector<std::string> custom_edition_sounds_tag_data_paths;
    static std::vector<std::vector<std::byte>> custom_edition_loc_tag_data;
    static std::vector<bool> custom_edition_loc_tag_data_fixed; // we don't know what tag is what, so we depend on the map to determine that for us
    
    extern "C" {
        void map_loading_asm() noexcept;
        void map_loading_server_asm() noexcept;
        void free_map_handle_bugfix_asm() noexcept;
        void on_check_if_map_is_bullshit_asm() noexcept;
        void on_read_map_file_data_asm() noexcept;
        void on_map_load_multiplayer_asm() noexcept;
        void on_server_join_text_asm() noexcept;
        charmeleon download_text_string[128] = {};
        void *on_map_load_multiplayer_fail = nullptr;
    }
    
    extern "C" bool using_custom_map_on_retail() {
        return get_map_header().engine_type == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION && game_engine() == GameEngine::GAME_ENGINE_RETAIL;
    }
    
    LoadedMap *get_loaded_map(const charmander *name) noexcept {
        for(auto &i : loaded_maps) {
            if(i.name == name) {
                return &i;
            }
        }
        return nullptr;
    }
    
    static void unload_map(LoadedMap *map) {
        auto iterator = loaded_maps.begin();
        auto last = loaded_maps.end();
        while(iterator != last) {
            if(iterator.operator->() == map) {
                loaded_maps.erase(iterator);
                return;
            }
            
            iterator++;
        }
    }
    
    static std::filesystem::path path_for_tmp(std::size_t tmp) {
        charmander tmp_name[64];
        std::snprintf(tmp_name, sizeof(tmp_name), "tmp_%zu.map", tmp);
        return std::filesystem::path(get_chimera().get_path()) / "tmp" / tmp_name;
    }
    
    static std::filesystem::path path_for_map_local(const charmander *map_name) {
        return add_map_to_map_list(map_name).get_file_path();
    }
    
    static std::uint32_t calculate_crc32_of_map_file(const LoadedMap *map) noexcept {
        std::uint32_t crc = 0;
        std::uint32_t tag_data_size;
        std::uint32_t tag_data_offset;
        std::uint32_t current_offset = 0;
        
        auto *maps_in_ram_region = map->memory_location.value_or(nullptr);
        std::FILE *f = (maps_in_ram_region != nullptr) ? nullptr : std::fopen(map->path.string().c_str(), "rb");

        auto seek = [&f, &current_offset](std::size_t offset) {
            if(f) {
                std::fseek(f, offset, SEEK_SET);
            }
            else {
                current_offset = offset;
            }
        };

        auto read = [&f, &current_offset, &maps_in_ram_region](void *where, std::size_t size) {
            if(f) {
                std::fread(where, size, 1, f);
            }
            else {
                std::copy(maps_in_ram_region + current_offset, maps_in_ram_region + size + current_offset, reinterpret_cast<std::byte *>(where));
            }
            current_offset += size;
        };
        
        CacheFileEngine engine;
        union {
            MapHeaderDemo demo_header;
            MapHeader fv_header;
        } header;
        seek(0);
        read(&header, sizeof(header));
        
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO && header.demo_header.is_valid()) {
            engine = header.demo_header.engine_type;
            tag_data_size = header.demo_header.tag_data_size;
            tag_data_offset = header.demo_header.tag_data_offset;
        }
        else {
            engine = header.fv_header.engine_type;
            tag_data_size = header.fv_header.tag_data_size;
            tag_data_offset = header.fv_header.tag_data_offset;
        }
        
        std::uint32_t tag_data_addr;
        switch(engine) {
            case CacheFileEngine::CACHE_FILE_DEMO:
                tag_data_addr = 0x4BF10000;
                break;
            default:
                tag_data_addr = 0x40440000;
                break;
        }

        // Load tag data
        auto tag_data_ptr = std::make_unique<std::byte []>(tag_data_size);
        auto *tag_data = tag_data_ptr.get();
        seek(tag_data_offset);
        read(tag_data, tag_data_size);

        // Get the scenario tag so we can get the BSPs
        auto *scenario_tag = tag_data + (*reinterpret_cast<std::uint32_t *>(tag_data) - tag_data_addr) + (*reinterpret_cast<std::uint32_t *>(tag_data + 4) & 0xFFFF) * 0x20;
        auto *scenario_tag_data = tag_data + (*reinterpret_cast<std::uint32_t *>(scenario_tag + 0x14) - tag_data_addr);

        // CRC32 the BSP(s)
        auto &structure_bsp_count = *reinterpret_cast<std::uint32_t *>(scenario_tag_data + 0x5A4);
        auto *structure_bsps = tag_data + (*reinterpret_cast<std::uint32_t *>(scenario_tag_data + 0x5A4 + 4) - tag_data_addr);
        for(std::size_t b=0;b<structure_bsp_count;b++) {
            auto *bsp = structure_bsps + b * 0x20;
            auto &bsp_offset = *reinterpret_cast<std::uint32_t *>(bsp);
            auto &bsp_size = *reinterpret_cast<std::uint32_t *>(bsp + 4);

            auto bsp_data = std::make_unique<std::byte []>(bsp_size);
            seek(bsp_offset);
            read(bsp_data.get(), bsp_size);
            crc = crc32(crc, bsp_data.get(), bsp_size);
        }

        // Next, CRC32 the model data
        auto &model_vertices_offset = *reinterpret_cast<std::uint32_t *>(tag_data + 0x14);
        auto &vertices_size = *reinterpret_cast<std::uint32_t *>(tag_data + 0x20);

        auto model_vertices = std::make_unique<std::byte []>(vertices_size);
        seek(model_vertices_offset);
        read(model_vertices.get(), vertices_size);
        crc = crc32(crc, model_vertices.get(), vertices_size);

        // Lastly, CRC32 the tag data itself
        crc = crc32(crc, tag_data, tag_data_size);

        return crc;
    }
    
    static void preload_assets(LoadedMap &map) {
        // Set this byte stuff
        std::byte *cursor = *map.memory_location + map.loaded_size;
        auto *end = *map.memory_location + map.buffer_size;
        
        std::printf("Preloading %s...\n\n", map.name.c_str());
        std::printf("Destination: 0x%08zX\n", reinterpret_cast<std::uintptr_t>(*map.memory_location));
        std::printf("     Cursor: 0x%08zX\n", reinterpret_cast<std::uintptr_t>(cursor));
        std::printf("        End: 0x%08zX\n\n", reinterpret_cast<std::uintptr_t>(end));
        
        std::byte *tag_data;
        std::FILE *bitmaps = nullptr;
        std::FILE *sounds = nullptr;
        
        std::uint32_t tag_data_address = reinterpret_cast<std::uint32_t>(get_tag_data_address());
        CacheFileEngine map_engine;
        auto current_engine = game_engine();
        
        if(current_engine == GameEngine::GAME_ENGINE_DEMO) {
            auto &header = *reinterpret_cast<MapHeaderDemo *>(*map.memory_location);
            tag_data = *map.memory_location + header.tag_data_offset;
            map_engine = header.engine_type;
        }
        else {
            auto &header = *reinterpret_cast<MapHeader *>(*map.memory_location);
            tag_data = *map.memory_location + header.tag_data_offset;
            map_engine = header.engine_type;
        }
        
        auto &tag_data_header = *reinterpret_cast<TagDataHeader *>(tag_data);
        
        bool can_load_indexed_tags = map_engine == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
        std::filesystem::path bitmaps_path, sounds_path;
        
        if(map_engine == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION && current_engine != GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            bitmaps_path = std::filesystem::path("maps") / custom_bitmaps_file;
            sounds_path = std::filesystem::path("maps") / custom_sounds_file;
        }
        else {
            bitmaps_path = std::filesystem::path("maps") / bitmaps_file;
            sounds_path = std::filesystem::path("maps") / sounds_file;
        }
        
        // If it's a custom edition map, we ought to first figure out what tags go to what
        const std::uint32_t tag_count = tag_data_header.tag_count;
        std::printf("  Tag count: %zu\n", tag_count);
        
        auto translate_ptr = [&tag_data, &tag_data_address](auto ptr) -> std::byte * {
            return tag_data + (reinterpret_cast<uintptr_t>(ptr) - tag_data_address);
        };
        
        Tag *tag_array = reinterpret_cast<Tag *>(translate_ptr(tag_data_header.tag_array));
        std::printf("  Tag array: 0x%08zX (0x%08zX in buffer)\n\n", reinterpret_cast<std::uintptr_t>(tag_data_header.tag_array), reinterpret_cast<std::uintptr_t>(tag_array));
        
        auto translate_index = [](auto index, auto &of_what) -> std::byte * {
            auto index_val = reinterpret_cast<std::uint32_t>(index);
            if(index_val >= of_what.size()) {
                MessageBox(nullptr, "Map could not be loaded due to an invalid index", "Failed to load map", MB_OK | MB_ICONERROR);
                std::exit(EXIT_FAILURE);
            }
            return of_what[index_val].data();
        };
        
        std::vector<bool> was_indexed(tag_count);
        
        if(can_load_indexed_tags) {
            for(std::uint32_t i = 0; i < tag_count; i++) {
                if(tag_array[i].indexed) {
                    was_indexed[i] = true;
                    
                    switch(tag_array[i].primary_class) {
                        case TagClassInt::TAG_CLASS_BITMAP:
                            tag_array[i].indexed = 0;
                            tag_array[i].data = translate_index(tag_array[i].data, custom_edition_bitmaps_tag_data);
                            break;
                            
                        case TagClassInt::TAG_CLASS_SOUND: {
                            // tag_array[i].indexed = 0;
                            
                            // Do this fucking meme
                            const char *path = reinterpret_cast<char *>(translate_ptr(tag_array[i].path));
                            std::optional<std::size_t> path_index;
                            
                            // Set this stuff
                            for(auto &s : custom_edition_sounds_tag_data_paths) {
                                if(s == path) {
                                    path_index = &s - custom_edition_sounds_tag_data_paths.data();
                                    break;
                                }
                            }
                            
                            // Get these pointers ready
                            auto *data = translate_index(path_index.value_or(custom_edition_sounds_tag_data.size()), custom_edition_sounds_tag_data);
                            auto *old_data = translate_ptr(tag_array[i].data);
                            
                            // Set this value here
                            *reinterpret_cast<std::byte **>(old_data + 0x98 + 0x4) = data + 0xA4;
                            
                            // Fix our shit
                            // fix_tag(custom_edition_loc_tag_data[reinterpret_cast<std::uint32_t>(tag_array[i].data)], tag_array[i].primary_class, tag_array + i);
                            
                            break;
                        }
                            
                        default:
                            tag_array[i].indexed = 0;
                            fix_tag(custom_edition_loc_tag_data[reinterpret_cast<std::uint32_t>(tag_array[i].data)], tag_array[i].primary_class, nullptr);
                            tag_array[i].data = translate_index(tag_array[i].data, custom_edition_loc_tag_data);
                            break;
                    }
                }
            }
        }
        
        auto preload_asset_maybe = [&cursor, &end](std::uint32_t offset, std::uint32_t size, std::FILE *from, ResourceOrigin origin) -> bool {
            // Too big? Nope
            if(cursor + size > end || cursor + size < cursor) {
                return false;
            }
            
            // Already present? No need then.
            for(auto &i : metadata) {
                if(i.offset == offset && i.size == size && i.origin == origin) {
                    return true;
                }
            }
            
            // Navigate to this
            std::fseek(from, offset, SEEK_SET);
            std::fread(cursor, size, 1, from);
            
            // Set asset data
            auto &new_asset = metadata.emplace_back();
            new_asset.data = cursor;
            new_asset.origin = origin;
            new_asset.offset = offset;
            new_asset.size = size;
            
            // Increment the cursor
            cursor += size;
        
            return true;
        };
        
        auto preload_all_tags_of_class = [&preload_asset_maybe, &tag_count, &tag_array, &was_indexed, &tag_data_address, &tag_data, &can_load_indexed_tags, &bitmaps](TagClassInt class_int) {
            for(std::uint32_t i = 0; i < tag_count; i++) {
                auto &tag = tag_array[i];
                
                auto get_real_address = [&i, &was_indexed, &tag_data_address, &tag_data](auto address) -> std::byte * {
                    if(was_indexed[i]) {
                        return reinterpret_cast<std::byte *>(address);
                    }
                    else {
                        auto result = tag_data + (reinterpret_cast<std::uintptr_t>(address) - tag_data_address);
                        return result;
                    }
                };
                
                if(tag.primary_class == class_int) {
                    switch(class_int) {
                        case TagClassInt::TAG_CLASS_BITMAP: {
                            auto *td = get_real_address(tag.data);
                            
                            auto *bitmap_data = get_real_address(*reinterpret_cast<std::uint32_t *>(td + 0x60 + 0x4));
                            std::uint32_t bitmap_count = *reinterpret_cast<std::uint32_t *>(td + 0x60);
                            
                            for(std::uint32_t bd = 0; bd < bitmap_count; bd++) {
                                auto *bitmap = bitmap_data + bd * 0x64;
                                
                                std::uint8_t &external = *reinterpret_cast<std::uint8_t *>(bitmap + 0xF);
                                
                                // Ignore internal tags
                                if(!(external & 1)) {
                                    continue;
                                }
                                
                                std::uint32_t bitmap_size = *reinterpret_cast<std::uint32_t *>(bitmap + 0x1C);
                                std::uint32_t bitmap_offset = *reinterpret_cast<std::uint32_t *>(bitmap + 0x18);
                                
                                preload_asset_maybe(bitmap_offset, bitmap_size, bitmaps, can_load_indexed_tags ? ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_BITMAPS : ResourceOrigin::RESOURCE_ORIGIN_BITMAPS);
                            }
                            
                            break;
                        }
                        case TagClassInt::TAG_CLASS_SOUND: {
                            auto *td = get_real_address(tag.data);
                            
                            auto pitch_range_count = *reinterpret_cast<std::uint32_t *>(td + 0x98);
                            auto *pitch_ranges = *reinterpret_cast<std::byte **>(td + 0x98 + 0x4);
                            
                            for(std::uint32_t pr = 0; pr < pitch_range_count; pr++) {
                                auto *pitch_range = pitch_ranges + pr * 0x48;
                                
                                auto permutation_count = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x3C);
                                auto *permutation_ptr = *reinterpret_cast<std::byte **>(pitch_range + 0x3C + 0x4);
                                
                                for(std::uint32_t pe = 0; pe < permutation_count; pe++) {
                                    auto *permutation = permutation_ptr + pe * 0x7C;
                                    
                                    std::uint8_t &external = *reinterpret_cast<std::uint8_t *>(permutation + 0x44);
                                    
                                    // Ignore internal tags
                                    if(!(external & 1)) {
                                        continue;
                                    }
                                    
                                    std::uint32_t sound_offset = *reinterpret_cast<std::uint32_t *>(permutation + 0x48);
                                    std::uint32_t sound_size = *reinterpret_cast<std::uint32_t *>(permutation + 0x40);
                                    
                                    preload_asset_maybe(sound_offset, sound_size, bitmaps, can_load_indexed_tags ? ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_SOUNDS : ResourceOrigin::RESOURCE_ORIGIN_SOUNDS);
                                }
                            }
                            
                            break;
                        }
                        
                        default: break;
                    }
                }
            }
        };
        
        bitmaps = std::fopen(bitmaps_path.string().c_str(), "rb");
        sounds = std::fopen(sounds_path.string().c_str(), "rb");
        
        // If stuff's missing... um... how? Anyway give up.
        if(!bitmaps || !sounds) {
            goto done_preloading_assets;
        }
        
        // Prioritize loading sounds over bitmaps
        // preload_all_tags_of_class(TagClassInt::TAG_CLASS_SOUND);
        preload_all_tags_of_class(TagClassInt::TAG_CLASS_BITMAP);
        
        // Cleanup
        done_preloading_assets:
        std::printf("Preloading done!\n");
        
        map.loaded_size = (cursor - *map.memory_location);
        map.buffer_size = end - cursor;
        if(bitmaps) {
            std::fclose(bitmaps);
        }
        if(sounds) {
            std::fclose(sounds);
        }
    }
    
    std::unique_ptr<HACMapDownloader> map_downloader;
    
    // Load the map
    LoadedMap *load_map(const charmander *map_name) {
        // Get the map path
        auto map_path = path_for_map_local(map_name);
        auto timestamp = std::filesystem::last_write_time(map_path);
        std::size_t actual_size;
        
        // Is the map already loaded?
        for(auto &i : loaded_maps) {
            if(i.name == map_name) {
                // If the map is loaded and it hasn't been modified, do not reload it
                if(i.timestamp == timestamp) {
                    // Move it to the front of the array, though
                    auto copy = i;
                    unload_map(&i);
                    return &loaded_maps.emplace_back(copy);
                }
                
                // Remove the map from the list; we're reloading it
                unload_map(&i);
                break;
            }
        }
        
        // Add our map to the list
        std::size_t size = std::filesystem::file_size(map_path);
        LoadedMap new_map;
        new_map.name = map_name;
        new_map.timestamp = timestamp;
        new_map.file_size = size;
        new_map.decompressed_size = size;
        new_map.path = map_path;
        
        // Load it
        std::FILE *f = nullptr;
        auto invalid = [&f, &map_name](const charmander *error) {
            // Close first
            if(f) {
                std::fclose(f);
            }
            
            charmander title[128];
            std::snprintf(title, sizeof(title), "Failed to load %s", map_name);
            MessageBox(nullptr, error, title, MB_ICONERROR | MB_OK);
            std::exit(1);
        };
        
        // Attempt to load directly into memory
        f = std::fopen(map_path.string().c_str(), "rb");
        if(!f) {
            invalid("Map could not be opened");
        }
        
        // Load the thing
        union {
            MapHeaderDemo demo_header;
            MapHeader fv_header;
        } header;
        
        if(std::fread(&header, sizeof(header), 1, f) != 1) {
            invalid("Failed to read map header into memory from the file");
        }
        
        bool needs_decompressed = false;
        
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO && header.demo_header.is_valid()) {
            switch(header.demo_header.engine_type) {
                case CacheFileEngine::CACHE_FILE_DEMO:
                    break;
                default:
                    invalid("Invalid map type");
            }
        }
        else if(header.fv_header.is_valid()) {
            switch(header.fv_header.engine_type) {
                case CacheFileEngine::CACHE_FILE_RETAIL:
                case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                    break;
                    
                case CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                case CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                    size = header.fv_header.file_size;
                    needs_decompressed = true;
                    break;
                    
                default:
                    invalid("Invalid map type");
            }
        }
        else {
            invalid("Header is invalid");
        }
        
        // Do we have enough space to load into memory?
        bool tmp_file = true;
        if(total_buffer_size > 0) {
            std::size_t remaining_buffer_size = total_buffer_size;
            auto *buffer_location = buffer;
            
            // If it's not ui.map, then we need to ensure ui.map is always loaded
            if(std::strcmp(map_name, "ui") != 0) {
                for(auto &i : loaded_maps) {
                    if(i.name == "ui" && i.memory_location.has_value()) {
                        auto size = i.loaded_size;
                        remaining_buffer_size -= size;
                        buffer_location += size;
                        break;
                    }
                }
            }
            
            // We do!
            if(remaining_buffer_size >= size) {
                if(needs_decompressed) {
                    try {
                        actual_size = decompress_map_file(map_path.string().c_str(), buffer_location, size);
                    }
                    catch (std::exception &) {
                        invalid("Failed to read map");
                    }
                    if(actual_size != size) {
                        invalid("Size in map is incorrect");
                    }
                    new_map.decompressed_size = actual_size;
                }
                else {
                    std::fseek(f, 0, SEEK_SET);
                    if(std::fread(buffer_location, size, 1, f) != 1) {
                        invalid("Failed to read map");
                    }
                }
                
                // Next, we need to remove any loaded map we may have, not including ui.map
                for(auto &i : loaded_maps) {
                    if((i.name != "ui" || std::strcmp(map_name, "ui") == 0) && i.memory_location.has_value()) {
                        unload_map(&i);
                        break;
                    }
                }
                
                // We're done with this
                std::fclose(f);
                f = nullptr;
                
                // Find all metadata after the thing we're loading to
                std::size_t metadata_size = metadata.size();
                for(std::size_t m = 0; m < metadata_size; m++) {
                    auto &md = metadata[m];
                    if(md.data >= buffer_location) {
                        metadata.erase(metadata.begin() + m);
                        m--;
                        metadata_size--;
                        continue;
                    }
                }
                
                new_map.loaded_size = size;
                new_map.memory_location = buffer_location;
                new_map.buffer_size = remaining_buffer_size;
                
                tmp_file = false;
            }
        }
        
        if(tmp_file) {
            // Nothing more to do with this
            std::fclose(f);
            f = nullptr;
            
            // Does it need decompressed?
            if(needs_decompressed) {
                // First we need to reserve a temp file
                if(max_temp_files == 0) {
                    invalid("Temporary files are disabled");
                }
                
                // Go through each possible index. See if we can reserve something.
                for(std::size_t t = 0; t < max_temp_files; t++) {
                    bool found = false;
                    for(auto &i : loaded_maps) {
                        if(i.tmp_file == t) {
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        new_map.tmp_file = t;
                        break;
                    }
                }
                
                // No? We need to take one then. Find the lowest-index temp file and remove it
                if(!new_map.tmp_file.has_value()) {
                    for(auto &i : loaded_maps) {
                        if(i.tmp_file.has_value()) {
                            new_map.tmp_file = i.tmp_file;
                            unload_map(&i);
                            break;
                        }
                    }
                }
                
                new_map.path = path_for_tmp(new_map.tmp_file.value());
                
                // Decompress it
                try {
                    actual_size = decompress_map_file(map_path.string().c_str(), new_map.path.string().c_str());
                }
                catch (std::exception &) {
                    invalid("Failed to read map");
                }
                if(actual_size != size) {
                    invalid("Size in map is incorrect");
                }
                
                new_map.decompressed_size = size;
            }
            
            // No action needs to be taken
            else {
                new_map.path = map_path;
            }
        }
        
        // Calculate CRC32
        get_map_entry(new_map.name.c_str())->crc32 = ~calculate_crc32_of_map_file(&new_map);
        new_map.absolute_path = std::filesystem::absolute(new_map.path);
        
        // Preload if need be
        if(!tmp_file) {
            preload_assets(new_map);
        }
        
        return &loaded_maps.emplace_back(new_map);
    }
    
    static bool retail_fallback = false;
    static charmander download_temp_file[1024];
    static charmander connect_command[1024];
    
    extern "C" int on_map_load_multiplayer(const charmander *map) noexcept;

    extern "C" void do_free_map_handle_bugfix(HANDLE &handle) {
        if(handle) {
            CloseHandle(handle);
            handle = 0;
        }
    }
    
    extern "C" void do_map_loading_handling(charmander *map_path, const charmander *map_name) {
        std::strcpy(map_path, load_map(map_name)->path.string().c_str());
    }
    
    static void initiate_connection() {
        remove_preframe_event(initiate_connection);
        execute_script(connect_command);
    }
    
    static void download_frame() {
        charmander output[128] = {};

        std::int16_t x = -320 + 20;
        std::int16_t width = ((640 / 2) - (640 / 2 + x)) * 2;
        std::int16_t y = 210;
        std::int16_t height = 240 - y;

        ColorARGB color { 1.0F, 1.0F, 1.0F, 1.0F };

        if(server_type() == ServerType::SERVER_NONE) {
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
                apply_text("Transferred:", x, y, 100, height, color, download_font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_CENTER);
                charmander progress_buffer[80];
                std::snprintf(progress_buffer, sizeof(progress_buffer), "%.02f ", dlnow / 1024.0F / 1024.0F);
                apply_text(std::string(progress_buffer), x + 100, y, 100, height, color, download_font, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                std::snprintf(progress_buffer, sizeof(progress_buffer), "/ %.02f MiB", dltotal / 1024.0F / 1024.0F);
                apply_text(std::string(progress_buffer), x + 200, y, 150, height, color, download_font, FontAlignment::ALIGN_LEFT, TextAnchor::ANCHOR_CENTER);

                std::snprintf(progress_buffer, sizeof(progress_buffer), "%0.02f %%", 100.0F * dlnow / dltotal);
                apply_text(std::string(progress_buffer), x + 350, y, 100, height, color, download_font, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                charmander download_speed_buffer[64];
                auto download_speed = map_downloader->get_download_speed();
                if(download_speed > 1000) {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%.01f MB/s", download_speed / 1000.0F);
                }
                else {
                    std::snprintf(download_speed_buffer, sizeof(download_speed_buffer), "%zu kB/s", download_speed);
                }
                apply_text(download_speed_buffer, x + 450, y, 150, height, color, download_font, FontAlignment::ALIGN_RIGHT, TextAnchor::ANCHOR_CENTER);

                break;
            }
            case HACMapDownloader::DownloadStage::DOWNLOAD_STAGE_COMPLETE: {
                std::snprintf(output, sizeof(output), "Reconnecting...");
                console_output("Download complete. Reconnecting...");

                charmander to_path[MAX_PATH];
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
                if(retail_fallback || !custom_edition_maps_supported) {
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
            apply_text(output, x, y, width, height, color, download_font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
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

    extern "C" int on_map_load_multiplayer(const charmander *map) noexcept {
        std::string name_lowercase_copy = map;
        for(charmander &c : name_lowercase_copy) {
            c = std::tolower(c);
        }

        // Does it exist?
        if(get_map_entry(map)) {
            return 0;
        }

        // Determine what we're downloading from
        const charmander *game_engine_str;
        switch(game_engine()) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                game_engine_str = "halom";
                break;
            case GameEngine::GAME_ENGINE_RETAIL:
                game_engine_str = (custom_edition_maps_supported && !retail_fallback) ? "halom" : "halor";
                break;
            case GameEngine::GAME_ENGINE_DEMO:
                game_engine_str = "halod";
                break;
            default:
                game_engine_str = nullptr;
                return 1;
        }

        // Can we even do this?
        if(game_engine_str && std::strcmp(game_engine_str, "halor") == 0 && !download_retail_maps) {
            console_error(localize("chimera_error_cannot_download_retail_maps_1"));
            console_error(localize("chimera_error_cannot_download_retail_maps_2"));
            std::snprintf(connect_command, sizeof(connect_command), "connect \"256.256.256.256\" \"\"");
            add_preframe_event(initiate_connection);
            return 1;
        }

        // Change the server status text
        static Hook hook1, hook2;
        charmander text_string8[sizeof(download_text_string) / sizeof(*download_text_string)] = {};
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
        charmander path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s\\download.map", get_chimera().get_path());
        map_downloader = std::make_unique<HACMapDownloader>(name_lowercase_copy.c_str(), path, game_engine_str);
        map_downloader->set_preferred_server_node(get_chimera().get_ini()->get_value_long("memory.download_preferred_node"));
        map_downloader->dispatch();

        // Add callbacks so we can check every frame the status
        std::snprintf(download_temp_file, sizeof(download_temp_file), "%s\\download.map", get_chimera().get_path());
        add_preframe_event(download_frame);
        return 1;
    }
    
    extern "C" int on_read_map_file_data(HANDLE file_descriptor, std::byte *output, std::size_t size, LPOVERLAPPED overlapped) {
        std::size_t file_offset = overlapped->Offset;
        
        // Get the name
        charmander file_path_chars[MAX_PATH + 1] = {};
        GetFinalPathNameByHandle(file_descriptor, file_path_chars, sizeof(file_path_chars) - 1, VOLUME_NAME_NONE);
        auto file_path = std::filesystem::path(file_path_chars);
        
        // If it's not a .map file, forget about it
        charmander file_path_extension[5] = {};
        std::snprintf(file_path_extension, sizeof(file_path_extension), "%s", file_path.extension().string().c_str());
        for(auto &fpe : file_path_extension) {
            fpe = std::tolower(fpe);
        }
        
        // Get the resource file if possible
        auto file_name = file_path.filename();
        std::optional<ResourceOrigin> origin;
        
        if(file_name == custom_bitmaps_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_BITMAPS;
        }
        else if(file_name == custom_sounds_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_SOUNDS;
        }
        else if(file_name == custom_loc_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_LOC;
        }
        else if(file_name == bitmaps_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_BITMAPS;
        }
        else if(file_name == sounds_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_SOUNDS;
        }
        else if(file_name == loc_file) {
            origin = ResourceOrigin::RESOURCE_ORIGIN_LOC;
        }

        if(origin.has_value()) {
            // load this map now!
            load_map(get_map_name());
            
            // If we're on retail and we are loading from a custom edition map's resource map, handle that
            if(using_custom_map_on_retail() || game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
                origin = static_cast<ResourceOrigin>(*origin | ResourceOrigin::RESOURCE_ORIGIN_CUSTOM_BIT);
            }
            
            // Copy it in?
            for(auto &md : metadata) {
                if(md.origin == origin && file_offset == md.offset && size >= md.size) {
                    std::memcpy(output, md.data, size);
                    return 1;
                }
            }
            
            return 0;
        }

        else {
            // Get the path
            auto absolute_path = std::filesystem::absolute(file_path);
            
            // Load the map if it's not loaded
            load_map(file_path.stem().string().c_str());
            
            // Do it
            for(auto &i : loaded_maps) {
                if(i.absolute_path == absolute_path && i.memory_location.has_value()) {
                    std::memcpy(output, *i.memory_location + file_offset, size);
                    return 1;
                }
            }
        }

        return 0;
    }
    
    static bool fix_tag(std::vector<std::byte> &tag_data, TagClassInt primary_class, Tag *tag) noexcept {
        std::byte *base = tag_data.data();
        auto base_offset = reinterpret_cast<std::uint32_t>(tag_data.data());
        
        // If we're loc, mark as such
        for(auto &i : custom_edition_loc_tag_data) {
            if(&i == &tag_data) {
                std::size_t index = &i - custom_edition_loc_tag_data.data();
                if(custom_edition_loc_tag_data_fixed[index]) {
                    return true;
                }
                else {
                    custom_edition_loc_tag_data_fixed[index] = true;
                }
            }
        }
        
        #define INCREMENT_IF_NECESSARY(what) if(tag == nullptr) { \
            auto &ptr = *reinterpret_cast<std::byte **>(what); \
            if(ptr != 0) { \
                ptr += base_offset; \
            } \
        }
        
        switch(primary_class) {
            case TagClassInt::TAG_CLASS_BITMAP: {
                INCREMENT_IF_NECESSARY(base + 0x54 + 0x4);
                INCREMENT_IF_NECESSARY(base + 0x60 + 0x4);
                auto sequence_count = *reinterpret_cast<std::uint32_t *>(base + 0x54);
                auto *sequences = *reinterpret_cast<std::byte **>(base + 0x54 + 0x4);
                for(std::uint32_t s = 0; s < sequence_count; s++) {
                    INCREMENT_IF_NECESSARY(sequences + s * 64 + 0x34 + 0x4);
                }
                break;
            }
            case TagClassInt::TAG_CLASS_SOUND: {
                // Let's begin.
                auto pitch_range_count = *reinterpret_cast<std::uint32_t *>(base + 0x98);

                // Fix the currently loaded tag
                if(tag) {
                    // Copy over channel count and format
                    *reinterpret_cast<std::uint16_t *>(tag->data + 0x6C) = *reinterpret_cast<std::uint16_t *>(base + 0x6C);
                    *reinterpret_cast<std::uint16_t *>(tag->data + 0x6E) = *reinterpret_cast<std::uint16_t *>(base + 0x6E);

                    // Copy over sample rate
                    *reinterpret_cast<std::uint16_t *>(tag->data + 0x6) = *reinterpret_cast<std::uint16_t *>(base + 0x6);

                    // Copy over longest permutation length
                    *reinterpret_cast<std::uint32_t *>(tag->data + 0x84) = *reinterpret_cast<std::uint32_t *>(base + 0x84);
                }
                
                // Add this to account for the header
                base_offset += 0xA4;
                base += 0xA4;
                
                // Set this pointer
                if(tag) {
                    *reinterpret_cast<std::uint32_t *>(tag->data + 0xA0) = base_offset;
                }

                for(std::uint32_t p = 0; p < pitch_range_count; p++) {
                    auto *pitch_range = base + p * 72;
                    INCREMENT_IF_NECESSARY(pitch_range + 0x3C + 0x4);
                    auto permutation_count = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x3C);
                    auto permutations = *reinterpret_cast<std::byte **>(pitch_range + 0x3C + 0x4);

                    *reinterpret_cast<std::uint32_t *>(pitch_range + 0x34) = 0xFFFFFFFF;
                    *reinterpret_cast<std::uint32_t *>(pitch_range + 0x38) = 0xFFFFFFFF;

                    for(std::uint32_t pe = 0; pe < permutation_count; pe++) {
                        auto *permutation = permutations + pe * 124;

                        *reinterpret_cast<std::uint32_t *>(permutation + 0x2C) = 0xFFFFFFFF;
                        *reinterpret_cast<std::uint32_t *>(permutation + 0x30) = 0;

                        INCREMENT_IF_NECESSARY(permutation + 0x54 + 0xC);
                        INCREMENT_IF_NECESSARY(permutation + 0x68 + 0xC);
                        
                        if(tag) {
                            *reinterpret_cast<TagID *>(permutation + 0x34) = tag->id;
                            *reinterpret_cast<TagID *>(permutation + 0x3C) = tag->id;
                        }
                        
                        *reinterpret_cast<std::uint32_t *>(permutation + 0x2C) = 0xFFFFFFFF;
                    }
                }
                break;
            }
            case TagClassInt::TAG_CLASS_FONT: {
                INCREMENT_IF_NECESSARY(base + 0x7C + 0x4);
                INCREMENT_IF_NECESSARY(base + 0x30 + 0x4);
                INCREMENT_IF_NECESSARY(base + 0x88 + 0xC);
                std::uint32_t table_count = *reinterpret_cast<std::uint32_t *>(base + 0x30);
                auto *tables = *reinterpret_cast<std::byte **>(base + 0x30 + 0x4);
                for(std::uint32_t t = 0; t < table_count; t++) {
                    INCREMENT_IF_NECESSARY(tables + t * 12 + 0x0 + 0x4);
                }
                break;
            }
            case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST: {
                INCREMENT_IF_NECESSARY(base + 0x0 + 0x4);
                std::uint32_t string_count = *reinterpret_cast<std::uint32_t *>(base + 0x0);
                std::byte *strings = *reinterpret_cast<std::byte **>(base + 0x0 + 0x4);
                for(std::uint32_t s = 0; s < string_count; s++) {
                    INCREMENT_IF_NECESSARY(strings + s * 20 + 0x0 + 0xC);
                }
                break;
            }
            case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT: {
                INCREMENT_IF_NECESSARY(base + 0x0 + 0xC);
                INCREMENT_IF_NECESSARY(base + 0x14 + 0x4);
                INCREMENT_IF_NECESSARY(base + 0x20 + 0x4);
                break;
            }
            default:
                break;
        }
        
        return true;
    }
    
    static bool set_up_custom_edition_map_support() {
        std::FILE *bitmaps = nullptr;
        std::FILE *sounds = nullptr;
        std::FILE *loc = nullptr;
        auto is_custom_edition = game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION;
        
        auto maps_folder = std::filesystem::path("maps");
        
        bitmaps = std::fopen((maps_folder / custom_bitmaps_file).string().c_str(), "rb");
        sounds = std::fopen((maps_folder / custom_sounds_file).string().c_str(), "rb");
        loc = std::fopen((maps_folder / custom_loc_file).string().c_str(), "rb");
        
        auto try_close = [](auto *&what) {
            if(what) {
                std::fclose(what);
            }
            what = nullptr;
        };
        
        if(is_custom_edition) {
            if(bitmaps && sounds && loc) {
                // TODODILE: make Halo Custom Edition load these files instead
                std::printf("fixme:set_up_custom_edition_map_support:custom_* maps are not yet supported\n");
                goto spaghetti_monster;
            }
            else {
                spaghetti_monster:
                
                // Fail on Custom Edition - try opening the normal ones
                try_close(bitmaps);
                try_close(sounds);
                try_close(loc);
                
                bitmaps = std::fopen((maps_folder / bitmaps_file).string().c_str(), "rb");
                sounds = std::fopen((maps_folder / sounds_file).string().c_str(), "rb");
                loc = std::fopen((maps_folder / loc_file).string().c_str(), "rb");
            }
        }
        
        // Fail
        if(!bitmaps || !sounds || !loc) {
            try_close(bitmaps);
            try_close(sounds);
            try_close(loc);
            
            if(is_custom_edition) {
                MessageBox(nullptr, "Missing bitmaps.map/sounds.map/loc.map or custom_bitmaps.map/custom_sounds.map/custom_loc.map from your maps folder.", "Files missing or unreadable", MB_OK | MB_ICONERROR);
                std::exit(EXIT_FAILURE);
            }
            return false;
        }
        
        auto read_all_tags = [](std::FILE *from, auto &to_what, std::vector<std::string> *to_what_paths) -> bool {
            struct {
                std::uint32_t type;
                std::uint32_t paths;
                std::uint32_t resources;
                std::uint32_t resource_count;
            } header;
            
            #define read_at(offset, what) \
                std::fseek(from, offset, SEEK_SET); \
                if(std::fread(&what, sizeof(what), 1, from) != 1) { \
                    return false; \
                }
            
            // Read to this address
            read_at(0, header);
            
            // Read every other?
            bool read_every_other = &to_what != &custom_edition_loc_tag_data;
            
            // Reserve
            to_what.reserve((header.resource_count + 1) / (read_every_other ? 2 : 1));
            
            // Read
            for(std::uint32_t i = 0; i < header.resource_count; i++) {
                struct {
                    std::uint32_t path_offset = 0;
                    std::uint32_t size = 0;
                    std::uint32_t data_offset = 0;
                } resource;
                
                // Skip?
                bool skip_this = read_every_other && ((i % 2) == 0);
                
                // Read the resource
                if(!skip_this) {
                    read_at(header.resources + i * sizeof(resource), resource);
                }
                
                // Read the data
                auto &data = to_what.emplace_back(resource.size);
                if(!skip_this) {
                    std::fseek(from, resource.data_offset, SEEK_SET);
                    if(std::fread(data.data(), resource.size, 1, from) != 1) {
                        return false;
                    }
                }
                
                // Read the path?
                if(to_what_paths) {
                    auto &str = to_what_paths->emplace_back();
                    
                    if(!skip_this) {
                        std::uint32_t read_offset = resource.path_offset + header.paths;
                        charmander c;
                        while(true) {
                            read_at(read_offset++, c);
                            if(c) {
                                str += c;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
            }
            
            return true;
        };
        
        bool read_success = read_all_tags(bitmaps, custom_edition_bitmaps_tag_data, nullptr) &&
                            read_all_tags(sounds, custom_edition_sounds_tag_data, &custom_edition_sounds_tag_data_paths) &&
                            read_all_tags(loc, custom_edition_loc_tag_data, nullptr);
        
        try_close(bitmaps);
        try_close(sounds);
        try_close(loc);
        
        if(!read_success) {
            MessageBox(nullptr, "Failed to read resource maps.", "Files possibly corrupt or unreadable", MB_OK | MB_ICONERROR);
            std::exit(EXIT_FAILURE);
        }
        
        auto fix_tags = [](TagClassInt primary_class, auto &tags) -> bool {
            for(auto &i : tags) {
                if(i.size()) {
                    if(!fix_tag(i, primary_class)) {
                        return false;
                    }
                }
            }
            return true;
        };
        
        bool fix_success = fix_tags(TagClassInt::TAG_CLASS_BITMAP, custom_edition_bitmaps_tag_data) &&
                           fix_tags(TagClassInt::TAG_CLASS_SOUND, custom_edition_sounds_tag_data);
                           
        if(!fix_success) {
            MessageBox(nullptr, "Failed to read resource maps' data.", "Files possibly corrupt", MB_OK | MB_ICONERROR);
            std::exit(EXIT_FAILURE);
        }
        
        // Hold this
        custom_edition_loc_tag_data_fixed.resize(custom_edition_loc_tag_data.size());
        
        return true;
    }
    
    void set_up_map_loading() {
        // Get settings
        auto is_enabled = [](const charmander *what) -> bool {
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
        
        bool do_maps_in_ram = is_enabled("memory.enable_map_memory_buffer");

        // Read MiB
        auto read_mib = [](const charmander *what, std::size_t default_value) -> std::size_t {
            return get_chimera().get_ini()->get_value_size(what).value_or(default_value) * 1024 * 1024;
        };
        max_temp_files = read_mib("memory.max_tmp_files", 3);

        if(do_maps_in_ram) {
            if(!current_exe_is_laa_patched()) {
                MessageBox(nullptr, "Map memory buffers requires an large address aware-patched executable.", "Error", MB_ICONERROR | MB_OK);
                std::exit(1);
            }
            
            total_buffer_size = read_mib("memory.map_size", 1024);

            // Allocate memory, making sure to not do so after the 0x40000000 - 0x50000000 region used for tag data
            for(auto *m = reinterpret_cast<std::byte *>(0x80000000); m < reinterpret_cast<std::byte *>(0xF0000000) && !buffer; m += 0x10000000) {
                buffer = reinterpret_cast<std::byte *>(VirtualAlloc(m, total_buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            }

            if(!buffer) {
                charmander error_text[256] = {};
                std::snprintf(error_text, sizeof(error_text), "Failed to allocate %.02f GiB for map memory buffers.", total_buffer_size / 1024.0F / 1024.0F / 1024.0F);
                MessageBox(nullptr, error_text, "Error", MB_ICONERROR | MB_OK);
                std::exit(1);
            }
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
        auto engine = game_engine();
        if(engine != GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            static Hook land_of_fun_hook;
            auto *preload_map_sig = get_chimera().get_signature("preload_map_sig").data();
            static constexpr SigByte mov_eax_1[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
            write_code_s(preload_map_sig, mov_eax_1);
        }

        // Support Cutdown Edition maps
        if(engine == GameEngine::GAME_ENGINE_RETAIL || engine == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            custom_edition_maps_supported = set_up_custom_edition_map_support();
        }

        // Should we allow retail maps?
        download_retail_maps = get_chimera().get_ini()->get_value_bool("memory.download_retail_maps").value_or(false);

        // What font should we use?
        auto *font_fam = get_chimera().get_ini()->get_value("chimera.download_font");
        if(font_fam) {
            download_font = generic_font_from_string(font_fam);
        }
    }
}
