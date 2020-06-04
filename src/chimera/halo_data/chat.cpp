// SPDX-License-Identifier: GPL-3.0-only

#include "chat.hpp"
#include <algorithm>
#include <windows.h>
#include <cstring>

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
        MultiByteToWideChar(CP_UTF8, 0, message, -1, message_to_send, sizeof(message_to_send) / sizeof(*message_to_send));
        chat_out_asm(channel, message_to_send);
    }
}
