// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include "encode.hpp"

namespace Chimera {
    std::wstring u8_to_u16(const char *str) {
        wchar_t strw[1024] = {};
        if(MultiByteToWideChar(CP_UTF8, 0, str, -1, strw, sizeof(strw) / sizeof(*strw)) == 0) {
            return std::wstring();
        }
        else {
            return std::wstring(strw);
        }
    }

    std::string u16_to_u8(const wchar_t *strw) {
        char str[1024] = {};
        if(WideCharToMultiByte(CP_UTF8, 0, strw, -1, str, sizeof(str) / sizeof(*str), nullptr, nullptr) == 0) {
            return std::string();
        }
        else {
            return std::string(str);
        }
    }
}
