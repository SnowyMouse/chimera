// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_PORT_HPP
#define CHIMERA_PORT_HPP

#include <cstdint>

namespace Chimera {
    /**
     * Set the ports if needed
     */
    void set_ports() noexcept;

    /**
     * Get the client port
     * @return client port
     */
    std::uint16_t get_client_port() noexcept;

    /**
     * Get the server port
     * @return server port
     */
    std::uint16_t get_server_port() noexcept;
}

#endif
