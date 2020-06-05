// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../chimera.hpp"
#include "../../halo_data/keyboard.hpp"
#include "../../signature/hook.hpp"
#include "../../output/output.hpp"

namespace Chimera {
    extern "C" void override_master_server_list_asm() noexcept;

    void set_up_custom_edition_master_server_support() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("server_list_query_info_sig").data() + 7, hook, nullptr, reinterpret_cast<const void *>(override_master_server_list_asm));
    }

    extern "C" void do_override_master_server(char *server_list) noexcept {
        auto &controls = get_keyboard_keys();
        std::strcpy(server_list, (controls.left_shift || controls.right_shift) && (controls.left_control || controls.right_control) ? "halom" : "halor");
    }
}
