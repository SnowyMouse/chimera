// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

// decorated with prefix tab_out_video_ to avoid name clashes later
extern "C" {
    std::uint8_t *tab_out_video_disable_video_ptr = nullptr;
    const void* tab_out_video_device_subroutine_original = nullptr;
    void tab_out_video_reacquire_device_subroutine_asm() noexcept; // dont call this directly
}

namespace Chimera {
    void enable_tab_out_video() noexcept {
        overwrite(get_chimera().get_signature("tab_out_video_1_sig").data() + 6, static_cast<std::uint8_t>(0xEB));
        overwrite(get_chimera().get_signature("tab_out_video_2_sig").data() + 0, static_cast<std::uint16_t>(0x9090));
        overwrite(get_chimera().get_signature("tab_out_video_3_sig").data() + 6, static_cast<std::uint8_t>(0xEB));
        
        tab_out_video_disable_video_ptr = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("tab_out_video_1_sig").data() + 2);
        
        static Hook hook_video_device_reacquire_sub;
        write_function_override(get_chimera().get_signature("renderer_begin_scene_reacquire_device_subroutine_sig").data() + 9, hook_video_device_reacquire_sub,
                                reinterpret_cast<const void *>(tab_out_video_reacquire_device_subroutine_asm), &tab_out_video_device_subroutine_original);
    }
}
