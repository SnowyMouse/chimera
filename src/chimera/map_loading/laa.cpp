// SPDX-License-Identifier: GPL-3.0-only

#include "laa.hpp"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../chimera.hpp"

namespace Chimera {
    bool current_exe_is_laa_patched() {
        for(const std::byte *i = reinterpret_cast<std::byte *>(0x400000) ;; i++) {
            if(memcmp(i, "PE\x00\x00", 4) == 0) {
                i += 0x16;
                auto value = static_cast<unsigned char>(*i);
                return value & IMAGE_FILE_LARGE_ADDRESS_AWARE;
            }
        }
        return false;
    }
}
