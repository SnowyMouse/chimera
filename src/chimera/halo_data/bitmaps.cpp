// SPDX-License-Identifier: GPL-3.0-only

#include "bitmaps.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "tag.hpp"
#include "tag_class.hpp"

namespace Chimera {

    Bitmap *get_bitmap_tag(TagID tag_id) noexcept {
        auto *tag = get_tag(tag_id);
        if(!tag || tag->primary_class != TAG_CLASS_BITMAP) {
            return nullptr;
        }
        return reinterpret_cast<Bitmap *>(tag->data);
    }

    BitmapData *get_bitmap_data_element(Bitmap *bitmap, std::uint32_t index) noexcept {
        return reinterpret_cast<BitmapData *>(get_tag_block_data(&bitmap->bitmap_data, index, sizeof(BitmapData)));
    }

}
