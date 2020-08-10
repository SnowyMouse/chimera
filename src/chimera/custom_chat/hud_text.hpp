// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HUD_TEXT_HPP
#define CHIMERA_HUD_TEXT_HPP
namespace Chimera {
    struct ColorARGB;

    /**
     * Initialize the sexiest HUD text you'll ever see
     */
    void initialize_hud_text() noexcept;

    /**
     * Get whether or not we initialized this stuff
     * @return true if we did it
     */
    bool hud_text_mod_initialized() noexcept;

    /**
     * Get the HUD color
     * @return HUD color
     */
    const ColorARGB &get_hud_text_color() noexcept;
}
#endif
