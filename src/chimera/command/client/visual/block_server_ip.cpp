// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool block_server_ip_command(int argc, const char **argv) {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                auto &join_server_ip_text_sig = get_chimera().get_signature("join_server_ip_text_sig");
                auto &f1_ip_text_render_call_sig = get_chimera().get_signature("f1_ip_text_render_call_sig");
                auto &create_server_ip_text_sig = get_chimera().get_signature("create_server_ip_text_sig");
                if(new_enabled) {
                    const SigByte mod[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
                    write_code_s(join_server_ip_text_sig.data() + 5, mod);
                    write_code_s(f1_ip_text_render_call_sig.data(), mod);

                    const SigByte mod_create_server_ip[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0x90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x66, 0xB9, 0x00, 0x00, 0x90, 0x90, 0x90 };
                    write_code_s(create_server_ip_text_sig.data(), mod_create_server_ip);
                }
                else {
                    join_server_ip_text_sig.rollback();
                    f1_ip_text_render_call_sig.rollback();
                    create_server_ip_text_sig.rollback();
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));

        return true;
    }
}
