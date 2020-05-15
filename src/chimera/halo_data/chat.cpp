#include "chat.hpp"

namespace Chimera {
    extern "C" void chat_out_asm(int channel, const wchar_t *message);

    void chat_out(int channel, const wchar_t *message) {
        if(!message) {
            return;
        }
        chat_out_asm(channel, message);
    }
    void chat_out(int channel, const char *message) {
        if(!message) {
            return;
        }
        wchar_t message_to_send[256] = {};
        for(int s = 0; s < sizeof(message_to_send) / sizeof(*message_to_send) - 1 && message[s]; s++) {
            message_to_send[s] = message[s];
        }
        chat_out_asm(channel, message_to_send);
    }
}
