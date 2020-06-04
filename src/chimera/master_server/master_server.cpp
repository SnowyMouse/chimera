// SPDX-License-Identifier: GPL-3.0-only

#include "master_server.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_master_server_connection_threads(std::int8_t threads) noexcept {
        overwrite(get_chimera().get_signature("setup_master_server_connection_sig").data() + 10, threads);
    }

    static const char *server_1 = "s1.ms01.hosthpc.com";
    static const char *server_2 = "s1.master.hosthpc.com";
    static const char *server_3 = "natneg1.hosthpc.com";
    static const char *server_4 = "natneg2.hosthpc.com";
    static const char * const *server_3_ptr = &server_3;
    static const char * const *server_4_ptr = &server_4;

    void set_up_demo_master_server() noexcept {
        overwrite(get_chimera().get_signature("demo_master_server_1_sig").data() + 1, server_1);
        overwrite(get_chimera().get_signature("demo_master_server_2_sig").data() + 1, server_2);
        overwrite(get_chimera().get_signature("demo_master_server_3_sig").data() + 1, server_3_ptr);
        overwrite(get_chimera().get_signature("demo_master_server_4_sig").data() + 1, server_4_ptr);
    }
}
