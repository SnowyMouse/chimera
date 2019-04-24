#ifndef CHIMERA_NAV_NUMBERS_HPP
#define CHIMERA_NAV_NUMBERS_HPP

namespace Chimera {
    /**
     * Fix the really wide numbers spacing in NAV points
     */
    void set_up_nav_numbers_fix() noexcept;

    /**
     * Disable the fix
     */
    void undo_nav_numbers_fix() noexcept;
}

#endif
