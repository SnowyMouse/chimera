// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void enable_tab_out_video() noexcept {
        overwrite(get_chimera().get_signature("tab_out_video_1_sig").data() + 6, static_cast<std::uint8_t>(0xEB));
        overwrite(get_chimera().get_signature("tab_out_video_2_sig").data() + 0, static_cast<std::uint16_t>(0x9090));
        overwrite(get_chimera().get_signature("tab_out_video_3_sig").data() + 6, static_cast<std::uint8_t>(0xEB));
    }
}
