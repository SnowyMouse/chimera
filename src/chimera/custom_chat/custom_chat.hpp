// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CUSTOM_CHAT_HPP
#define CHIMERA_CUSTOM_CHAT_HPP
namespace Chimera {
    /**
     * Initialize the custom chat
     */
    void initialize_custom_chat() noexcept;

    /**
     * Check if the custom chat was enabled
     * @return true if enabled, false if not
     */
    bool custom_chat_enabled() noexcept;

    /**
     * Add a chat message to the server messages
     * @param message message to add
     */
    void add_server_message(const char *message);

    /**
     * Set whether to show color help
     * @param show_help true to show color help
     */
    void set_show_color_help(bool show_help) noexcept;

    /**
     * Get whether or not to show color help
     * @return true if showing color help
     */
    bool show_color_help() noexcept;

    /**
     * Get whether or not to block ips
     * @return true if blocking ips
     */
    bool ips_blocked() noexcept;

    /**
     * Set whether or not to block ips
     * @param block should block ips
     */
    void set_block_ips(bool block) noexcept;
}
#endif
