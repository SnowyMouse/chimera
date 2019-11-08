#include <filesystem>
#include "map_loading.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"

namespace Chimera {
    extern "C" void do_map_loading_handling(char *map_path, const char *map_name) {
        const char *new_path = path_for_map(map_name);
        if(new_path) {
            std::strcpy(map_path, new_path);
        }
    }

    extern "C" void map_loading_asm();

    void set_up_map_loading(bool maps_in_ram) {
        static Hook hook;
        auto &map_load_path_sig = get_chimera().get_signature("map_load_path_sig");
        write_jmp_call(map_load_path_sig.data(), hook, nullptr, reinterpret_cast<const void *>(map_loading_asm));
    }

    const char *path_for_map(const char *map) noexcept {
        static char path[MAX_PATH];
        #define RETURN_IF_FOUND(...) std::snprintf(path, sizeof(path), __VA_ARGS__, map); if(std::filesystem::exists(path)) return path;
        RETURN_IF_FOUND("maps\\%s.map");
        RETURN_IF_FOUND("%s\\maps\\%s.map", get_chimera().get_path());
        return nullptr;
    }
}
