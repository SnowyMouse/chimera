// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/map.hpp"
#include "../event/map_load.hpp"
#include "../output/output.hpp"

namespace Chimera {
    static void on_map_load() noexcept {
        auto &fix_biped_ui_spawn_sig = get_chimera().get_signature("fix_biped_spawning_on_ui_sig");

        // Check if current map is UI type
        if (get_map_header().game_type == MapGameType::MAP_USER_INTERFACE) {
            // There is a float compare operation after this instruction that results in a an
            // empty register being read from. This causes a crash. We can just skip the instruction
            // and jump to the end of the function, just in the UI case.

            static SigByte skip[] = { 0xE9, 0x44, 0x01, 0x00, 0x00 };
            // NOTE: We might need to investigate this further to fix it properly.
            write_code_s(fix_biped_ui_spawn_sig.data() + 0x6B, skip);
            return;
        }
        // Revert to original instructions
        fix_biped_ui_spawn_sig.rollback();
    }

    void set_up_fix_biped_ui_spawn() noexcept {
        if(get_chimera().feature_present("client_custom_edition")) {
            add_map_load_event(on_map_load);
        }
    }
}
