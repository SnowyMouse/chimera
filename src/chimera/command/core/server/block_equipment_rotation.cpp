// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../chimera.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../halo_data/multiplayer.hpp"

namespace Chimera {
    extern "C" void null_equipment_spawn_rotation() noexcept;

    bool block_equipment_rotation_command(int argc, const char **argv) noexcept {
        static bool active = false;

        if(argc) {
            auto &sig = get_chimera().get_signature("weapon_rotation_spawn_sig");
            bool new_active = STR_TO_BOOL(*argv);
            if(new_active != active) {
                active = new_active;
                if(!active) {
                    sig.rollback();
                }
                else {
                    static Hook hook;

                    // Nop out the existing fld sig
                    const SigByte patch[] = { 0x90, 0x90, 0x90 };
                    write_code_s(sig.data(), patch);

                    write_jmp_call(sig.data(), hook, reinterpret_cast<const void *>(null_equipment_spawn_rotation), nullptr, false);
                }
            }
        }

        console_output(BOOL_TO_STR(active));
        return true;
    }
}
