// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_WATSON_HPP
#define CHIMERA_WATSON_HPP

namespace Chimera {
    /**
     * Disable Watson (doesn't work in 2020 due to the servers being down - makes crashes even more annoying)
     */
    void remove_watson() noexcept;
}

#endif
