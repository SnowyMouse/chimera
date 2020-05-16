// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_LOCALIZATION_HPP
#define CHIMERA_LOCALIZATION_HPP

#include "../chimera.hpp"

namespace Chimera {
    /**
     * Get the localized form of the string
     * @param  string string to get
     * @return        localized form or the string, itself, if not found
     */
    const char *localize(const char *string, Language = get_chimera().get_language()) noexcept;
}

#endif
