// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MULTIPLAYER_HPP
#define CHIMERA_MULTIPLAYER_HPP

#include <cstdint>

namespace Chimera {
    enum ServerType : std::uint16_t {
        SERVER_NONE = 0,
        SERVER_DEDICATED = 1,
        SERVER_LOCAL = 2
    };

    /**
     * Return the server type.
     * @return Return the server type.
     */
    ServerType server_type();

    enum Gametype : std::uint16_t {
        GAMETYPE_CTF = 1,
        GAMETYPE_SLAYER = 2,
        GAMETYPE_ODDBALL = 3,
        GAMETYPE_KING = 4,
        GAMETYPE_RACE = 5
    };

    /**
     * Return the gametype.
     * @return Return the gametype.
     */
    Gametype gametype();

    /**
     * Get if team game
     * @return true if team
     */
    bool is_team();
}

#endif
