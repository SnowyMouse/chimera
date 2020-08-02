// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static bool block_zoom_blur_disabled = false;
    static bool zoom_blur_block_on = false;

    static void block_zoom_blur(bool new_value) {
        static bool enabled = false;

        if(block_zoom_blur_disabled) {
            new_value = false;
        }

        auto &zoom_blur_1_s = get_chimera().get_signature("zoom_blur_1_sig");
        auto &zoom_blur_2_s = get_chimera().get_signature("zoom_blur_2_sig");
        auto &zoom_blur_3_s = get_chimera().get_signature("zoom_blur_3_sig");

        static const SigByte zoom_blur_1_mod[] = {  -1,   -1, 0x38,   -1,   -1, 0x38};
        static const SigByte zoom_blur_2_mod[] = {  -1,   -1, 0x38};
        static const SigByte zoom_blur_3_mod[] = {0x01};

        if(new_value != enabled) {
            if(new_value) {
                write_code_s(zoom_blur_1_s.data(), zoom_blur_1_mod);
                write_code_s(zoom_blur_2_s.data(), zoom_blur_2_mod);
                write_code_s(zoom_blur_3_s.data(), zoom_blur_3_mod);
            }
            else {
                zoom_blur_1_s.rollback();
                zoom_blur_2_s.rollback();
                zoom_blur_3_s.rollback();
            }
            enabled = new_value;
        }
    }

    void set_block_zoom_blur_disabled(bool disabled) {
        block_zoom_blur_disabled = disabled;
        block_zoom_blur(zoom_blur_block_on);
    }

    bool block_zoom_blur_command(int argc, const char **argv) {
        if(argc) {
            zoom_blur_block_on = STR_TO_BOOL(*argv);
            block_zoom_blur(zoom_blur_block_on);
        }

        console_output(BOOL_TO_STR(zoom_blur_block_on));

        return true;
    }
}
