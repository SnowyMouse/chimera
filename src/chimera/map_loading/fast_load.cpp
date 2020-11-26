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

    extern "C" std::uint32_t on_get_crc32_custom_edition_loading() noexcept {
        return get_map_entry(get_map_name())->crc32.value();
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
            map->map_name_index = i.index.value_or(13);
            
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
    
    MapEntry &add_map_to_map_list(const char *map_name, std::optional<std::uint32_t> map_index) {
        // Don't add maps we've already added
        for(auto &m : all_maps) {
            if(same_string_case_insensitive(map_name, m.name.c_str())) {
                return m;
            }
        }
        
        // Add it!
        auto &map = all_maps.emplace_back();
        map.name = map_name;
        map.index = map_index;
        map.multiplayer = true;
        
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
        
        return map;
    }

    static void reload_map_list() {
        // Clear the bitch
        all_maps.clear();
        
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
                "bitmaps",
                "sounds",
                "loc",
                BITMAPS_CUSTOM_MAP_NAME,
                SOUNDS_CUSTOM_MAP_NAME,
                LOC_CUSTOM_MAP_NAME
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
