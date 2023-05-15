// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>
#include "checkpoint_fix.hpp"
#include "../event/tick.hpp"
#include "../event/frame.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::uint8_t *saving;

    //Disable checkpoint function.
    static void block_cp() noexcept {
        overwrite(get_chimera().get_signature("checkpoint_sig").data(), static_cast<std::uint8_t>(0xC3));
        overwrite(get_chimera().get_signature("checkpoint_sig").data() + 0x1, static_cast<std::uint32_t>(0x90909090));
    }

    // Check at end of every frame if the game is trying to save and if so, block the checkpoint function until next tick.
    static void game_save_frame() noexcept {
        if(*saving) {
            block_cp();
        }
    }

    // Re-enable checkpoints every tick to ensure one counter update cycle passes every tick.
    static void game_save_tick() noexcept {
        get_chimera().get_signature("checkpoint_sig").rollback();
    }

    void set_up_checkpoint_fix() noexcept {
        auto *save_func = get_chimera().get_signature("checkpoint_sig").data();
        saving = *reinterpret_cast<std::uint8_t **>(save_func + 195);
        add_frame_event(game_save_frame);
        add_pretick_event(game_save_tick);
    }
}
