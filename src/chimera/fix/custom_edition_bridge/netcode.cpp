// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../output/output.hpp"
#include "../../map_loading/fast_load.hpp"
#include "../../map_loading/map_loading.hpp"

namespace Chimera {
    extern "C" {
        void override_master_server_list_asm() noexcept;
        bool custom_edition_netcode_is_enabled = false;
        void override_join_name_packet_asm() noexcept;
        void override_join_name_packet_size_asm() noexcept;
        void calculate_crc32_of_current_map_file_please(const std::uint8_t *key, std::uint8_t *output) {
            auto crc = *get_map_entry(get_map_header().name)->crc32;

            // From "Halo Update" by Samuco (which I originally helped make) <https://github.com/ShadovvMoon/HaloMC/blob/e611b2319c40c101dfa35f83b798019933f505c6/HaloUpdate/HaloJS/SMHaloUP.m#L1269>
            std::uint8_t b0 = static_cast<std::uint8_t>(crc >> (8 * 3));
            std::uint8_t b1 = static_cast<std::uint8_t>(crc >> (8 * 2));
            std::uint8_t b2 = static_cast<std::uint8_t>(crc >> (8 * 1));
            std::uint8_t b3 = static_cast<std::uint8_t>(crc >> (8 * 0));

            output[0] = key[3] ^ b0;
            output[1] = key[2] ^ key[6] ^ b1;
            output[2] = key[1] ^ key[5] ^ b2;
            output[3] = key[0] ^ key[4] ^ b3;
        }
    }

    void enable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();

        // Version hack (for supporting servers that don't allow bullshit clients to join)
        overwrite(chimera.get_signature("version_join_sig").data() + 3, static_cast<std::uint32_t>(0x00096A27)); // hack the version to this

        return;

        // Master server
        static Hook master_server_hook;
        write_jmp_call(chimera.get_signature("server_list_query_info_sig").data() + 7, master_server_hook, nullptr, reinterpret_cast<const void *>(override_master_server_list_asm), false);

        // We need to be able send the CRC32
        static Hook prepare_challenge_packet_hook;
        write_jmp_call(chimera.get_signature("prepare_challenge_packet_sig").data(), prepare_challenge_packet_hook, nullptr, reinterpret_cast<const void *>(override_join_name_packet_asm), true);

        // Change the packet size, too
        static Hook prepare_challenge_packet_size_hook;
        write_jmp_call(chimera.get_signature("prepare_challenge_packet_size_sig").data(), prepare_challenge_packet_size_hook, nullptr, reinterpret_cast<const void *>(override_join_name_packet_size_asm), false);

        custom_edition_netcode_is_enabled = true;
    }

    void disable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("version_join_sig").rollback();
        // chimera.get_signature("server_list_query_info_sig").rollback(); // intentional comment-out - halom might still be cached
        chimera.get_signature("prepare_challenge_packet_sig").rollback();
        chimera.get_signature("prepare_challenge_packet_size_sig").rollback();

        custom_edition_netcode_is_enabled = false;
    }
}
