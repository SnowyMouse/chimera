// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INDEX_BUFFER_FIX_HPP
#define CHIMERA_INDEX_BUFFER_FIX_HPP

namespace Chimera {
    /**
     * Custom edition and demo create an index buffer 8 bytes wide, lock the first 4 bytes and still try to write to all 8.
     */
    void set_up_index_buffer_fix() noexcept;
}

#endif
