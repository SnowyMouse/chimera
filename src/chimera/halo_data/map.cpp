// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "map.hpp"
#include "tag.hpp"
#include "tag_class.hpp"
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

    // hack from Invader
    bool map_is_protected() noexcept {
        if(get_tag(0)->primary_class != TagClassInt::TAG_CLASS_SCENARIO) {
            return true;
        }

        auto &tag_data_header = get_tag_data_header();

        auto tag_count = tag_data_header.tag_count;

        for(std::size_t t = 0; t < tag_count; t++) {
            auto *tag = get_tag(t);

            auto *tag_path = tag->path;
            auto tag_class = tag->primary_class;

            if(std::strcmp(tag_path, "") == 0) {
                return true;
            }

            if(tag_class == TagClassInt::TAG_CLASS_NONE || tag_class == TagClassInt::TAG_CLASS_NULL) {
                return true;
            }

            for(std::size_t t2 = t + 1; t2 < tag_count; t2++) {
                auto *tag2 = get_tag(t2);
                if(tag_class != tag2->primary_class) {
                    continue;
                }
                if(std::strcmp(tag_path, tag2->path) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    extern "C" void load_ui_map_asm() noexcept;
    void load_ui_map() noexcept {
        load_ui_map_asm();
    }
}
