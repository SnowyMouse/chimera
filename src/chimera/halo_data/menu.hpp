// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MENU_HPP
#define CHIMERA_MENU_HPP

namespace Chimera {
    /**
     * Load a UI widget
     */
    bool load_ui_widget(const char *tag_path) noexcept;

    /**
     * Load the UI main menu
     */
    void load_main_menu() noexcept;
}

#endif