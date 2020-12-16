// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/error_box.hpp"

namespace Chimera {
    void enable_multiple_instance() noexcept {
        if(!get_chimera().feature_present("client_multiple_instances")) {
            show_error_box("Error", "Multiple instances is not supported on this client. (client is probably modified)");
            std::exit(1);
        }

        static SigByte nop5[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(get_chimera().get_signature(!get_chimera().feature_present("client_demo") ? "multiple_instance_1_fv_sig" : "multiple_instance_1_demo_sig").data(), nop5);

        static SigByte save_patch[] = { 0xEB, 0x4A };
        write_code_s(get_chimera().get_signature("multiple_instance_2_sig").data(), save_patch);
    }
}
