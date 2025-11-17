// SPDX-License-Identifier: GPL-3.0-only

#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/controls.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/game_engine.hpp"

extern "C" {
    void auto_uncrouch_asm() noexcept;
}

extern "C" std::uint32_t auto_uncrouch_cpp() {
    using namespace Chimera;
    float move_forward;
    float move_left;
    if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
        auto &controls = get_custom_edition_controls();
        move_forward = controls.move_forward;
        move_left = controls.move_left;
    }
    else {
        auto &controls = get_retail_demo_controls();
        move_forward = controls.move_forward;
        move_left = controls.move_left;
    }

    if(std::abs(move_forward) >= 0.98F || std::abs(move_left) >= 0.98F) {
        auto *player = PlayerTable::get_player_table().get_client_player();
        if(player) {
            auto &object_table = ObjectTable::get_object_table();
            auto *object = object_table.get_dynamic_object(player->object_id);
            if(object && object->on_ground) {
                return 1;
            }
        }
    }
    return 0;
}

namespace Chimera {
    bool auto_uncrouch_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                static Hook control_bitmasks_hook;
                auto *control_bitmask_data = get_chimera().get_signature("control_bitmask_sig").data();
                if(new_value) {
                    write_jmp_call(control_bitmask_data, control_bitmasks_hook, reinterpret_cast<const void *>(auto_uncrouch_asm));
                }
                else {
                    control_bitmasks_hook.rollback();
                }
                active = new_value;
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
