// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CONSOLE_HPP
#define CHIMERA_CONSOLE_HPP

namespace Chimera {
    /**
     * Enable the console hook, enabling console interactivity.
     */
    void initialize_console_hook();

    /**
     * Set whether Halo should automatically suspend if the console is open in singleplayer.
     * @param suspend if true, suspend as normal
     */
    void set_auto_suspend_on_console(bool suspend) noexcept;

    /**
     * Set whether the console is enabled
     * @param enabled true to enable
     */
    void set_console_enabled(bool enabled) noexcept;

    /**
     * Get whether the console is open
     * @return true if open
     */
    bool get_console_open() noexcept;

    /**
     * Get whether the console is enabled
     * @return true if enabled
     */
    bool get_console_enabled() noexcept;

    /**
     * Return true if the rcon command was used recently
     * @return true if rcon was used recently
     */
    bool rcon_used_recently() noexcept;

    /**
     * Set up and enable the console fade fix
     */
    void setup_console_fade_fix() noexcept;

    /**
     * Set up the custom consolizer
     */
    void setup_custom_console() noexcept;
}

#endif
