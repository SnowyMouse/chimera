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
#include "../output/error_box.hpp"

#include "fast_load.hpp"

extern "C" {
    std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;
    void on_get_crc32_hook() noexcept;
}

namespace Chimera {
    std::filesystem::path MapEntry::get_file_path() {
        auto p1 = std::filesystem::path("maps") / (this->name + ".map");
        if(std::filesystem::exists(p1)) {
            return p1;
        }
        else {
            return std::filesystem::path(get_chimera().get_path()) / "maps" / (this->name + ".map");
        }
    }
    
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

    extern "C" void on_get_crc32_custom_edition_loading() noexcept {
        static char *loading_map = *reinterpret_cast<char **>(get_chimera().get_signature("loading_map_sig").data() + 1);
        load_map(loading_map);
        auto *entry = get_map_entry(loading_map);
        auto &map_list = get_map_list();
        auto *indices = reinterpret_cast<MapIndexCustomEdition *>(map_list.map_list);
        for(std::size_t i=0; i<map_list.map_count; i++) {
            if(entry->name == indices[i].file_name) {
                indices[i].crc32 = entry->crc32.value();
                break;
            }
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

                // Load the maps list on the next tick
                add_frame_event(reload_map_list_frame);

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }

            case GameEngine::GAME_ENGINE_DEMO: {
                // Meme Halo into showing custom maps
                overwrite(get_chimera().get_signature("load_multiplayer_maps_demo_sig").data(), static_cast<std::uint8_t>(0xC3));

                // Load the maps list on the next tick
                add_frame_event(reload_map_list_frame);

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_demo_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }
        }
    }
    
    static std::vector<MapEntry> all_maps;
    
    template <typename MapIndexType> static void resync_map_list() {
        // Hold our indices
        static MapIndexType **indices = nullptr;
        static std::uint32_t *count = nullptr;
        static std::vector<MapIndexType> indices_vector;
        
        auto &map_list = get_map_list();
        indices = reinterpret_cast<MapIndexType **>(&map_list.map_list);
        count = reinterpret_cast<std::uint32_t *>(&map_list.map_count);
        indices_vector.clear();
        
        for(auto &i : all_maps) {
            if(!i.multiplayer) {
                continue;
            }
            
            auto *map = &indices_vector.emplace_back();
            map->file_name = i.name.c_str();
            map->map_name_index = i.index.value_or(19);
            
            if(sizeof(*map) >= sizeof(MapIndexRetail)) {
                reinterpret_cast<MapIndexRetail *>(map)->loaded = 1;
                
                if(sizeof(*map) >= sizeof(MapIndexCustomEdition)) {
                    reinterpret_cast<MapIndexCustomEdition *>(map)->crc32 = i.crc32.value_or(0xFFFFFFFF);
                }
            }
        }
        
        *indices = indices_vector.data();
        *count = indices_vector.size();
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
    
    MapEntry *get_map_entry(const char *map_name) {
        for(auto &map : all_maps) {
            if(same_string_case_insensitive(map_name, map.name.c_str())) {
                return &map;
            }
        }
        return nullptr;
    }
    
    static MapEntry *maybe_add_map_to_map_list(const char *map_name, std::optional<std::uint32_t> map_index = std::nullopt) {
        // Don't add maps we've already added
        MapEntry *map_possibly;
        if((map_possibly = get_map_entry(map_name)) != nullptr) {
            return map_possibly;
        }
        
        // First, let's lowercase it
        char map_name_lowercase[32];
        std::strncpy(map_name_lowercase, map_name, sizeof(map_name_lowercase) - 1);
        for(auto &i : map_name_lowercase) {
            i = std::tolower(i);
        }
        
        // Add it!
        MapEntry map;
        map.name = map_name_lowercase;
        map.index = map_index;
        map.multiplayer = true;
        
        // Make sure it exists first.
        if(!std::filesystem::exists(map.get_file_path())) {
            return nullptr;
        }
        
        // If it's known to not be a multiplayer map, set this
        static const char *NON_MULTIPLAYER_MAPS[] = {
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
            "ui"
        };
        for(auto &nmp : NON_MULTIPLAYER_MAPS) {
            if(same_string_case_insensitive(nmp, map_name)) {
                map.multiplayer = false;
            }
        }
        
        return &all_maps.emplace_back(std::move(map));
    }
    
    MapEntry &add_map_to_map_list(const char *map_name) {
        // Attempt to add it. If it fails, exit gracefully
        auto *ptr = maybe_add_map_to_map_list(map_name, std::nullopt);
        if(ptr == nullptr) {
            char error_message[256];
            std::snprintf(error_message, sizeof(error_message), "Failed to load %s.map into the maps list.\n\nMake sure the map exists and try again.", map_name);
            show_error_box("Map error", error_message);
            std::exit(EXIT_FAILURE);
        }
        return *ptr;
    }

    static void reload_map_list() {
        // Clear the bitch
        auto old_maps = all_maps;
        all_maps.clear();
        
        #define ADD_STOCK_MAP(map_name, index) maybe_add_map_to_map_list(map_name, index)
        
        ADD_STOCK_MAP("beavercreek", 0);
        ADD_STOCK_MAP("sidewinder", 1);
        ADD_STOCK_MAP("damnation", 2);
        ADD_STOCK_MAP("ratrace", 3);
        ADD_STOCK_MAP("prisoner", 4);
        ADD_STOCK_MAP("hangemhigh", 5);
        ADD_STOCK_MAP("chillout", 6);
        ADD_STOCK_MAP("carousel", 7);
        ADD_STOCK_MAP("boardingaction", 8);
        ADD_STOCK_MAP("bloodgulch", 9);
        ADD_STOCK_MAP("wizard", 10);
        ADD_STOCK_MAP("putput", 11);
        ADD_STOCK_MAP("longest", 12);
        ADD_STOCK_MAP("icefields", 13);
        ADD_STOCK_MAP("deathisland", 14);
        ADD_STOCK_MAP("dangercanyon", 15);
        ADD_STOCK_MAP("infinity", 16);
        ADD_STOCK_MAP("timberland", 17);
        ADD_STOCK_MAP("gephyrophobia", 18);
        
        auto add_map_folder = [](std::filesystem::path directory) {
            static const char *BLACKLISTED_MAPS[] = {
                "bitmaps",
                "sounds",
                "loc",
                "custom_bitmaps",
                "custom_sounds",
                "custom_loc"
            };
            
            for(auto &map : std::filesystem::directory_iterator(directory)) {
                if(map.is_regular_file()) {
                    auto &path = map.path();
                    
                    // Get extension
                    auto extension = path.extension().string();
                    if(same_string_case_insensitive(extension.c_str(), ".map")) {
                        // Get name
                        auto name = path.stem().string();
                        
                        // Is it blacklisted?
                        for(auto &b : BLACKLISTED_MAPS) {
                            if(same_string_case_insensitive(name.c_str(), b)) {
                                goto nope;
                            }
                        }
                        
                        maybe_add_map_to_map_list(name.c_str());
                    }
                    
                    nope: continue;
                }
            }
        };
        
        add_map_folder("maps");
        add_map_folder(std::filesystem::path(get_chimera().get_path()) / "maps");
        
        // Reset CRC32
        for(auto &i : old_maps) {
            auto *map = get_map_entry(i.name.c_str());
            if(map) {
                map->crc32 = i.crc32;
            }
        }
        
        resync_map_list();
    }

    void reload_map_list_frame() noexcept {
        remove_frame_event(reload_map_list_frame);
        reload_map_list();
    }
}
