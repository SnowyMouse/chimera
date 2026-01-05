// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../fix/map_hacks/map_hacks.hpp"
#include "../../../halo_data/tag.hpp"
#include "../../../halo_data/bitmaps.hpp"
#include "../../../halo_data/shader_defs.hpp"

namespace Chimera {
    bool map_config_alternate_bump_attenuation(int, const char **) {
        global_fix_flags.alternate_bump_attenuation = global_fix_flags.alternate_bump_attenuation ? false : true;
        console_output(BOOL_TO_STR(global_fix_flags.alternate_bump_attenuation));
        return true;
    }

    bool map_config_gearbox_meters(int, const char **) {
        global_fix_flags.gearbox_meters = global_fix_flags.gearbox_meters ? false : true;
        console_output(BOOL_TO_STR(global_fix_flags.gearbox_meters));
        return true;
    }

    bool map_config_gearbox_multitexture(int, const char **) {
        global_fix_flags.gearbox_multitexture_blend_modes = global_fix_flags.gearbox_multitexture_blend_modes ? false : true;
        console_output(BOOL_TO_STR(global_fix_flags.gearbox_multitexture_blend_modes));
        return true;
    }

    bool map_config_gearbox_bump_attenuation(int, const char **) {
        global_fix_flags.gearbox_bump_attenuation = global_fix_flags.gearbox_bump_attenuation ? false : true;
        console_output(BOOL_TO_STR(global_fix_flags.gearbox_bump_attenuation));
        return true;
    }

    bool map_config_gearbox_chicago_multiply(int, const char **) {
        global_fix_flags.gearbox_chicago_multiply = global_fix_flags.gearbox_chicago_multiply ? false : true;
        console_output(BOOL_TO_STR(global_fix_flags.gearbox_chicago_multiply));
        return true;
    }

    bool map_config_detail_after_reflection(int, const char **) {
        global_fix_flags.invert_detail_after_reflection = global_fix_flags.invert_detail_after_reflection ? false : true;
        auto &tag_data_header = get_tag_data_header();
        for(std::uint32_t i = 0; i < tag_data_header.tag_count; i++) {
            auto &tag = tag_data_header.tag_array[i];
            if(tag.data && tag.primary_class == TAG_CLASS_SHADER_MODEL) {
                auto tag_data = reinterpret_cast<ShaderModel *>(tag.data);

                // Flip detail after reflection flag.
                SWAP_FLAG(tag_data->model.flags, SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT);
            }
        }

        console_output(BOOL_TO_STR(global_fix_flags.invert_detail_after_reflection));
        return true;
    }

    bool map_config_bitmap_hud_scale_flags(int, const char **) {
        if(!global_fix_flags.disable_bitmap_hud_scale_flags) {
            global_fix_flags.disable_bitmap_hud_scale_flags = true;
            auto &tag_data_header = get_tag_data_header();
            for(std::uint32_t i = 0; i < tag_data_header.tag_count; i++) {
                auto &tag = tag_data_header.tag_array[i];
                if(tag.data && tag.primary_class == TAG_CLASS_BITMAP && !tag.externally_loaded) {
                    auto tag_data = reinterpret_cast<Bitmap *>(tag.data);

                    // Clear HUD scale flags.
                    SET_FLAG(tag_data->flags, BITMAP_FLAGS_HALF_HUD_SCALE_BIT, false);
                    SET_FLAG(tag_data->flags, BITMAP_FLAGS_FORCE_HUD_USE_HIGHRES_SCALE_BIT, false);
                }
            }
        }

        console_output("true");
        return true;
    }

}
