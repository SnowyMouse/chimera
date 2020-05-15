#include "../../../localization/localization.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/chat.hpp"
#include "../../../halo_data/multiplayer.hpp"

namespace Chimera {
    bool send_chat_message_command(int argc, const char **argv) noexcept {
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
            console_error(localize("send_chat_message_invalid_channel"), argv[0]);
        }

        // Send it!
        chat_out(channel, argv[1]);

        return true;
    }
}
