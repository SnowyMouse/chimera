// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_PLAYER_HPP
#define CHIMERA_PLAYER_HPP

#include <cstdint>
#include "pad.hpp"
#include "type.hpp"
#include "table.hpp"

namespace Chimera {
    /**
     * These are players.
     */
    struct Player {
        /** This is some sort of an index of the player. If it's 0xFFFF, the slot is not used. */
        std::uint16_t player_id;

        /** Unknown - I am guessing that this is the local client ID of the player (if valid - otherwise 0xFFFF). */
        std::uint16_t local_id;

        /** Name of the player */
        wchar_t name[12];

        PAD(0x4);

        /** Team of the player 0 = red; 1 = blue; etc. */
        std::uint8_t team;

        PAD(0x3);

        /** Object ID for whatever is being interacted with right now */
        ObjectID interaction_object_id;

        /** Type of object being interacted with right now */
        std::uint16_t interaction_object_type;

        /** If interacting with a vehicle, this is the seat */
        std::uint16_t interaction_object_seat;

        /** Ticks remaining for the player to respawn */
        TickCount respawn_time;

        /** Number of ticks being added to the player's next respawn time */
        TickCount respawn_time_growth;

        /** Player's current object ID. */
        ObjectID object_id;

        /** Player's object ID. Does this have to do with death cam or something? */
        ObjectID last_object_id;

        PAD(0x4);
        PAD(0x4);

        /** Last tick the player fired */
        TickCount last_fire_time;

        /** Name of the player (again) */
        wchar_t name2[12];

        /** Color of the player in respect to FFA */
        std::uint16_t color;

        PAD(0x2);

        /** Machine index of the player. This + 1 = rcon index */
        std::uint8_t machine_index;

        /** Unknown - Is this the controller input being used or something? */
        std::uint8_t controller_index;

        /** Team again? 0 = red; 1 = blue; etc. */
        std::uint8_t team2;

        /** Index of the player */
        std::uint8_t index;

        /** Number of ticks remaining for the player to be invisible. */
        std::uint16_t invisibility_time;

        PAD(0x2);

        /** Speed multiplier of the player. */
        float speed;

        /** Unknown */
        HaloID teleporter_flag_id;

        /** Unknown */
        std::uint32_t objective_mode;

        /** Unknown */
        PlayerID objective_player_id;

        /** Unknown - Player ID the player is looking at? */
        PlayerID target_player;

        /** Unknown - Some timer for fading in the name of the player being looked at? */
        std::uint32_t target_time;

        /** Tick the player last died */
        TickCount last_death_time;

        /** Current slayer target */
        PlayerID slayer_target;

        /** Player is odd man out */
        std::uint32_t odd_man_out;

        PAD(0x6);

        /** Player's current kill streak */
        std::uint16_t kill_streak;

        /** Unknown */
        std::uint16_t multikill;

        /** Last tick a player killed someone */
        std::uint16_t last_kill_time;

        /** Number of kills made by the player */
        std::uint16_t kills;

        PAD(0x6);

        /** Number of assists made by the player */
        std::uint16_t assists;

        PAD(0x6);

        /** Number of times player killed a member on the team (including themself) */
        std::uint16_t betrays;

        /** Number of times the player has died. If 0, instantly respawn player. */
        std::uint16_t deaths;

        /** Number of times the player killed themself */
        std::uint16_t suicides;

        PAD(0xE);

        /** Unknown - Number of times the player betrayed? */
        std::uint16_t team_kills;

        PAD(0x2);

        /** Unknown - Possibly number of flag steals, hill time (ticks), race time (ticks)? */
        std::uint16_t stat1;

        /** Unknown - Possibly number of flag returns or oddball target kills? */
        std::uint16_t stat2;

        /** Unknown - Possibly number of scores, oddball kills, or race best time (ticks)? */
        std::uint16_t stat3;

        PAD(0x2);

        /** Time being telefragged in ticks */
        TickCount telefrag_timer;

        /** Tick the player left */
        TickCount quit_time;

        PAD(0x8);

        /** Current ping of the player in milliseconds */
        std::uint32_t ping;

        /** Number of betrayals */
        std::uint32_t team_kill_count;

        /** Number of ticks since the player's last betrayal */
        TickCount team_kill_timer;

        PAD(0x10);

        /** Current position */
        Point3D position;

        PAD(0x18);

        /** Player is meleeing */
        std::uint16_t melee : 1;

        /** Player is actioning */
        std::uint16_t action : 1;

        PAD_BIT(std::uint16_t, 1);

        /** Player is using a flashlight */
        std::uint16_t flashlight : 1;

        PAD_BIT(std::uint16_t, 9);

        /** Player is pressing the reload key */
        std::uint16_t reload : 1;

        PAD_BIT(std::uint16_t, 2);

        PAD(0x2);
        PAD(0x18);

        /** XY aim; updated every remote_player_position_baseliine_update_rate ticks */
        float baseline_update_xy_aim;

        /** Aiming; updated every remote_player_position_baseliine_update_rate ticks */
        float baseline_update_z_aim;

        /** Player is moving forward (analog, -1 to 1); updated every remote_player_position_baseliine_update_rate ticks */
        float baseline_update_forward;

        /** Player is moving to the left (analog, -1 to 1); updated every remote_player_position_baseliine_update_rate ticks */
        float baseline_update_left;

        /** Rate of fire (0 = min, 1 = max); updated every remote_player_position_baseliine_update_rate ticks */
        float baseline_update_rate_of_fire;

        /** Current weapon slot; updated every remote_player_position_baseliine_update_rate ticks */
        std::uint16_t baseline_update_weapon_slot;

        /** Current grenade slot; updated every remote_player_position_baseliine_update_rate ticks */
        std::uint16_t baseline_update_grenade_slot;

        PAD(0x4);

        /** Player aiming thingy; updated every remote_player_position_baseline_update_rate ticks */
        Point3D update_aiming;

        PAD(0x10);

        /** Position; updated every remote_player_position_baseliine_update_rate ticks */
        Point3D update_position;

        PAD(0x84);

        /**
         * Get the full ID of the player
         * @return the full ID of the player
         */
        PlayerID get_full_id() const noexcept;
    };
    static_assert(sizeof(Player) == 0x200);

    /**
     * Get the ID of the client player
     * @return ID of the client player
     */
    PlayerID get_client_player_id() noexcept;

    struct PlayerTable : GenericTable<Player> {
        /**
         * Get the player
         * @param  player_id ID of the player to get
         * @return           the player if found, or nullptr if not
         */
        Player *get_player(PlayerID player_id) noexcept;

        /**
         * Get a player by rcon ID (0-15)
         * @return pointer to the player if found
         */
        Player *get_player_by_rcon_id(std::size_t rcon_id) noexcept;

        /**
         * Get the client player
         * @return pointer to the client player
         */
        Player *get_client_player() noexcept;

        /**
         * Get the player table
         * @return reference to the player table
         */
        static PlayerTable &get_player_table() noexcept;
    };
}

#endif
