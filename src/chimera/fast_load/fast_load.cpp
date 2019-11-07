#include <windows.h>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <shlwapi.h>

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/tick.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/game_engine.hpp"

#include "fast_load.hpp"

extern "C" {
    std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;
    void on_get_crc32_hook() noexcept;
}

namespace Chimera {
    template<typename MapIndexType> static void do_load_multiplayer_maps();

    static bool same_string_case_insensitive(const char *a, const char *b) {
        if(a == b) return true;
        while(std::tolower(*a) == std::tolower(*b)) {
            if(*a == 0) return true;
            a++;
            b++;
        }
        return false;
    }

    static std::uint32_t calculate_crc32_of_map_file(std::FILE *f, const MapHeader &header) noexcept {
        std::uint32_t crc = 0;
        std::uint32_t current_offset = 0;

        auto seek = [&f, &current_offset](std::size_t offset) {
            if(f) {
                std::fseek(f, offset, SEEK_SET);
            }
            else {
                current_offset = offset;
            }
        };

        auto read = [&f, &current_offset](void *where, std::size_t size) {
            std::fread(where, size, 1, f);
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

    extern "C" void on_get_crc32() noexcept {
        // Get the loading map and all map indices so we can find which map is loading
        static char *loading_map = *reinterpret_cast<char **>(get_chimera().get_signature("loading_map_sig").data() + 1);
        auto &map_list = get_map_list();
        auto *indices = reinterpret_cast<MapIndexCustomEdition *>(map_list.map_list);

        // Iterate through each map
        for(std::size_t i=0;i<map_list.map_count;i++) {
            if(same_string_case_insensitive(indices[i].file_name, loading_map)) {
                auto *path = path_for_map(indices[i].file_name);
                bool map_already_crc = indices[i].crc32 != 0xFFFFFFFF;

                // Do what we need to do
                if(map_already_crc || !path) {
                    return;
                }

                // Load the header
                std::FILE *f = std::fopen(path, "rb");
                if(!f) {
                    return;
                }
                MapHeader header;
                std::fread(&header, sizeof(header), 1, f);
                indices[i].crc32 = ~calculate_crc32_of_map_file(f, header);
                std::fclose(f);

                return;
            }
        }
    }

    void (*function_to_use)() = nullptr;

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
                add_tick_event(reload_map_list);
                function_to_use = do_load_multiplayer_maps<MapIndexCustomEdition>;

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }

            case GameEngine::GAME_ENGINE_RETAIL: {
                // Prevent Halo from loading the map list (speed up loading)
                overwrite(get_chimera().get_signature("load_multiplayer_maps_retail_sig").data(), static_cast<std::uint8_t>(0xC3));

                // Load the maps list on the next tick
                add_tick_event(reload_map_list);
                function_to_use = do_load_multiplayer_maps<MapIndexRetail>;

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }

            case GameEngine::GAME_ENGINE_DEMO: {
                // Load the maps list on the next tick
                add_tick_event(reload_map_list);
                function_to_use = do_load_multiplayer_maps<MapIndex>;

                // Stop Halo from freeing the map list on close since it will just segfault if it does that
                overwrite(get_chimera().get_signature("free_map_index_demo_sig").data(), static_cast<std::uint8_t>(0xC3));
                break;
            }
        }
    }

    template<typename MapIndexType> static void do_load_multiplayer_maps() {
        static std::vector<std::pair<std::unique_ptr<char []>, std::size_t>> names_vector;
        static MapIndexType **indices = nullptr;
        static std::uint32_t *count = nullptr;
        static std::vector<MapIndexType> indices_vector;

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
            "ui"
        };

        // If we've been here before, clear things. Otherwise, get addresses
        if(indices) {
            names_vector.clear();
            indices_vector.clear();
            *count = 0;
        }
        else {
            // Find locations
            auto &map_list = get_map_list();
            indices = reinterpret_cast<MapIndexType **>(&map_list.map_list);
            count = reinterpret_cast<std::uint32_t *>(&map_list.map_count);
            *count = 0;
        }

        auto add_map = [](const char *map_name, std::size_t string_length) {
            // Make sure we don't have the map already
            for(auto &name : names_vector) {
                if(string_length == name.second && std::memcmp(map_name, name.first.get(), string_length) == 0) {
                    return;
                }
            }

            // Make sure it's not blacklisted
            for(auto &name : BLACKLISTED_MAPS) {
                if(std::strcmp(map_name, name) == 0) {
                    return;
                }
            }

            // Allocate name
            auto map_name_copy = std::make_unique<char[]>(string_length + 1);
            std::memcpy(map_name_copy.get(), map_name, string_length + 1);

            // Add the string to the list
            names_vector.emplace_back(std::move(map_name_copy), string_length);

            // Increment the map count
            (*count)++;

            // Return true (we did it)
            return;
        };

        #define ADD_STOCK_MAP(map_name) add_map(map_name, std::strlen(map_name))

        // First, add the stock maps, only adding blood gulch if the demo
        if(sizeof(MapIndexType) == sizeof(MapIndex)) {
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

        auto add_map_by_path = [&add_map](const char *path) {
            // Next, add new maps
            WIN32_FIND_DATA find_file_data;
            auto handle = FindFirstFile(path, &find_file_data);
            BOOL ok = handle != INVALID_HANDLE_VALUE;
            while(ok) {
                // Cut off the extension
                std::size_t len = strlen(find_file_data.cFileName);
                find_file_data.cFileName[len - 4] = 0;
                len -= 4;

                // Make it all lowercase
                for(std::size_t i = 0; i < len; i++) {
                    find_file_data.cFileName[i] = std::tolower(find_file_data.cFileName[i]);
                }

                // Add it maybe
                add_map(find_file_data.cFileName, len);
                ok = FindNextFile(handle, &find_file_data);
            }
        };

        add_map_by_path("maps\\*.map");
        char dir[MAX_PATH];
        const char *chimera_path = get_chimera().get_path();
        if(std::snprintf(dir, sizeof(dir), "%s\\maps\\*.map", chimera_path) < sizeof(dir)) {
            add_map_by_path(dir);
        }

        // Lastly, allocate things
        indices_vector.clear();
        indices_vector.reserve(*count);
        for(std::size_t i = 0; i < *count; i++) {
            MapIndexType &index = indices_vector.emplace_back();
            if(sizeof(index) == sizeof(MapIndexCustomEdition)) {
                reinterpret_cast<MapIndexCustomEdition *>(&index)->crc32 = 0xFFFFFFFF;
            }
            index.file_name = names_vector[i].first.get();
            if(sizeof(index) >= sizeof(MapIndexRetail)) {
                reinterpret_cast<MapIndexRetail *>(&index)->loaded = 1;
            }

            // If it's demo, do this
            if(sizeof(index) == sizeof(MapIndex)) {
                index.map_name_index = std::strcmp(index.file_name, "bloodgulch") == 0 ? 0x9 : 0x13;
            }
            else {
                index.map_name_index = i < 0x13 ? i : 0x13;
            }
        }

        // Set pointers and such
        *indices = indices_vector.data();
    }

    const char *path_for_map(const char *map) noexcept {
        static char path[MAX_PATH];
        #define RETURN_IF_FOUND(...) std::snprintf(path, sizeof(path), __VA_ARGS__, map); if(PathFileExistsA(path)) return path;
        RETURN_IF_FOUND("maps\\%s.map");
        RETURN_IF_FOUND("%s\\maps\\%s.map", get_chimera().get_path());
        return nullptr;
    }

    void reload_map_list() noexcept {
        remove_tick_event(reload_map_list);
        function_to_use();
    }
}
