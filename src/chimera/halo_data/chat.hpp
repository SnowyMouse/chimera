// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HALO_DATA_CHAT_HPP
#define CHIMERA_HALO_DATA_CHAT_HPP

namespace Chimera {
    /**
     * Send a chat message
     * @param channel channel to send on
     * @param message message to send
     */
    void chat_out(int channel, const wchar_t *message);

    /**
     * Send a chat message
     * @param channel channel to send on
     * @param message message to send
     */
    void chat_out(int channel, const char *message);
}

#endif
