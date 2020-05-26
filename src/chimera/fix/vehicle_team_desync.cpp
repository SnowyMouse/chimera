#include "vehicle_team_desync.hpp"
#include "../chimera.hpp"
#include "../halo_data/multiplayer.hpp"
#include "../signature/hook.hpp"
#include "../event/map_load.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static void on_map_load() noexcept {
        add_map_load_event(on_map_load);

        auto &s1 = get_chimera().get_signature("vehicle_team_desync_1_sig");
        auto &s2 = get_chimera().get_signature("vehicle_team_desync_2_sig");

        if(server_type() != ServerType::SERVER_DEDICATED) {
            s1.rollback();
            s2.rollback();
        }
        else {
            auto *a = s1.data() + 7;
            auto *b = s2.data() + 5;

            static constexpr SigByte fix[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
            write_code_s(a, fix);
            write_code_s(b, fix);
        }
    }

    void set_up_vehicle_team_desync_fix() noexcept {
        if(!get_chimera().feature_present("client_mtv_desync")) {
            return;
        }

        add_map_load_event(on_map_load);
    }
}
