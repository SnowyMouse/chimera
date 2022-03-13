// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MASTER_SERVER_HPP
#define CHIMERA_MASTER_SERVER_HPP

#include <cstdint>

namespace Chimera {
    /**
     * Set the number of threads Halo uses to connect to the master server
     * @param threads number of threads (0 - 127)
     */
    void set_master_server_connection_threads(std::int8_t threads) noexcept;

    /**
     * Fix the master server stuff
     */
    void set_up_master_server() noexcept;

    /**
     * Set master server
     * @param list    list server (if nullptr, use default)
     * @param key     key server (if nullptr, use default)
     * @param natneg1 natneg1 server (if nullptr, use default)
     * @param natneg2 natneg2 server (if nullptr, use default)
     */
    void set_master_server(const char *list, const char *key, const char *natneg1, const char *natneg2);

    /**
     * Get master server details
     * @param list    list server
     * @param key     key server
     * @param natneg1 natneg1 server
     * @param natneg2 natneg2 server
     */
    void get_master_server(const char **list, const char **key, const char **natneg1, const char **natneg2);
}

#endif
