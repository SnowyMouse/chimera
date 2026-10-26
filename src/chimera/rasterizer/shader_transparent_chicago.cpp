// SPDX-License-Identifier: GPL-3.0-only


#include "rasterizer_vertex_shaders.hpp"
#include "rasterizer_transparent_geometry.hpp"

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/game_functions.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/tag.hpp"
#include "../event/game_loop.hpp"
#include "../event/map_load.hpp"
#include "../fix/af.hpp"
#include "../fix/chicago_fix.hpp"
#include "../fix/map_hacks/map_hacks.hpp"


namespace Chimera {

    // I heckin love register combiners...
    struct StageColorSettings {
        float a_or_b_a;
        float a_or_b_b;
        float rgb_or_alpha_b;
        float use_1_b;
        float a_or_b_c;
        float use_1_c;
        float a_or_b_d;
        float rgb_or_alpha_d;
        float d_scale;
        float use_1_d;
        float ab_scale;
        float cd_scale;
    };

    struct StageAlphaSettings
    {
        float a_or_b_a;
        float a_or_b_b;
        float use_1_b;
        float a_or_b_c;
        float use_1_c;
        float a_or_b_d;
        float d_scale;
        float use_1_d;
        float ab_scale;
        float cd_scale;
    };

    StageColorSettings stage_color_settings[NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTIONS] = {
        {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT
        {1.0F, 0.0F, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_NEXT_MAP
        {1.0F, 0.0F, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_MULTIPLY
        {1.0F, 0.0F, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_DOUBLE_MULTIPLY
        {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD
        {1.0F, 0.0F, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, -1.0f}, //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD_SIGNED_CURRENT
        {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 2.0f, 0.0f, 1.0f, -1.0f}, //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD_SIGNED_NEXT_MAP
        {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f}, //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_SUBTRACT_CURRENT
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f}, //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_SUBTRACT_NEXT_MAP
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_CURRENT
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_CURRENT_INVERSE
        {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_NEXT_MAP
        {0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_NEXT_MAP_INVERSE
    };

    StageAlphaSettings stage_alpha_settings[NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTIONS] = {
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT
        {1.0F, 0.0F, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_NEXT_MAP
        {1.0F, 0.0F, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_MULTIPLY
        {1.0F, 0.0F, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_DOUBLE_MULTIPLY
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD
        {1.0F, 0.0F, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD_SIGNED_CURRENT
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ADD_SIGNED_NEXT_MAP
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_SUBTRACT_CURRENT
        {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f},  //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_SUBTRACT_NEXT_MAP
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_CURRENT
        {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_CURRENT_INVERSE
        {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_NEXT_MAP
        {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},   //SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_ALPHA_BLEND_NEXT_MAP_INVERSE
    };


    void shader_transparent_chicago_create(ShaderTransparentChicago *shader_transparent_chicago) noexcept {
        // maps
        if (shader_transparent_chicago->chicago.maps.count>0) {
            float func_config[72];

            float* stage_color[3];
            float* stage_alpha[3];

            stage_color[0] = &func_config[0];
            stage_alpha[0] = &func_config[12];
            stage_color[1] = &func_config[24];
            stage_alpha[1] = &func_config[36];
            stage_color[2] = &func_config[48];
            stage_alpha[2] = &func_config[60];

            // Default to SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT
            memcpy(stage_color[0], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[0], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));
            memcpy(stage_color[1], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[1], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));
            memcpy(stage_color[2], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[2], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));

            float alpha_replicate[4] ={0.0f, 0.0f, 0.0f, 0.0f};
            float fog_config[4];

            // Set the first stage
            for (std::uint32_t map_index= 0; map_index < shader_transparent_chicago->chicago.maps.count; map_index++)
            {
                ShaderTransparentChicagoMap *map= shader_transparent_chicago_get_map(shader_transparent_chicago, map_index);

                if (map_index != shader_transparent_chicago->chicago.maps.count-1)
                {
                    memcpy(stage_color[map_index], &stage_color_settings[map->color_function], sizeof(StageColorSettings));
                    memcpy(stage_alpha[map_index], &stage_alpha_settings[map->alpha_function], sizeof(StageAlphaSettings));
                    alpha_replicate[map_index] = TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_ALPHA_REPLICATE) ? 1.0f : 0.0f;
                }
            }

            alpha_replicate[3] = (TEST_FLAG(shader_transparent_chicago->chicago.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING)
                                    || global_fix_flags.gearbox_chicago_multiply) ? 1.0f : 0.0f;

            long framebuffer_blend_func = 0;
            switch(shader_transparent_chicago->chicago.framebuffer_blend_function)
            {
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND:
                    framebuffer_blend_func = 0;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MIN:
                    framebuffer_blend_func = 1;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY:
                    framebuffer_blend_func = 2;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ADD:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_REVERSE_SUBTRACT:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MAX:
                    framebuffer_blend_func = 3;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD:
                    framebuffer_blend_func = 4;
                    break;
                default:
                    // Nice tags dingus.
                    break;
            }

            fog_config[0] = static_cast<float>(shader_transparent_chicago->chicago.framebuffer_fade_mode);
            fog_config[1] = static_cast<float>(framebuffer_blend_func);
            fog_config[2] = 0.0f;
            fog_config[3] = 0.0f;

            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 1, func_config, 18);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 19, fog_config, 1);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 20, alpha_replicate, 1);
        }
    }

    void shader_transparent_chicago_extended_create(ShaderTransparentChicagoExtended *shader_transparent_chicago) noexcept {
        // maps
        if (shader_transparent_chicago->chicago_extended.maps.count>0) {
            float func_config[72];

            float* stage_color[3];
            float* stage_alpha[3];

            stage_color[0] = &func_config[0];
            stage_alpha[0] = &func_config[12];
            stage_color[1] = &func_config[24];
            stage_alpha[1] = &func_config[36];
            stage_color[2] = &func_config[48];
            stage_alpha[2] = &func_config[60];

            // Default to SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT
            memcpy(stage_color[0], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[0], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));
            memcpy(stage_color[1], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[1], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));
            memcpy(stage_color[2], &stage_color_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageColorSettings));
            memcpy(stage_alpha[2], &stage_alpha_settings[SHADER_TRANSPARENT_CHICAGO_MAP_FUNCTION_CURRENT], sizeof(StageAlphaSettings));

            float alpha_replicate[4] ={0.0f, 0.0f, 0.0f, 0.0f};
            float fog_config[4];

            // Set the first stage
            for (std::uint32_t map_index= 0; map_index < shader_transparent_chicago->chicago_extended.maps.count; map_index++)
            {
                ShaderTransparentChicagoMap *map= shader_transparent_chicago_extended_get_map(shader_transparent_chicago, map_index);

                if (map_index != shader_transparent_chicago->chicago_extended.maps.count-1)
                {
                    memcpy(stage_color[map_index], &stage_color_settings[map->color_function], sizeof(StageColorSettings));
                    memcpy(stage_alpha[map_index], &stage_alpha_settings[map->alpha_function], sizeof(StageAlphaSettings));
                    alpha_replicate[map_index] = TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_ALPHA_REPLICATE) ? 1.0f : 0.0f;
                }
            }

            alpha_replicate[3] = (TEST_FLAG(shader_transparent_chicago->chicago_extended.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING)
                                    || global_fix_flags.gearbox_chicago_multiply) ? 1.0f : 0.0f;

            long framebuffer_blend_func = 0;
            switch(shader_transparent_chicago->chicago_extended.framebuffer_blend_function)
            {
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND:
                    framebuffer_blend_func = 0;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MIN:
                    framebuffer_blend_func = 1;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY:
                    framebuffer_blend_func = 2;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ADD:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_REVERSE_SUBTRACT:
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MAX:
                    framebuffer_blend_func = 3;
                    break;
                case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD:
                    framebuffer_blend_func = 4;
                    break;
                default:
                    // Nice tags dingus.
                    break;
            }

            fog_config[0] = static_cast<float>(shader_transparent_chicago->chicago_extended.framebuffer_fade_mode);
            fog_config[1] = static_cast<float>(framebuffer_blend_func);
            fog_config[2] = 0.0f;
            fog_config[3] = 0.0f;

            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 1, func_config, 18);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 19, fog_config, 1);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 20, alpha_replicate, 1);
        }
    }

    extern "C" bool rasterizer_shader_transparent_chicago_draw(TransparentGeometryGroup *group, bool is_dirty) noexcept {
        // We can't draw sm3 chicago if the d3d9 device doesn't support it
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0300 || d3d9_device_caps->VertexShaderVersion < 0xfffe0300) {
            return false;
        }

        ShaderTransparentChicago *shader_data = shader_transparent_chicago_get(group->shader);
        std::uint16_t bitmap_index = group->shader_permutation_index;

        for(std::uint16_t layer_index = 0; layer_index < shader_data->chicago.extra_layers.count; layer_index++) {
            TransparentGeometryGroup group_layer;
            memcpy(&group_layer, group, sizeof(TransparentGeometryGroup));
            group_layer.sorted_index = -1;
            group_layer.shader = get_tag(shader_transparent_chicago_get_layer(shader_data, layer_index)->tag_id)->data;

            rasterizer_transparent_geometry_group_draw(&group_layer, is_dirty);
        }

        short vertex_shader_permutation = shader_get_vertex_shader_permutation(reinterpret_cast<std::byte *>(group->shader));
        short vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);
        IDirect3DVertexShader9 *vertex_shader = rasterizer_get_vertex_shader_for_permutation(vertex_shader_permutation, vertex_type);
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, vertex_shader);
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(vertex_type));
        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, NULL);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

        rasterizer_set_render_state(D3DRS_CULLMODE, TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_TWO_SIDED_BIT) ? D3DCULL_NONE : D3DCULL_CCW);
        rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
        rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
        rasterizer_set_render_state(D3DRS_ALPHATESTENABLE, TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_ALPHA_TESTED_BIT));
        rasterizer_set_render_state(D3DRS_ALPHAREF, 0x0000007F);

        rasterizer_set_framebuffer_blend_function(shader_data->chicago.framebuffer_blend_function);

        // Numeric bitmap indexing
        if(TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_NUMERIC_BIT) && group->animation && shader_data->chicago.maps.count > 0) {
            short base = get_bitmap_tag(shader_transparent_chicago_get_map(shader_data, 0)->map.tag_id)->bitmap_data.count;
            if(TEST_FLAG(shader_data->chicago.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_NUMERIC_COUNTDOWN_TIMER)) {
                bitmap_index = numeric_countdown_timer_get(group->shader_permutation_index);
            }
            else {
                short numeric_counter_source_index = (base == 8) ? 3 : 0;

                short numeric_counter_limit = shader_data->chicago.numeric_counter_limit;
                short numeric_counter = PIN(fast_ftol_floor(0.5f + group->animation->values[numeric_counter_source_index] * static_cast<float>(numeric_counter_limit)), 0, numeric_counter_limit);
                for(short i = 0; i < group->shader_permutation_index; i++) {
                    numeric_counter /= base;
                }

                bitmap_index = numeric_counter % base;
            }

        }

        // Maps
        if(shader_data->chicago.maps.count > 0) {
            float vsh_constants_texanim[4 * 8] = {0};

            for(std::uint16_t map_index = 0; map_index < 4; map_index++) {
                if(map_index < shader_data->chicago.maps.count) {
                    ShaderTransparentChicagoMap *map = shader_transparent_chicago_get_map(shader_data, map_index);
                    ShaderTransparentChicagoType type = shader_data->chicago.type;
                    BitmapDataType bitmap_type;
                    bool is_first_map = map_index == 0;

                    const BitmapDataType bitmap_type_array[4] = {BITMAP_DATA_TYPE_2D, BITMAP_DATA_TYPE_CUBE_MAP, BITMAP_DATA_TYPE_CUBE_MAP, BITMAP_DATA_TYPE_CUBE_MAP};
                    bitmap_type = is_first_map ? bitmap_type_array[type] : static_cast<BitmapDataType>(BITMAP_DATA_TYPE_2D);

                    DWORD u_address, v_address, w_address;

                    // Set texture
                    rasterizer_set_texture(map_index, bitmap_type, BITMAP_USAGE_ADDITIVE, bitmap_index, map->map.tag_id);

                    const DWORD bitmap_address_array[4] = {D3DTADDRESS_WRAP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP};
                    if(bitmap_type == BITMAP_DATA_TYPE_2D && TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_U_CLAMPED_BIT)) {
                        u_address = D3DTADDRESS_CLAMP;
                    }
                    else {
                        u_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);
                    }
                    if(bitmap_type == BITMAP_DATA_TYPE_2D && TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_V_CLAMPED_BIT)) {
                        v_address = D3DTADDRESS_CLAMP;
                    }
                    else {
                        v_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);
                    }
                    w_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);

                    DWORD filter_type = D3DTEXF_LINEAR;
                    if(af_is_enabled) {
                        if(*af_is_enabled && (d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                            rasterizer_set_sampler_state(map_index, D3DSAMP_MAXANISOTROPY, d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy);
                            filter_type = D3DTEXF_ANISOTROPIC;
                        }
                    }

                    bool point_sampled = TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_POINT_SAMPLED_BIT);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSU, u_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSV, v_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSW, w_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MAGFILTER, filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MINFILTER, point_sampled ? static_cast<DWORD>(D3DTEXF_POINT) : filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MIPFILTER, point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
                }
                else {
                    IDirect3DDevice9_SetTexture(*global_d3d9_device, map_index, NULL);
                }

                if(map_index < shader_data->chicago.maps.count && (map_index > 0 || shader_data->chicago.type == SHADER_TRANSPARENT_CHICAGO_TYPE_2D_MAP)) {
                    ShaderTransparentChicagoMap *map = shader_transparent_chicago_get_map(shader_data, map_index);
                    Point2D map_scale = map->scale;

                    if(map_index == 0 && TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_SCALE_FIRST_MAP_WITH_DISTANCE_BIT)) {
                        map_scale.x *= -group->z_sort;
                        map_scale.y *= -group->z_sort;
                    }

                    if(map_index > 0 || !TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_FIRST_MAP_IS_IN_SCREENSPACE_BIT)) {
                        map_scale.x *= group->model_base_map_scale.x;
                        map_scale.y *= group->model_base_map_scale.y;
                    }

                    ShaderTextureAnimation *texture_animation = &map->animation;
                    float map_u_offset = map->offset.x;
                    float map_v_offset = map->offset.y;
                    float map_rotation = map->rotation;

                    shader_texture_animation_evaluate(map_scale.x, map_scale.y, map_u_offset, map_v_offset, map_rotation, global_frame_parameters->elapsed_time_sec,
                                                        texture_animation, group->animation, &vsh_constants_texanim[map_index * 8],
                                                        &vsh_constants_texanim[map_index * 8 + 4]);
                }
                else if(map_index < shader_data->chicago.maps.count && TEST_FLAG(shader_data->chicago.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_FIRST_MAP_IS_IN_SCREENSPACE_BIT)) {
                    vsh_constants_texanim[map_index * 8 + 0] = global_window_parameters->frustum.view_to_world.forward.i;
                    vsh_constants_texanim[map_index * 8 + 1] = global_window_parameters->frustum.view_to_world.forward.j;
                    vsh_constants_texanim[map_index * 8 + 2] = global_window_parameters->frustum.view_to_world.forward.k;
                    vsh_constants_texanim[map_index * 8 + 3] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 4] = global_window_parameters->frustum.view_to_world.left.i;
                    vsh_constants_texanim[map_index * 8 + 5] = global_window_parameters->frustum.view_to_world.left.j;
                    vsh_constants_texanim[map_index * 8 + 6] = global_window_parameters->frustum.view_to_world.left.k;
                    vsh_constants_texanim[map_index * 8 + 7] = 0.0f;
                }
                else {
                    vsh_constants_texanim[map_index * 8 + 0] = 1.0f;
                    vsh_constants_texanim[map_index * 8 + 1] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 2] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 3] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 4] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 5] = 1.0f;
                    vsh_constants_texanim[map_index * 8 + 6] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 7] = 0.0f;
                }
            }
            IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, vsh_constants_texanim, 8);
        }
        else {
            // Just draw with no pixel shader to handle bullshit custom tags with no maps...
            rasterizer_transparent_geometry_group_draw_vertices(group, FALSE);
            IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);

            return true;
        }

        shader_transparent_chicago_create(shader_data);

        if(rasterizer_debug_options->draw_environment_fog) {
            if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_SKY_BIT) && shader_data->chicago.framebuffer_blend_function == SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND) {
                Plane3D *plane = &global_window_parameters->fog.plane;
                Point3D *camera = &global_window_parameters->camera.position;
                float eye_distance_to_fog_plane = plane->i * camera->x + plane->j * camera->y + plane->k * camera->z - plane->w;
                float planar_eye_density = global_window_parameters->fog.planar_maximum_density * PIN(-eye_distance_to_fog_plane / global_window_parameters->fog.planar_maximum_depth, 0.0f, 1.0f);

                float constant_color[4] = {
                    global_window_parameters->fog.planar_color.red,
                    global_window_parameters->fog.planar_color.green,
                    global_window_parameters->fog.planar_color.blue,
                    planar_eye_density,
                };

                // Tell the pixel shader to use fog plane color instead.
                float framebuffer_fade_mode[4] = {4.0f, 0.0f, 0.0f, 0.0f};

                IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, constant_color, 1);
                IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 19, framebuffer_fade_mode, 1);

            }
            else {
                float vsh_constants_texscale[3 * 4] = {0};
                vsh_constants_texscale[10] = 1.0f;

                if(group->effect.type == RENDER_MODEL_EFFECT_TYPE_ACTIVE_CAMOUFLAGE && !TEST_FLAG(shader_data->chicago.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_DONT_FADE_ACTIVE_CAMOUFLAGE)) {
                    vsh_constants_texscale[10] *= PIN(1.0f - group->effect.intensity, 0.0f, 1.0f);
                }

                if(shader_data->chicago.framebuffer_fade_source > 0 && group->animation != NULL && group->animation->values != NULL) {
                    vsh_constants_texscale[10] *= group->animation->values[shader_data->chicago.framebuffer_fade_source - 1];
                }

                IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 10, vsh_constants_texscale, 3);
            }
        }


        IDirect3DPixelShader9 *shader = (shader_data->chicago.type == SHADER_TRANSPARENT_CHICAGO_TYPE_2D_MAP) ? 
                                        chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_TRANSPARENT_CHICAGO] : 
                                        chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_TRANSPARENT_CHICAGO_CUBE];

        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, shader);
        rasterizer_transparent_geometry_group_draw_vertices(group, FALSE);
        IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);

        return true;
    }

    extern "C" bool rasterizer_shader_transparent_chicago_extended_draw(TransparentGeometryGroup *group, bool is_dirty) noexcept {
        // We can't draw sm3 chicago if the d3d9 device doesn't support it
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0300 || d3d9_device_caps->VertexShaderVersion < 0xfffe0300) {
            return false;
        }

        ShaderTransparentChicagoExtended *shader_data = shader_transparent_chicago_extended_get(group->shader);
        std::uint16_t bitmap_index = group->shader_permutation_index;

        for(std::uint16_t layer_index = 0; layer_index < shader_data->chicago_extended.extra_layers.count; layer_index++) {
            TransparentGeometryGroup group_layer;
            memcpy(&group_layer, group, sizeof(TransparentGeometryGroup));
            group_layer.sorted_index = -1;
            group_layer.shader = get_tag(shader_transparent_chicago_extended_get_layer(shader_data, layer_index)->tag_id)->data;

            rasterizer_transparent_geometry_group_draw(&group_layer, is_dirty);
        }

        short vertex_shader_permutation = shader_get_vertex_shader_permutation(reinterpret_cast<std::byte *>(group->shader));
        short vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);
        IDirect3DVertexShader9 *vertex_shader = rasterizer_get_vertex_shader_for_permutation(vertex_shader_permutation, vertex_type);
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, vertex_shader);
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(vertex_type));
        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, NULL);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

        rasterizer_set_render_state(D3DRS_CULLMODE, TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_TWO_SIDED_BIT) ? D3DCULL_NONE : D3DCULL_CCW);
        rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
        rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
        rasterizer_set_render_state(D3DRS_ALPHATESTENABLE, TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_ALPHA_TESTED_BIT));
        rasterizer_set_render_state(D3DRS_ALPHAREF, 0x0000007F);

        rasterizer_set_framebuffer_blend_function(shader_data->chicago_extended.framebuffer_blend_function);

        // Numeric bitmap indexing
        if(TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_NUMERIC_BIT) && group->animation && shader_data->chicago_extended.maps.count > 0) {
            short base = get_bitmap_tag(shader_transparent_chicago_extended_get_map(shader_data, 0)->map.tag_id)->bitmap_data.count;
            if(TEST_FLAG(shader_data->chicago_extended.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_NUMERIC_COUNTDOWN_TIMER)) {
                bitmap_index = numeric_countdown_timer_get(group->shader_permutation_index);
            }
            else {
                short numeric_counter_source_index = (base == 8) ? 3 : 0;

                short numeric_counter_limit = shader_data->chicago_extended.numeric_counter_limit;
                short numeric_counter = PIN(fast_ftol_floor(0.5f + group->animation->values[numeric_counter_source_index] * static_cast<float>(numeric_counter_limit)), 0, numeric_counter_limit);
                for(short i = 0; i < group->shader_permutation_index; i++) {
                    numeric_counter /= base;
                }

                bitmap_index = numeric_counter % base;
            }

        }

        // Maps
        if(shader_data->chicago_extended.maps.count > 0) {
            float vsh_constants_texanim[4 * 8] = {0};

            for(std::uint16_t map_index = 0; map_index < 4; map_index++) {
                if(map_index < shader_data->chicago_extended.maps.count) {
                    ShaderTransparentChicagoMap *map = shader_transparent_chicago_extended_get_map(shader_data, map_index);
                    ShaderTransparentChicagoType type = shader_data->chicago_extended.type;
                    BitmapDataType bitmap_type;
                    bool is_first_map = map_index == 0;

                    const BitmapDataType bitmap_type_array[4] = {BITMAP_DATA_TYPE_2D, BITMAP_DATA_TYPE_CUBE_MAP, BITMAP_DATA_TYPE_CUBE_MAP, BITMAP_DATA_TYPE_CUBE_MAP};
                    bitmap_type = is_first_map ? bitmap_type_array[type] : static_cast<BitmapDataType>(BITMAP_DATA_TYPE_2D);

                    DWORD u_address, v_address, w_address;

                    // Set texture
                    rasterizer_set_texture(map_index, bitmap_type, BITMAP_USAGE_ADDITIVE, bitmap_index, map->map.tag_id);

                    const DWORD bitmap_address_array[4] = {D3DTADDRESS_WRAP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP};
                    if(bitmap_type == BITMAP_DATA_TYPE_2D && TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_U_CLAMPED_BIT)) {
                        u_address = D3DTADDRESS_CLAMP;
                    }
                    else {
                        u_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);
                    }
                    if(bitmap_type == BITMAP_DATA_TYPE_2D && TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_V_CLAMPED_BIT)) {
                        v_address = D3DTADDRESS_CLAMP;
                    }
                    else {
                        v_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);
                    }
                    w_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);

                    DWORD filter_type = D3DTEXF_LINEAR;
                    if(af_is_enabled) {
                        if(*af_is_enabled && (d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                            rasterizer_set_sampler_state(map_index, D3DSAMP_MAXANISOTROPY, d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy);
                            filter_type = D3DTEXF_ANISOTROPIC;
                        }
                    }

                    bool point_sampled = TEST_FLAG(map->flags, SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_POINT_SAMPLED_BIT);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSU, u_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSV, v_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSW, w_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MAGFILTER, filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MINFILTER, point_sampled ? static_cast<DWORD>(D3DTEXF_POINT) : filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MIPFILTER, point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
                }
                else {
                    IDirect3DDevice9_SetTexture(*global_d3d9_device, map_index, NULL);
                }

                if(map_index < shader_data->chicago_extended.maps.count && (map_index > 0 || shader_data->chicago_extended.type == SHADER_TRANSPARENT_CHICAGO_TYPE_2D_MAP)) {
                    ShaderTransparentChicagoMap *map = shader_transparent_chicago_extended_get_map(shader_data, map_index);
                    Point2D map_scale = map->scale;

                    if(map_index == 0 && TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_SCALE_FIRST_MAP_WITH_DISTANCE_BIT)) {
                        map_scale.x *= -group->z_sort;
                        map_scale.y *= -group->z_sort;
                    }

                    if(map_index > 0 || !TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_FIRST_MAP_IS_IN_SCREENSPACE_BIT)) {
                        map_scale.x *= group->model_base_map_scale.x;
                        map_scale.y *= group->model_base_map_scale.y;
                    }

                    ShaderTextureAnimation *texture_animation = &map->animation;
                    float map_u_offset = map->offset.x;
                    float map_v_offset = map->offset.y;
                    float map_rotation = map->rotation;

                    shader_texture_animation_evaluate(map_scale.x, map_scale.y, map_u_offset, map_v_offset, map_rotation, global_frame_parameters->elapsed_time_sec,
                                                        texture_animation, group->animation, &vsh_constants_texanim[map_index * 8],
                                                        &vsh_constants_texanim[map_index * 8 + 4]);
                }
                else if(map_index < shader_data->chicago_extended.maps.count && TEST_FLAG(shader_data->chicago_extended.flags, SHADER_TRANSPARENT_CHICAGO_FLAGS_FIRST_MAP_IS_IN_SCREENSPACE_BIT)) {
                    vsh_constants_texanim[map_index * 8 + 0] = global_window_parameters->frustum.view_to_world.forward.i;
                    vsh_constants_texanim[map_index * 8 + 1] = global_window_parameters->frustum.view_to_world.forward.j;
                    vsh_constants_texanim[map_index * 8 + 2] = global_window_parameters->frustum.view_to_world.forward.k;
                    vsh_constants_texanim[map_index * 8 + 3] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 4] = global_window_parameters->frustum.view_to_world.left.i;
                    vsh_constants_texanim[map_index * 8 + 5] = global_window_parameters->frustum.view_to_world.left.j;
                    vsh_constants_texanim[map_index * 8 + 6] = global_window_parameters->frustum.view_to_world.left.k;
                    vsh_constants_texanim[map_index * 8 + 7] = 0.0f;
                }
                else {
                    vsh_constants_texanim[map_index * 8 + 0] = 1.0f;
                    vsh_constants_texanim[map_index * 8 + 1] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 2] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 3] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 4] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 5] = 1.0f;
                    vsh_constants_texanim[map_index * 8 + 6] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 7] = 0.0f;
                }
            }
            IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, vsh_constants_texanim, 8);
        }

        else {
            // Just draw with no pixel shader to handle bullshit custom tags with no maps...
            rasterizer_transparent_geometry_group_draw_vertices(group, FALSE);
            IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);

            return true;
        }

        shader_transparent_chicago_extended_create(shader_data);

        if(rasterizer_debug_options->draw_environment_fog) {
            if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_SKY_BIT) && shader_data->chicago_extended.framebuffer_blend_function == SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND) {
                Plane3D *plane = &global_window_parameters->fog.plane;
                Point3D *camera = &global_window_parameters->camera.position;
                float eye_distance_to_fog_plane = plane->i * camera->x + plane->j * camera->y + plane->k * camera->z - plane->w;
                float planar_eye_density = global_window_parameters->fog.planar_maximum_density * PIN(-eye_distance_to_fog_plane / global_window_parameters->fog.planar_maximum_depth, 0.0f, 1.0f);

                float constant_color[4] = {
                    global_window_parameters->fog.planar_color.red,
                    global_window_parameters->fog.planar_color.green,
                    global_window_parameters->fog.planar_color.blue,
                    planar_eye_density,
                };

                // Tell the pixel shader to use fog plane color instead.
                float framebuffer_fade_mode[4] = {4.0f, 0.0f, 0.0f, 0.0f};

                IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, constant_color, 1);
                IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 19, framebuffer_fade_mode, 1);

            }
            else {
                float vsh_constants_texscale[3 * 4] = {0};
                vsh_constants_texscale[10] = 1.0f;

                if(group->effect.type == RENDER_MODEL_EFFECT_TYPE_ACTIVE_CAMOUFLAGE && !TEST_FLAG(shader_data->chicago_extended.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_DONT_FADE_ACTIVE_CAMOUFLAGE)) {
                    vsh_constants_texscale[10] *= PIN(1.0f - group->effect.intensity, 0.0f, 1.0f);
                }

                if(shader_data->chicago_extended.framebuffer_fade_source > 0 && group->animation != NULL && group->animation->values != NULL) {
                    vsh_constants_texscale[10] *= group->animation->values[shader_data->chicago_extended.framebuffer_fade_source - 1];
                }

                IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 10, vsh_constants_texscale, 3);
            }
        }


        IDirect3DPixelShader9 *shader = (shader_data->chicago_extended.type == SHADER_TRANSPARENT_CHICAGO_TYPE_2D_MAP) ? 
                                        chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_TRANSPARENT_CHICAGO] : 
                                        chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_TRANSPARENT_CHICAGO_CUBE];

        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, shader);
        rasterizer_transparent_geometry_group_draw_vertices(group, FALSE);
        IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);

        return true;
    }

}
