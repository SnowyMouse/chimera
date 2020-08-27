// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ENCODE_HPP
#define CHIMERA_ENCODE_HPP

#include <string>

namespace Chimera {
    /**
     * Encode UTF-8 to UTF-16
     * @param  str UTF-8 input
     * @return     UTF-16 output
     */
    std::wstring u8_to_u16(const char *str);

    /**
     * Encode UTF-16 to UTF-8
     * @param  strw UTF-16 input
     * @return      UTF-8 output
     */
    std::string u16_to_u8(const wchar_t *strw);
}

#endif
