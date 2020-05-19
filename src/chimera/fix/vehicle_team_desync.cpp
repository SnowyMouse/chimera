#include "vehicle_team_desync.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/output.hpp"

namespace Chimera {
    void set_up_vehicle_team_desync_fix() noexcept {
        if(!get_chimera().feature_present("client_mtv_desync")) {
            return;
        }
        auto *a = get_chimera().get_signature("vehicle_team_desync_1_sig").data() + 7;
        auto *b = get_chimera().get_signature("vehicle_team_desync_2_sig").data() + 5;

        static constexpr SigByte fix[] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
        write_code_s(a, fix);
        write_code_s(b, fix);
    }
}
