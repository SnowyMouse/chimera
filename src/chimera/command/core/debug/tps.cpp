// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/tick.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../output/output.hpp"
#include "../../../localization/localization.hpp"
#include "../../../command/command.hpp"
#include "../../../signature/signature.hpp"
#include "../../../signature/hook.hpp"
#include "../../../halo_data/multiplayer.hpp"

namespace Chimera {
    static float *tps = nullptr;
    static void revert_if_needed() noexcept;
    bool tps_command(int argc, const char **argv) noexcept {
        if(!tps) {
            tps = *reinterpret_cast<float **>(get_chimera().get_signature("tick_rate_sig").data() + 2);
        }
        if(argc) {
            float new_tps = std::atof(*argv);
            if(new_tps < 0.0F) {
                new_tps = 0.0F;
            }
            if(new_tps == 30.0F) {
                remove_pretick_event(revert_if_needed);
            }
            else {
                if(server_type() == ServerType::SERVER_DEDICATED) {
                    console_error(localize("chimera_error_must_be_host"));
                    return false;
                }
                add_pretick_event(revert_if_needed);
            }
            overwrite(tps, new_tps);
        }
        console_output("%f", *tps);
        return true;
    }

    static void revert_if_needed() noexcept {
        if(server_type() == ServerType::SERVER_DEDICATED) {
            get_chimera().execute_command("chimera_tps 30.0", nullptr, false);
        }
    }
}
