// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <shlwapi.h>

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/frame.hpp"
#include "../event/tick.hpp"
#include "../event/map_load.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/game_engine.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../map_loading/map_loading.hpp"
#include "../output/output.hpp"

#include "fast_load.hpp"

extern "C" {
    std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;
    void on_get_crc32_hook() noexcept;
}

namespace Chimera {
    static bool same_string_case_insensitive(const char *a, const char *b) {
        if(a == b) return true;
        while(std::tolower(*a) == std::tolower(*b)) {
            if(*a == 0) return true;
            a++;
            b++;
        }
        return false;
    }

    std::optional<std::uint32_t> crc32_for_stock_map(const char *stock_map) noexcept {
        if(std::strcmp(stock_map, "beavercreek") == 0) {
            return 0x07B3876A;
        }
        else if(std::strcmp(stock_map, "bloodgulch") == 0) {
            return 0x7B309554;
        }
        else if(std::strcmp(stock_map, "boardingaction") == 0) {
            return 0xF4DEEF94;
        }
        else if(std::strcmp(stock_map, "carousel") == 0) {
            return 0x9C301A08;
        }
        else if(std::strcmp(stock_map, "chillout") == 0) {
            return 0x93C53C27;
        }
        else if(std::strcmp(stock_map, "damnation") == 0) {
            return 0x0FBA059D;
        }
        else if(std::strcmp(stock_map, "dangercanyon") == 0) {
            return 0xC410CD74;
        }
        else if(std::strcmp(stock_map, "deathisland") == 0) {
            return 0x1DF8C97F;
        }
        else if(std::strcmp(stock_map, "gephyrophobia") == 0) {
            return 0xD2872165;
        }
        else if(std::strcmp(stock_map, "hangemhigh") == 0) {
            return 0xA7C8B9C6;
        }
        else if(std::strcmp(stock_map, "icefields") == 0) {
            return 0x5EC1DEB7;
        }
        else if(std::strcmp(stock_map, "infinity") == 0) {
            return 0x0E7F7FE7;
        }
        else if(std::strcmp(stock_map, "longest") == 0) {
            return 0xC8F48FF6;
        }
        else if(std::strcmp(stock_map, "prisoner") == 0) {
            return 0x43B81A8B;
        }
        else if(std::strcmp(stock_map, "putput") == 0) {
            return 0xAF2F0B84;
        }
        else if(std::strcmp(stock_map, "ratrace") == 0) {
            return 0xF7F8E14C;
        }
        else if(std::strcmp(stock_map, "sidewinder") == 0) {
            return 0xBD95CF55;
        }
        else if(std::strcmp(stock_map, "timberland") == 0) {
            return 0x54446470;
        }
        else if(std::strcmp(stock_map, "wizard") == 0) {
            return 0xCF3359B1;
        }
        return std::nullopt;
    }

    extern std::byte *maps_in_ram_region;

    template <typename MapHeader> std::uint32_t calculate_crc32_of_map_file(std::FILE *f, const MapHeader &header) noexcept {
        std::uint32_t crc = 0;
        std::uint32_t current_offset = 0;

        // Use a built-in CRC32 if possible (CRC32s from Invader)
        if(header.engine_type == CACHE_FILE_RETAIL || header.engine_type == CACHE_FILE_RETAIL_COMPRESSED) {
            auto crc = crc32_for_stock_map(header.name);
            if(crc.has_value()) {
                return *crc;
            }
        }

        auto seek = [&f, &current_offset](std::size_t offset) {
            if(f) {
                std::fseek(f, offset, SEEK_SET);
            }
            else {
                current_offset = offset;
            }
        };

        auto read = [&f, &current_offset](void *where, std::size_t size) {
            if(f) {
                std::fread(where, size, 1, f);
            }
            else {
                std::copy(maps_in_ram_region + current_offset, maps_in_ram_region + size + current_offset, reinterpret_cast<std::byte *>(where));
            }
            current_offset += size;
        };

        // Load tag data
        auto *tag_data = new char[header.tag_data_size];
        seek(header.tag_data_offset);
        read(tag_data, header.tag_data_size);

        // Get the scenario tag so we can get the BSPs
        std::uint32_t tag_data_addr = reinterpret_cast<std::uint32_t>(get_tag_data_address());
        auto *scenario_tag = tag_data + (*reinterpret_cast<std::uint32_t *>(tag_data) - tag_data_addr) + (*reinterpret_cast<std::uint32_t *>(tag_data + 4) & 0xFFFF) * 0x20;
        auto *scenario_tag_data = tag_data + (*reinterpret_cast<std::uint32_t *>(scenario_tag + 0x14) - tag_data_addr);

        // CRC32 the BSP(s)
        auto &structure_bsp_count = *reinterpret_cast<std::uint32_t *>(scenario_tag_data + 0x5A4);
        auto *structure_bsps = tag_data + (*reinterpret_cast<std::uint32_t *>(scenario_tag_data + 0x5A4 + 4) - tag_data_addr);
        for(std::size_t b=0;b<structure_bsp_count;b++) {
            char *bsp = structure_bsps + b * 0x20;
            auto &bsp_offset = *reinterpret_cast<std::uint32_t *>(bsp);
            auto &bsp_size = *reinterpret_cast<std::uint32_t *>(bsp + 4);

            char *bsp_data = new char[bsp_size];
            seek(bsp_offset);
            read(bsp_data, bsp_size);
            crc = crc32(crc, bsp_data, bsp_size);
            delete[] bsp_data;
        }

        // Next, CRC32 the model data
        auto &model_vertices_offset = *reinterpret_cast<std::uint32_t *>(tag_data + 0x14);
        auto &vertices_size = *reinterpret_cast<std::uint32_t *>(tag_data + 0x20);

        auto *model_vertices = new char[vertices_size];
        seek(model_vertices_offset);
        read(model_vertices, vertices_size);
        crc = crc32(crc, model_vertices, vertices_size);
        delete[] model_vertices;

        // Lastly, CRC32 the tag data itself
        crc = crc32(crc, tag_data, header.tag_data_size);
        delete[] tag_data;

        return crc;
    }

    extern std::uint32_t maps_in_ram_crc32;
    std::uint32_t current_loaded_crc32 = 0xFFFFFFFF;

    // Function for getting CRC32 without a Custom Edition map index
    template <typename MapHeader> static void on_get_crc32_non_custom() noexcept {
        const MapHeader *header;
        if(get_chimera().feature_present("client_demo")) {
            header = &reinterpret_cast<MapHeader &>(get_demo_map_header());
        }
        else {
            header = &reinterpret_cast<MapHeader &>(get_map_header());
        }

        auto *path = path_for_map(header->name, true);
        if(path) {
            // Load the header
            std::FILE *f = nullptr;

            if(!maps_in_ram_region) {
                f = std::fopen(path, "rb");
                if(!f) {
                    return;
                }
                current_loaded_crc32 = ~calculate_crc32_of_map_file(f, *header);

                // Close if open
                if(f) {
                    std::fclose(f);
                    f = nullptr;
                }
            }
            else {
                current_loaded_crc32 = maps_in_ram_crc32;
            }
        }
    }

    extern "C" void on_get_crc32() noexcept {
        // Get the loading map and all map indices so we can find which map is loading
        static char *loading_map = *reinterpret_cast<char **>(get_chimera().get_signature("loading_map_sig").data() + 1);
        auto &map_list = get_map_list();
        auto *indices = reinterpret_cast<MapIndexCustomEdition *>(map_list.map_list);

        // Iterate through each map
        for(std::size_t i=0;i<map_list.map_count;i++) {
            if(same_string_case_insensitive(indices[i].file_name, loading_map)) {
                auto *path = path_for_map(indices[i].file_name, true);
                bool map_already_crc = indices[i].crc32 != 0xFFFFFFFF;

                // Do what we need to do
                if(map_already_crc || !path) {
                    goto set_current_loaded_crc32;
                }
                else {
                    // Load the header
                    std::FILE *f = nullptr;

                    MapHeader header;
                    if(!maps_in_ram_region) {
                        f = std::fopen(path, "rb");
                        if(!f) {
                            goto set_current_loaded_crc32;
                        }
                        std::fread(&header, sizeof(header), 1, f);
                        indices[i].crc32 = ~calculate_crc32_of_map_file(f, header);

                        // Close if open
                        if(f) {
                            std::fclose(f);
                            f = nullptr;
                        }
                    }
                    else {
                        indices[i].crc32 = maps_in_ram_crc32;
                    }
                }

                set_current_loaded_crc32:
                current_loaded_crc32 = indices[i].crc32;

                return;
            }
        }
    }

    static void on_get_crc32_deferred() {
        if(server_type() == ServerType::SERVER_NONE) {
            on_get_crc32_non_custom<MapHeader>();
        }
        else {
            on_get_crc32();
        }
    }

    void initialize_fast_load() noexcept {
        auto engine = game_engine();

        switch(engine) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION: {
                // Disable Halo's CRC32ing (drastically speed up loading)
                auto *get_crc = get_chimera().get_signature("get_crc_sig").data();
                static unsigned char nop7[7] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
                overwrite(get_crc, nop7, sizeof(nop7));
                overwrite(get_crc, static_cast<std::uint8_t>(0xE8));
                overwrite(get_crc + 1, reinterpret_cast<std::uintptr_t>(on_get_crc32_hook) - reinterpret_cast<std::uintptr_t>(get_crc + 5));
                add_map_load_event(on_get_crc32_deferred);

                // Prevent Halo from loading the map list (speed up loading)
                overwrite(get_chimera().get_signature("load_multiplayer_maps_sig").data(), static_cast<std::uint8_t>(0xC3));

                // Load the maps list on the next tick
                add_frame_event(reload_map_list_frame);

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }

            case GameEngine::GAME_ENGINE_RETAIL: {
                // Meme Halo into showing custom maps
                overwrite(get_chimera().get_signature("load_multiplayer_maps_retail_sig").data(), static_cast<std::uint8_t>(0xC3));

                // Get the thing
                add_map_load_event(on_get_crc32_non_custom<MapHeader>);

                // Load the maps list on the next tick
                add_frame_event(reload_map_list_frame);

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }

            case GameEngine::GAME_ENGINE_DEMO: {
                // Meme Halo into showing custom maps
                overwrite(get_chimera().get_signature("load_multiplayer_maps_demo_sig").data(), static_cast<std::uint8_t>(0xC3));

                // Get the thing
                add_map_load_event(on_get_crc32_non_custom<MapHeaderDemo>);

                // Load the maps list on the next tick
                add_frame_event(reload_map_list_frame);

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_demo_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }
        }
    }
    
    static std::vector<MapEntry> multiplayer_maps;
    
    template <typename MapIndexType> static void resync_map_list() {
        // Hold our indices
        static MapIndexType **indices = nullptr;
        static std::uint32_t *count = nullptr;
        static std::vector<MapIndexType> indices_vector;
        
        auto &map_list = get_map_list();
        indices = reinterpret_cast<MapIndexType **>(&map_list.map_list);
        count = reinterpret_cast<std::uint32_t *>(&map_list.map_count);
        indices_vector.clear();
        
        for(auto &i : multiplayer_maps) {
            auto *map = &indices_vector.emplace_back();
            map->file_name = i.name.c_str();
            map->map_name_index = i.index.value_or(13);
            
            if(sizeof(*map) >= sizeof(MapIndexRetail)) {
                reinterpret_cast<MapIndexRetail *>(map)->loaded = 1;
                
                if(sizeof(*map) >= sizeof(MapIndexCustomEdition)) {
                    reinterpret_cast<MapIndexCustomEdition *>(map)->crc32 = i.crc32.value_or(0xFFFFFFFF); // we're overriding this
                }
            }
        }
        
        *indices = indices_vector.data();
        *count = multiplayer_maps.size();
    }
    
    void resync_map_list() {
        auto engine = game_engine();

        switch(engine) {
            case GameEngine::GAME_ENGINE_CUSTOM_EDITION:
                resync_map_list<MapIndexCustomEdition>();
                break;
            case GameEngine::GAME_ENGINE_RETAIL:
                resync_map_list<MapIndexRetail>();
                break;
            case GameEngine::GAME_ENGINE_DEMO:
                resync_map_list<MapIndex>();
                break;
        }
    }
    
    void add_map_to_map_list(const char *map_name, std::optional<std::uint32_t> map_index) {
        std::string name_lowercase = map_name;
        for(auto &c : name_lowercase) {
            c = std::tolower(c);
        }
    
        // Don't add maps we've already added
        for(auto &m : multiplayer_maps) {
            if(m.name == map_name) {
                return;
            }
        }
        
        // Add it!
        auto &map = multiplayer_maps.emplace_back();
        map.name = name_lowercase;
        map.index = map_index;
    }

    static void reload_map_list() {
        // Clear the bitch
        multiplayer_maps.clear();
        
        std::uint32_t stock_index = 0;
        #define ADD_STOCK_MAP(map_name) add_map_to_map_list(map_name, stock_index++)
        
        if(game_engine() == GameEngine::GAME_ENGINE_DEMO) {
            ADD_STOCK_MAP("bloodgulch");
        }
        else {
            ADD_STOCK_MAP("beavercreek");
            ADD_STOCK_MAP("sidewinder");
            ADD_STOCK_MAP("damnation");
            ADD_STOCK_MAP("ratrace");
            ADD_STOCK_MAP("prisoner");
            ADD_STOCK_MAP("hangemhigh");
            ADD_STOCK_MAP("chillout");
            ADD_STOCK_MAP("carousel");
            ADD_STOCK_MAP("boardingaction");
            ADD_STOCK_MAP("bloodgulch");
            ADD_STOCK_MAP("wizard");
            ADD_STOCK_MAP("putput");
            ADD_STOCK_MAP("longest");
            ADD_STOCK_MAP("icefields");
            ADD_STOCK_MAP("deathisland");
            ADD_STOCK_MAP("dangercanyon");
            ADD_STOCK_MAP("infinity");
            ADD_STOCK_MAP("timberland");
            ADD_STOCK_MAP("gephyrophobia");
        }
        
        auto add_map_folder = [](std::filesystem::path directory) {
            static const char *BLACKLISTED_MAPS[] = {
                "a10",
                "a30",
                "a50",
                "b30",
                "b40",
                "c10",
                "c20",
                "c40",
                "d20",
                "d40",
                "bitmaps",
                "sounds",
                "loc",
                "ui",
                BITMAPS_CUSTOM_MAP_NAME,
                SOUNDS_CUSTOM_MAP_NAME,
                LOC_CUSTOM_MAP_NAME
            };
            
            for(auto &map : std::filesystem::directory_iterator(directory)) {
                if(map.is_regular_file()) {
                    auto &path = map.path();
                    
                    // Get extension
                    auto extension = path.extension().string();
                    for(auto &c : extension) {
                        c = std::tolower(c);
                    }
                    
                    if(extension == ".map") {
                        // Get name
                        auto name = path.stem().string();
                        auto name_lowercase = name;
                        for(auto &c : name_lowercase) {
                            c = std::tolower(c);
                        }
                        
                        // Is it blacklisted?
                        for(auto &b : BLACKLISTED_MAPS) {
                            if(name == b) {
                                goto nope;
                            }
                        }
                        
                        add_map_to_map_list(name.c_str());
                    }
                    
                    nope: continue;
                }
            }
        };
        
        add_map_folder("maps");
        add_map_folder(std::filesystem::path(get_chimera().get_path()) / "maps");
        
        resync_map_list();
    }

    void reload_map_list_frame() noexcept {
        remove_frame_event(reload_map_list);
        reload_map_list();
    }
}
