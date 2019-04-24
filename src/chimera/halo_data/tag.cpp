#include "../output/output.hpp"
#include "tag.hpp"

namespace Chimera {
    Tag *get_tag(const char *path, TagClassInt tag_class) noexcept {
        auto &tag_data_header = get_tag_data_header();
        auto *tag = tag_data_header.tag_array;
        auto tag_count = tag_data_header.tag_count;
        for(std::size_t i = 0; i < tag_count; i++) {
            if(tag[i].primary_class == tag_class && std::strcmp(path, tag[i].path) == 0) {
                return tag + i;
            }
        }
        return nullptr;
    }

    Tag *get_tag(TagID tag_id) noexcept {
        if(tag_id.is_null()) {
            return nullptr;
        }

        auto &tag_data_header = get_tag_data_header();
        auto *tag = tag_data_header.tag_array + tag_id.index.index;
        auto tag_count = tag_data_header.tag_count;

        if(tag_id.index.index <= tag_count) {
            return tag;
        }

        return nullptr;
    }

    Tag *get_tag(std::size_t tag_index) noexcept {
        if(tag_index == 0xFFFFFFFF) {
            return nullptr;
        }

        auto &tag_data_header = get_tag_data_header();
        auto *tag = tag_data_header.tag_array + tag_index;
        auto tag_count = tag_data_header.tag_count;

        if(tag_index <= tag_count) {
            return tag;
        }

        return nullptr;
    }
}
