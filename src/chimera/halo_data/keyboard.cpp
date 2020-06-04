// SPDX-License-Identifier: GPL-3.0-only

#include "keyboard.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    KeyboardKeys &get_keyboard_keys() noexcept {
        static KeyboardKeys *buffer = nullptr;
        if(!buffer) {
            buffer = *reinterpret_cast<KeyboardKeys **>(get_chimera().get_signature("keyboard_keys_sig").data() + 1);
        }
        return *buffer;
    }
}
