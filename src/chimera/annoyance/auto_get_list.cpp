// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        std::uint32_t auto_get_list_thing_asm() noexcept;
        std::uint32_t auto_get_list_thing_add_asm() noexcept;
    }

    void set_up_auto_get_list() noexcept {
        auto &chimera = get_chimera();

        if(!chimera.get_ini()->get_value_bool("server_list.auto_query").value_or(false)) {
            return;
        }

        static Hook hook_get, hook_add;
        auto *auto_query_master_list_sig = chimera.get_signature("auto_query_master_list_sig").data();
        auto *auto_query_master_list_add_sig = chimera.get_signature("auto_query_master_list_add_sig").data();

        write_jmp_call(auto_query_master_list_sig, hook_get, nullptr, reinterpret_cast<const void *>(auto_get_list_thing_asm), false);
        write_jmp_call(auto_query_master_list_add_sig, hook_add, nullptr, reinterpret_cast<const void *>(auto_get_list_thing_add_asm), false);
    }
}
