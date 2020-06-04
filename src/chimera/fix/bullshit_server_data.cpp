// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

namespace Chimera {
    extern "C" void *check_for_bullshit_server_data_asm() noexcept;

    void set_up_bullshit_server_data_fix() noexcept {
        auto *data = get_chimera().get_signature("read_query_value_sig").data();
        static Hook hook;
        write_jmp_call(data + 1, hook, reinterpret_cast<const void *>(check_for_bullshit_server_data_asm), nullptr, false);
    }

    extern "C" const std::uint8_t *strip_bullshit_server_data(const char *key, const std::uint8_t *bullshit_server_data) noexcept {
        static const char *filtered_out = "Halo";

        // If this is not hostname, do nothing
        if(std::strcmp(key, "hostname") != 0) {
            return bullshit_server_data;
        }

        while(*bullshit_server_data && *bullshit_server_data <= 0x20) {
            bullshit_server_data++;
        }
        if(*bullshit_server_data) {
            return bullshit_server_data;
        }
        else {
            return reinterpret_cast<const std::uint8_t *>(filtered_out);
        }
    }
}
