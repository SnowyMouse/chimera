// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../event/map_load.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    static void disable_allow_all_passengers() noexcept {
        get_chimera().get_signature("vehicle_team_final_boss_sig").rollback();
    }

    static void allow_all_passengers() noexcept {
        overwrite(get_chimera().get_signature("vehicle_team_final_boss_sig").data(), static_cast<std::uint8_t>(server_type() == ServerType::SERVER_LOCAL ? 0xEB : 0x74));
    }

    bool allow_all_passengers_command(int argc, const char **argv) {
        static bool active = false;

        if(argc) {
            bool new_active = STR_TO_BOOL(*argv);
            if(new_active != active) {
                active = new_active;
                if(active) {
                    add_map_load_event(allow_all_passengers);
                    allow_all_passengers();
                }
                else {
                    remove_map_load_event(allow_all_passengers);
                    disable_allow_all_passengers();
                }
            }
        }

        console_output(BOOL_TO_STR(active));
        return true;
    }
}
