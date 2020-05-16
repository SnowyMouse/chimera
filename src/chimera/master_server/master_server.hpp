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
    void set_up_demo_master_server() noexcept;
}

#endif
