#include <filesystem>
#include <sys/stat.h>
#include "map_loading.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"

namespace Invader::Compression {
    void decompress_map_file(const char *input, const char *output);
}

namespace Chimera {
    struct CompressedMapIndex {
        char map_name[32];
        std::uint64_t date_modified;
    };

    static std::size_t last_loaded_map = 0;

    extern "C" void do_free_map_handle_bugfix(HANDLE &handle) {
        if(handle) {
            CloseHandle(handle);
            handle = 0;
        }
    }

    extern "C" void do_map_loading_handling(char *map_path, const char *map_name) {
        const char *new_path = path_for_map(map_name);
        if(new_path) {
            // Hold our compressed maps
            static CompressedMapIndex compressed_maps[2] = {};

            // Get filesystem data
            struct stat64 s;
            stat64(new_path, &s);
            std::uint64_t mtime = s.st_mtime;

            char tmp_path[MAX_PATH] = {};
            auto set_tmp_path = [&tmp_path](std::size_t index) {
                std::snprintf(tmp_path, MAX_PATH, "%s\\tmp_%zu.map", get_chimera().get_path(), index);
            };

            // See if we can find it
            for(auto &map : compressed_maps) {
                if(std::strcmp(map_name, map.map_name) == 0 && map.date_modified == mtime) {
                    set_tmp_path(&map - compressed_maps);
                    //console_output("Didn't need to decompress %s -> %s", new_path, tmp_path);
                    last_loaded_map = &map - compressed_maps;
                    std::strncpy(map_path, tmp_path, MAX_PATH);
                    return;
                }
            }

            // Attempt to decompress
            std::size_t new_index = !last_loaded_map;
            auto &compressed_map_to_use = compressed_maps[new_index];
            try {
                set_tmp_path(new_index);
                //console_output("Trying to compress %s @ %s -> %s...", map_name, new_path, tmp_path);
                Invader::Compression::decompress_map_file(new_path, tmp_path);
                std::strcpy(map_path, tmp_path);
                //console_output("Decompressed %s -> %s\n", new_path, map_path);
                compressed_map_to_use.date_modified = mtime;
                std::strncpy(compressed_map_to_use.map_name, map_name, sizeof(compressed_map_to_use.map_name));
                last_loaded_map = !last_loaded_map;
            }
            catch (std::exception &e) {
                compressed_map_to_use = {};
                std::strcpy(map_path, new_path);
                //console_output("Failed to decompress %s @ %s: %s", map_name, new_path, e.what());
            }
        }
    }

    extern "C" void map_loading_asm();
    extern "C" void free_map_handle_bugfix_asm();

    void set_up_map_loading(bool maps_in_ram) {
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
    }

    const char *path_for_map(const char *map) noexcept {
        static char path[MAX_PATH];
        #define RETURN_IF_FOUND(...) std::snprintf(path, sizeof(path), __VA_ARGS__, map); if(std::filesystem::exists(path)) return path;
        RETURN_IF_FOUND("maps\\%s.map");
        RETURN_IF_FOUND("%s\\maps\\%s.map", get_chimera().get_path());
        return nullptr;
    }
}
