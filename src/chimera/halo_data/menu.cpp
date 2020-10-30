// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include "../halo_data/tag.hpp"
#include "../halo_data/tag_class.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "menu.hpp"

namespace Chimera {
    extern "C" void load_ui_widget_asm(const char *ui_widget_path) noexcept;
    extern "C" void load_main_menu_asm() noexcept;

    bool load_ui_widget(const char *tag_path) noexcept {
        if(get_tag(tag_path, TAG_CLASS_UI_WIDGET_DEFINITION)) {
            load_ui_widget_asm(tag_path);
            return true;
        }
        return false;
    }

    void load_main_menu() noexcept {
        load_main_menu_asm();
    }
}