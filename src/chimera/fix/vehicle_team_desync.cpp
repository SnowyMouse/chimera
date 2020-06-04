// SPDX-License-Identifier: GPL-3.0-only

#include "vehicle_team_desync.hpp"
#include "../chimera.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" void on_check_for_desynced_vehicle_entry_asm();
    extern "C" bool on_check_for_desynced_vehicle_entry() {
        return server_type() == ServerType::SERVER_DEDICATED;
    }

    void set_up_vehicle_team_desync_fix() noexcept {
        if(!get_chimera().feature_present("client_mtv_desync")) {
            return;
        }

        auto &s1 = get_chimera().get_signature("vehicle_team_desync_1_sig");
        auto &s2 = get_chimera().get_signature("vehicle_team_desync_2_sig");

        auto *a = s1.data() + 7;
        auto *b = s2.data() + 5;

        static Hook h1, h2;
        write_jmp_call(a, h1, nullptr, reinterpret_cast<void *>(on_check_for_desynced_vehicle_entry_asm), false);
        write_jmp_call(b, h2, nullptr, reinterpret_cast<void *>(on_check_for_desynced_vehicle_entry_asm), false);
    }
}
