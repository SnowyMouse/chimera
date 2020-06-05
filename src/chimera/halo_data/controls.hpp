// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CONTROLS_HPP
#define CHIMERA_CONTROLS_HPP

#include <cstdint>
#include "pad.hpp"

namespace Chimera {
    struct Controls {
        std::uint8_t jump;
        std::uint8_t switch_grenade;
        std::uint8_t action;
        std::uint8_t switch_weapon;

        std::uint8_t melee;
        std::uint8_t flashlight;
        std::uint8_t secondary_fire;
        std::uint8_t primary_fire;

        std::uint8_t menu_forward;
        std::uint8_t menu_back;
        std::uint8_t crouch;
        std::uint8_t zoom;

        std::uint8_t scores;
        std::uint8_t reload;
        std::uint8_t exchange_weapons;
        std::uint8_t all_chat;

        std::uint8_t team_chat;
        std::uint8_t vehicle_chat;
        PAD(0x1);
        PAD(0x1);

        PAD(0x4);

        PAD(0x1);
        PAD(0x1);
        PAD(0x1);
        std::uint8_t rules;

        std::uint8_t show_player_names;
        PAD(0x3);

        float move_forward;
        float move_left;
        float aim_left;
        float aim_up;

        std::uint8_t controller_aim;
        PAD(0x3);
    };

    Controls &get_controls() noexcept;
}

#endif
