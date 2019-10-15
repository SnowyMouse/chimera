#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "map.hpp"

namespace Chimera {
    MapHeader &get_map_header() noexcept {
        static auto *map_header = *reinterpret_cast<MapHeader **>(get_chimera().get_signature("map_header_sig").data() + 2);
        return *map_header;
    }

    std::byte *map_indices() noexcept {
        static auto **all_map_indices = *reinterpret_cast<std::byte ***>(get_chimera().get_signature("map_index_sig").data() + 10);
        return *all_map_indices;
    }

    std::uint32_t maps_count() noexcept {
        static auto *map_count = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("map_index_sig").data() + 2);
        return *map_count;
    }

    extern "C" void load_ui_map_asm() noexcept;
    void load_ui_map() noexcept {
        load_ui_map_asm();
    }
}
