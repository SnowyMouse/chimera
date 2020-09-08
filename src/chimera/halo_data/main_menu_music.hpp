// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAIN_MENU_MUSIC_HPP
#define CHIMERA_MAIN_MENU_MUSIC_HPP

namespace Chimera {
    /**
     * Block the main menu music if we have to
     */
    void block_main_menu_music_if_needed() noexcept;

    /**
     * Set whether or not main menu music is forced
     * @param force force it or not
     */
    void set_force_block_main_menu_music(bool force) noexcept;
}

#endif
