// SPDX-License-Identifier: GPL-3.0-only

#include "map_hacks.hpp"
#include "../../chimera.hpp"
#include "../../config/ini.hpp"
#include "../../signature/hook.hpp"
#include "../../signature/signature.hpp"
#include "../../halo_data/map.hpp"
#include "../../halo_data/tag.hpp"
#include "../../halo_data/game_engine.hpp"
#include "../../event/map_load.hpp"

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
        global_fix_flags.hud_number_scale = false;
        global_fix_flags.disable_bitmap_hud_scale_flags = false;

        auto &map_header = get_map_header();

        char map_name_lowercase[32] = {};
        std::strncpy(map_name_lowercase, map_header.name, sizeof(map_name_lowercase));
        for(auto &i : map_name_lowercase) {
            i = std::tolower(i, std::locale("C"));
        }

        // Do not apply to stock campaign.
        if( std::strncmp("ui", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("a10", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("a30", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("a50", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("b30", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("b40", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("c10", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("c20", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("c40", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("d20", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("d40", map_name_lowercase, sizeof(map_name_lowercase)) == 0
        ) {
                return;
        }

        // Also don't apply to stock multiplayer I guess.
        if(std::strncmp("beavercreek", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("bloodgulch", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("boardingaction", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("carousel", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("chillout", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("damnation", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("dangercanyon", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("deathisland", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("gephyrophobia", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("hangemhigh", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("icefields", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("infinity", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("longest", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("prisoner", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("putput", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("ratrace", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("sidewinder", map_name_lowercase, sizeof(map_name_lowercase)) == 0 ||
            std::strncmp("timberland", map_name_lowercase, sizeof(map_name_lowercase)) == 0
        ) {
                return;
        }

        for(std::uint32_t i = 0; i < blacklist_map_count; i++) {
            if(strncmp(chimera_fix_blacklist[i].map_name, map_name_lowercase, sizeof(map_name_lowercase)) != 0) {
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
            if(chimera_fix_blacklist[i].config.hud_number_scale) {
                global_fix_flags.hud_number_scale = true;
            }
            if(chimera_fix_blacklist[i].config.disable_bitmap_hud_scale_flags) {
                global_fix_flags.disable_bitmap_hud_scale_flags = true;
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
        global_fix_flags.hud_number_scale = false;
        global_fix_flags.disable_bitmap_hud_scale_flags = false;

        // This is really only applicable to custom edition
        if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            if(get_chimera().get_ini()->get_value_bool("maps.map_compatibility_list").value_or(true)) {
                add_map_load_event(set_fix_config_for_new_map, EventPriority::EVENT_PRIORITY_BEFORE);
            }
        }
    }
}
