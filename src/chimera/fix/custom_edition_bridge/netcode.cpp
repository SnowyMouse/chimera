// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../output/output.hpp"

namespace Chimera {
    extern "C" {
        void override_master_server_list_asm() noexcept;
        bool custom_edition_netcode_is_enabled = false;
    }

    void enable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();

        // Version hack (for supporting servers that don't allow bullshit clients to join)
        overwrite(chimera.get_signature("version_join_sig").data() + 3, static_cast<std::uint32_t>(0x00096A27)); // hack the version to this

        // Master server
        static Hook master_server_hook;
        write_jmp_call(get_chimera().get_signature("server_list_query_info_sig").data() + 7, master_server_hook, nullptr, reinterpret_cast<const void *>(override_master_server_list_asm), false);

        custom_edition_netcode_is_enabled = true;
    }

    void disable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("version_join_sig").rollback();
        // chimera.get_signature("server_list_query_info_sig").rollback(); // intentional - halom might still be cached

        custom_edition_netcode_is_enabled = false;
    }
}
