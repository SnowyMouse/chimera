// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../output/output.hpp"

#include "multiplayer.hpp"

namespace Chimera {
    ServerType server_type() {
        static auto *server_type = *reinterpret_cast<ServerType **>(get_chimera().get_signature("server_type_sig").data() + 3);
        return *server_type;
    }

    Gametype gametype() {
        static auto *gametype = *reinterpret_cast<Gametype **>(get_chimera().get_signature("current_gametype_sig").data() + 2);
        return *gametype;
    }

    bool is_team() {
        static auto *is_team = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("current_gametype_sig").data() + 2) + 4;
        return *is_team;
    }
}
