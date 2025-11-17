// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "controls.hpp"

namespace Chimera {
    ControlsCustomEdition &get_custom_edition_controls() noexcept {
        static std::optional<ControlsCustomEdition *> controls_table;
        if(!controls_table.has_value()) {
            controls_table = *reinterpret_cast<ControlsCustomEdition **>(get_chimera().get_signature("controls_sig").data() + 11);
        }
        return **controls_table;
    }

    ControlsRetailDemo &get_retail_demo_controls() noexcept {
        static std::optional<ControlsRetailDemo *> controls_table;
        if(!controls_table.has_value()) {
            controls_table = *reinterpret_cast<ControlsRetailDemo **>(get_chimera().get_signature("controls_sig").data() + 11);
        }
        return **controls_table;
    }
}
