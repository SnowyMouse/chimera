// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "map.hpp"
#include "game_engine.hpp"
#include <optional>

namespace Chimera {
    MapHeader &get_map_header() noexcept {
        static auto *map_header = *reinterpret_cast<MapHeader **>(get_chimera().get_signature("map_header_sig").data() + 2);
        return *map_header;
    }

    MapHeaderDemo &get_demo_map_header() noexcept {
        auto &map_header_sig = get_chimera().get_signature("map_header_sig");
        static auto *map_header = reinterpret_cast<MapHeaderDemo *>(*reinterpret_cast<std::byte **>(map_header_sig.data() + 2) - 0x2C0);
        return *map_header;
    }

    MapList &get_map_list() noexcept {
        static std::optional<MapList *> all_map_indices;
        if(!all_map_indices.has_value()) {
            if(game_engine() == GAME_ENGINE_DEMO) {
                all_map_indices = *reinterpret_cast<MapList **>(get_chimera().get_signature("map_index_demo_sig").data() + 2);
            }
            else {
                all_map_indices = *reinterpret_cast<MapList **>(get_chimera().get_signature("map_index_sig").data() + 10);
            }
        }
        return **all_map_indices;
    }

    extern "C" void load_ui_map_asm() noexcept;
    void load_ui_map() noexcept {
        load_ui_map_asm();
    }
}
