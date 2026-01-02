// SPDX-License-Identifier: GPL-3.0-only

#include "map_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/map.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/game_engine.hpp"
#include "../event/map_load.hpp"

namespace Chimera {
    ChimeraFixes global_fix_flags;

    void set_fix_config_for_new_map() noexcept {
        // Reset flags
        global_fix_flags.gearbox_chicago_multiply = false;
        global_fix_flags.gearbox_meters = false;
        global_fix_flags.gearbox_multitexture_blend_modes = false;
        global_fix_flags.alternate_bump_attenuation = false;
        global_fix_flags.gearbox_bump_attenuation = false;
        global_fix_flags.invert_detail_after_reflection = false;
        global_fix_flags.embedded_lua = false;
        global_fix_flags.refined_number_scale = false;

        auto &map_header = get_map_header();

        // Do not apply to stock campaign.
        if( std::strcmp("ui",map_header.name) == 0 ||
            std::strcmp("a10",map_header.name) == 0 ||
            std::strcmp("a30",map_header.name) == 0 ||
            std::strcmp("a50",map_header.name) == 0 ||
            std::strcmp("b30",map_header.name) == 0 ||
            std::strcmp("b40",map_header.name) == 0 ||
            std::strcmp("c10",map_header.name) == 0 ||
            std::strcmp("c20",map_header.name) == 0 ||
            std::strcmp("c40",map_header.name) == 0 ||
            std::strcmp("d20",map_header.name) == 0 ||
            std::strcmp("d40",map_header.name) == 0
        ) {
                return;
        }

        // Also don't apply to stock multiplayer I guess.
        if(std::strcmp("beavercreek",map_header.name) == 0 ||
            std::strcmp("bloodgulch",map_header.name) == 0 ||
            std::strcmp("boardingaction",map_header.name) == 0 ||
            std::strcmp("carousel",map_header.name) == 0 ||
            std::strcmp("chillout",map_header.name) == 0 ||
            std::strcmp("damnation",map_header.name) == 0 ||
            std::strcmp("dangercanyon",map_header.name) == 0 ||
            std::strcmp("deathisland",map_header.name) == 0 ||
            std::strcmp("gephyrophobia",map_header.name) == 0 ||
            std::strcmp("hangemhigh",map_header.name) == 0 ||
            std::strcmp("icefields",map_header.name) == 0 ||
            std::strcmp("infinity",map_header.name) == 0 ||
            std::strcmp("longest",map_header.name) == 0 ||
            std::strcmp("prisoner",map_header.name) == 0 ||
            std::strcmp("putput",map_header.name) == 0 ||
            std::strcmp("ratrace",map_header.name) == 0 ||
            std::strcmp("sidewinder",map_header.name) == 0 ||
            std::strcmp("timberland",map_header.name) == 0
        ) {
                return;
        }

        for(std::uint32_t i = 0; i < blacklist_map_count; i++) {
            if(strncmp(map_header.name, chimera_fix_blacklist[i].map_name, 32) != 0) {
                continue;
            }

            auto &tag_header = get_tag_data_header();
            if(chimera_fix_blacklist[i].tags_checksum != tag_header.checksum) {
                continue;
            }

            // Map checks out, apply the memes.
            if(chimera_fix_blacklist[i].config.gearbox_chicago_multiply) {
                global_fix_flags.gearbox_chicago_multiply = true;
            }
            if(chimera_fix_blacklist[i].config.gearbox_meters) {
                global_fix_flags.gearbox_meters = true;
            }
            if(chimera_fix_blacklist[i].config.gearbox_multitexture_blend_modes) {
                global_fix_flags.gearbox_multitexture_blend_modes = true;
            }
            if(chimera_fix_blacklist[i].config.alternate_bump_attenuation) {
                global_fix_flags.alternate_bump_attenuation = true;
            }
            if(chimera_fix_blacklist[i].config.gearbox_bump_attenuation) {
                global_fix_flags.gearbox_bump_attenuation = true;
            }
            if(chimera_fix_blacklist[i].config.invert_detail_after_reflection) {
                global_fix_flags.invert_detail_after_reflection = true;
            }
            if(chimera_fix_blacklist[i].config.embedded_lua) {
                global_fix_flags.embedded_lua = true;
            }
            if(chimera_fix_blacklist[i].config.refined_number_scale) {
                global_fix_flags.refined_number_scale = true;
            }
            break;
        }
    }

    void set_up_map_config_control() noexcept {
        // Init flags struct.
        global_fix_flags.gearbox_chicago_multiply = false;
        global_fix_flags.gearbox_meters = false;
        global_fix_flags.gearbox_multitexture_blend_modes = false;
        global_fix_flags.alternate_bump_attenuation = false;
        global_fix_flags.gearbox_bump_attenuation = false;
        global_fix_flags.invert_detail_after_reflection = false;
        global_fix_flags.embedded_lua = false;
        global_fix_flags.refined_number_scale = false;

        // This is really only applicable to custom edition
        if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            add_map_load_event(set_fix_config_for_new_map, EventPriority::EVENT_PRIORITY_BEFORE);
        }
    }
}
