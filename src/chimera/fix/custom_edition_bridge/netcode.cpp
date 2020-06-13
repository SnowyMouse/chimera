// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../output/output.hpp"

namespace Chimera {
    void enable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();
        overwrite(chimera.get_signature("version_join_sig").data() + 3, static_cast<std::uint32_t>(0x00096A27)); // hack the version to this
    }

    void disable_custom_edition_netcode_support() noexcept {
        auto &chimera = get_chimera();
        chimera.get_signature("version_join_sig").rollback();
    }
}
