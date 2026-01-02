// SPDX-License-Identifier: GPL-3.0-only

#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../fix/map_hacks.hpp"
#include "../../../halo_data/tag.hpp"
#include "../../../halo_data/shader_defs.hpp"

namespace Chimera {
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
            if(!tag.data) {
                continue;
            }

            auto tag_data = reinterpret_cast<ShaderModel *>(tag.data);
            if(tag.primary_class == TAG_CLASS_SHADER_MODEL) {
                // Flip detail after reflection flag.
                if(TEST_FLAG(tag_data->model.flags, SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT)) {
                    SET_FLAG(tag_data->model.flags, SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT, 0);
                }
                else {
                    SET_FLAG(tag_data->model.flags, SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT, 1);
                }
            }
        }
        console_output(BOOL_TO_STR(global_fix_flags.invert_detail_after_reflection));
        return true;
    }
}
