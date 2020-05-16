// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SERVER_HPP
#define CHIMERA_SERVER_HPP

#include <cstdint>
#include "type.hpp"

namespace Chimera {
    struct Player;

    /**
     * This is a player in the server info player table. This information is used in chat and sv_players.
     */
    struct ServerInfoPlayer {
        /** Name of the player */
        wchar_t name[0xC];

        /** Armor color of the player; unused? */
        std::uint8_t armor_color;

        PAD(0x3);

        /** Index of the player in the machine table; 0xFF if not present */
        std::uint8_t machine_index;

        /** Status of the player; 0xFF if not present */
        std::uint8_t status;

        /** Team of the player; 0xFF if not present */
        std::uint8_t team;

        /** Index of the player in the player table; 0xFF if not present */
        std::uint8_t player_id;

        /**
         * Get the player in the player table
         * @return pointer to the player in the player table, or nullptr if not found
         */
        Player *get_player_table_player() const noexcept;
    };
    static_assert(sizeof(ServerInfoPlayer) == 0x20);

    /**
     * Machine connection info
     */
    struct ServerInfoMachineConnectionInfo {
        /** IP address */
        std::uint32_t ip_address;

        /** Port */
        std::uint16_t port;

        PAD(0x2);

        /** Map this out later? */
        PAD(0x148);
    };

    /**
     * Machine info - Retail server & client/Custom Edition client
     */
    struct ServerInfoMachine {
        /** Information about a player's connection */
        ServerInfoMachineConnectionInfo ***connection_info;

        PAD(0x8);

        /** Index in player table I think. Phasor says this is used for rcon. */
        std::uint16_t player_index;

        /** Equals 7; For some reason a player cannot chat if this is too much lower than 7 */
        std::uint16_t unknown;

        PAD(0x42);

        /** Key used in handshake */
        char key[10];

        /** Increases by 1 for each connection */
        std::uint32_t player_number;
    };
    static_assert(sizeof(ServerInfoMachine) == 0x60);

    /**
     * Machine info - Custom Edition dedicated server only
     */
    struct ServerInfoMachineCustomEditionDedicatedServer : ServerInfoMachine {
        /** This seems to be set to the player name when the player leaves */
        wchar_t last_player_name[12];

        PAD(0x8);

        /** IP:port of the player in ascii format */
        char ip_address[0x20];

        /** CD key hash in ascii format */
        char cd_key_hash[0x20];

        PAD(0x2C);
    };
    static_assert(sizeof(ServerInfoMachineCustomEditionDedicatedServer) == 0xEC);

    /**
     * Player list stuff
     */
    struct ServerInfoPlayerList {
        PAD(0x1);

        /** Maximum players in the server */
        std::uint8_t max_players;

        PAD(0x2);

        /** Number of players */
        std::uint8_t player_count;

        PAD(0x1);

        /** Stuff used with sv_players */
        ServerInfoPlayer players[16];

        PAD(0xE);

        // ServerInfoMachine[16] or ServerInfoMachineCustomEditionDedicatedServer[16]

        /**
         * Get the server info player list
         * @return pointer to the server info player list or nullptr if not possible
         */
        static ServerInfoPlayerList *get_server_info_player_list() noexcept;

        /**
         * Get the player struct by its ID
         * @param  player id of the player
         * @return        pointer to the player or nullptr if not valid
         */
        ServerInfoPlayer *get_player(PlayerID player) noexcept;

        // Nope
        ServerInfoPlayerList() = delete;
    };
    static_assert(sizeof(ServerInfoPlayerList) == 0x214);

    /**
     * This is server information (players, machines, etc.)
     */
    struct ServerInfo {
        PAD(0x8);

        /** Name of the server */
        wchar_t server_name[0x42];

        /** Name of the currently loaded map */
        char map_name[0x80];

        /** Name of the gametype */
        wchar_t gametype[0x18];

        /** There is a lot of fun stuff in here that I'm not sure about */
        PAD(0x68);

        // Add another 0x40 bytes if on Custom Edition


        /**
         * Get the machine struct of the player.
         * @param  machine index of the machine
         * @return         pointer to the machine or nullptr if not valid
         */
        ServerInfoMachine &get_machine(std::uint8_t machine) noexcept;

        /**
         * Get the server info struct
         * @return pointer to the server info struct or nullptr if not possible
         */
        static ServerInfo *get_server_info() noexcept;

        /** Nope */
        ServerInfo() = delete;
    };
    static_assert(sizeof(ServerInfo) == 0x1A4);
}

#endif
