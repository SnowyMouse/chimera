// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/chat.hpp"
#include "../../../event/tick.hpp"
#include "../../../halo_data/multiplayer.hpp"

namespace Chimera {
    static int throttle_time = 0;

    static void reduce_throttle() {
        if(--throttle_time < 0) {
            throttle_time = 0;
            remove_tick_event(reduce_throttle);
        }
    }

    bool send_chat_message_command(int, const char **argv) noexcept {
        if(server_type() == ServerType::SERVER_NONE) {
            console_error(localize("chimera_error_must_be_in_a_server"));
            return false;
        }

        // Get the channel
        int channel;
        try {
            channel = std::stoi(argv[0]);
        }
        catch(std::exception &) {
            console_error(localize("chimera_send_chat_message_invalid_channel"), argv[0]);
            return false;
        }

        // Keep the player from muting themselves by mistake
        if(server_type() == ServerType::SERVER_DEDICATED) {
            auto tick_rate = effective_tick_rate();
            if(throttle_time > tick_rate * 3.0F * 2) {
                console_error(localize("chimera_send_chat_message_throttled"));
                return false;
            }

            throttle_time += tick_rate * 3.0F;
            add_tick_event(reduce_throttle);
        }

        // Send it!
        chat_out(channel, argv[1]);

        return true;
    }
}
