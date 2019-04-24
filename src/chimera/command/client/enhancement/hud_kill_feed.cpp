#include "../../command.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"
#include "../../../output/output.hpp"

namespace Chimera {
    bool hud_kill_feed_command(int argc, const char **argv) {
        static bool active = false;
        if(argc == 1) {
            bool new_value = STR_TO_BOOL(argv[0]);
            if(new_value != active) {
                auto &hud_kill_feed_sig = get_chimera().get_signature("hud_kill_feed_sig");
                auto &hud_kill_feed_host_kill_sig = get_chimera().get_signature("hud_kill_feed_host_kill_sig");
                auto &hud_kill_feed_host_betray_sig = get_chimera().get_signature("hud_kill_feed_host_betray_sig");
                if(new_value) {
                    overwrite(hud_kill_feed_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_sig.data() + 5));
                    overwrite(hud_kill_feed_host_kill_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_host_kill_sig.data() + 5));
                    overwrite(hud_kill_feed_host_betray_sig.data() + 1, reinterpret_cast<int>(static_cast<void (*)(const wchar_t *)>(hud_output_raw)) - reinterpret_cast<int>(hud_kill_feed_host_betray_sig.data() + 5));
                }
                else {
                    hud_kill_feed_sig.rollback();
                    hud_kill_feed_host_kill_sig.rollback();
                    hud_kill_feed_host_betray_sig.rollback();
                }
                active = new_value;
            }
        }
        console_output(BOOL_TO_STR(active));
        return true;
    }
}
