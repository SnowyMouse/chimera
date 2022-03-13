// SPDX-License-Identifier: GPL-3.0-only

#include "master_server.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../config/ini.hpp"

namespace Chimera {
    void set_master_server_connection_threads(std::int8_t threads) noexcept {
        overwrite(get_chimera().get_signature("setup_master_server_connection_sig").data() + 10, threads);
    }

    char list_server_c[512] = "";
    char key_server_c[512] = "";
    char nat_neg_primary_c[512] = "";
    char nat_neg_secondary_c[512] = "";

    void set_master_server(const char *list, const char *key, const char *natneg1, const char *natneg2) {
        auto *ini = get_chimera().get_ini();
        std::string list_server = ini->get_value_string("server_list.master_server_list").value_or("s1.master.hosthpc.com"),
                    key_server = ini->get_value_string("server_list.master_server_key").value_or("s1.ms01.hosthpc.com"),
                    nat_neg_primary = ini->get_value_string("server_list.master_server_nat_primary").value_or("natneg1.hosthpc.com"),
                    nat_neg_secondary = ini->get_value_string("server_list.master_server_nat_secondary").value_or("natneg2.hosthpc.com");
        if(!list) {
            list = list_server.c_str();
        }
        if(!key) {
            key = key_server.c_str();
        }
        if(!natneg1) {
            natneg1 = nat_neg_primary.c_str();
        }
        if(!natneg2) {
            natneg2 = nat_neg_secondary.c_str();
        }

        std::strncpy(list_server_c, list, sizeof(list_server_c) - 1);
        std::strncpy(key_server_c, key, sizeof(key_server_c) - 1);
        std::strncpy(nat_neg_primary_c, natneg1, sizeof(nat_neg_primary_c) - 1);
        std::strncpy(nat_neg_secondary_c, natneg2, sizeof(nat_neg_secondary_c) - 1);
    }

    static const char *server_1 = key_server_c;
    static const char *server_2 = list_server_c;
    static const char *server_3 = nat_neg_primary_c;
    static const char *server_4 = nat_neg_secondary_c;
    static const char * const *server_3_ptr = &server_3;
    static const char * const *server_4_ptr = &server_4;

    void set_up_master_server() noexcept {
        overwrite(get_chimera().get_signature("master_server_1_sig").data() + 1, server_1);
        overwrite(get_chimera().get_signature("master_server_2_sig").data() + 1, server_2);
        overwrite(get_chimera().get_signature("master_server_3_sig").data() + 1, server_3_ptr);
        overwrite(get_chimera().get_signature("master_server_4_sig").data() + 1, server_4_ptr);

        set_master_server(nullptr, nullptr, nullptr, nullptr);
    }
    void get_master_server(const char **list, const char **key, const char **natneg1, const char **natneg2) {
        *list = list_server_c;
        *key = key_server_c;
        *natneg1 = nat_neg_primary_c;
        *natneg2 = nat_neg_secondary_c;
    }
}
