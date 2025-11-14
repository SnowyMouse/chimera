// SPDX-License-Identifier: GPL-3.0-only

//#define WRITE_DEFINES_TO_FILE

#ifdef WRITE_DEFINES_TO_FILE
#include <cstring>
#include <filesystem>
#include <fstream>
#endif

#include "shader_transparent_generic.hpp"
#include "rasterizer_vertex_shaders.hpp"
#include "rasterizer_transparent_geometry.hpp"

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/game_functions.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/shaders/shader_blob.hpp"
#include "../halo_data/shaders/shader_transparent_generic_blobs.hpp"
#include "../map_loading/crc32.hpp"
#include "../event/game_loop.hpp"
#include "../event/map_load.hpp"
#include "../fix/af.hpp"

#include "../../blake3/blake3.h"

namespace Chimera {

    extern "C" {
        void shader_transparent_generic_switch_function() noexcept;
        std::byte *generic_switch_return = nullptr;
        IDirect3DPixelShader9 *generic_pixel_shader = nullptr;
    }

#ifdef WRITE_DEFINES_TO_FILE
    std::ofstream shader_file;
#endif

    static GenericTag generic_tag_cache[MAX_GENERIC_TAG_COUNT] = {};
    std::uint32_t generic_tag_cache_index = 0;

    static GenericInstance generic_instance_cache[MAX_GENERIC_INSTANCE_COUNT] = {};
    std::uint32_t generic_instance_index = 0;

    enum {
        NUM_OF_SHADER_COMPILE_DEFINES = 10
    };

    struct ShaderStageParams {
        std::int16_t input_a;
        std::int16_t input_a_mapping;
        std::int16_t input_b;
        std::int16_t input_b_mapping;
        std::int16_t input_c;
        std::int16_t input_c_mapping;
        std::int16_t input_d;
        std::int16_t input_d_mapping;

        std::int16_t color_mux;
        std::int16_t output_ab;
        std::int16_t output_ab_function;
        std::int16_t output_cd;
        std::int16_t output_cd_function;
        std::int16_t output_ab_cd_mux_sum;
        std::int16_t output_mapping_color;

        std::int16_t input_a_alpha;
        std::int16_t input_a_mapping_alpha;
        std::int16_t input_b_alpha;
        std::int16_t input_b_mapping_alpha;
        std::int16_t input_c_alpha;
        std::int16_t input_c_mapping_alpha;
        std::int16_t input_d_alpha;
        std::int16_t input_d_mapping_alpha;

        std::int16_t alpha_mux;
        std::int16_t output_ab_alpha;
        std::int16_t output_cd_alpha;
        std::int16_t output_ab_cd_mux_sum_alpha;
        std::int16_t output_mapping_alpha;

        std::int16_t is_fog_stage;
    };
    static_assert(sizeof(ShaderStageParams) == sizeof(std::int16_t) * 29);

    ShaderTransparentGeneric *shader_transparent_generic_get(std::byte *shader) noexcept {
        auto *shader_test = reinterpret_cast<_shader *>(shader);

        if(shader_test->type == SHADER_TYPE_TRANSPARENT_GENERIC) {
            return reinterpret_cast<ShaderTransparentGeneric *>(shader);
        }
        show_error_box("Error", "Invalid shader type");
        std::exit(1);
    }

    TagReference *shader_transparent_generic_get_layer(ShaderTransparentGeneric *shader_data, std::uint16_t layer) noexcept {
        auto *extra_layer = get_tag_block_data(&shader_data->generic.extra_layers, layer, sizeof(TagReference));
        throw_error(extra_layer, "transparent generic invalid extra layer block data");
        return reinterpret_cast<TagReference *>(extra_layer);
    }

    ShaderTransparentGenericMap *shader_transparent_generic_get_map(ShaderTransparentGeneric *shader_data, std::uint16_t map_index) noexcept {
        auto *map_data = get_tag_block_data(&shader_data->generic.maps, map_index, sizeof(ShaderTransparentGenericMap));
        throw_error(map_data, "transparent generic invalid map block data");
        return reinterpret_cast<ShaderTransparentGenericMap *>(map_data);
    }

    ShaderTransparentGenericStage *shader_transparent_generic_get_stage(ShaderTransparentGeneric *shader_data, std::uint16_t stage_index) noexcept {
        auto *stage_data = get_tag_block_data(&shader_data->generic.stages, stage_index, sizeof(ShaderTransparentGenericStage));
        throw_error(stage_data, "transparent generic invalid stage block data");
        return reinterpret_cast<ShaderTransparentGenericStage *>(stage_data);
    }

    // Stage defines generation based on ringworld https://github.com/MangoFizz/ringworld/blob/master/src/impl/rasterizer/rasterizer_shader_transparent_generic.c#L75
    static D3D_SHADER_MACRO generate_stage_define(size_t stage_index, ShaderStageParams params) noexcept {
        char buffer[180];
        snprintf(buffer, 180, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
            stage_index, 

            params.input_a, params.input_a_mapping, 
            params.input_b, params.input_b_mapping, 
            params.input_c, params.input_c_mapping, 
            params.input_d, params.input_d_mapping, 

            params.color_mux, 
            params.output_ab, params.output_ab_function, 
            params.output_cd, params.output_cd_function, 
            params.output_ab_cd_mux_sum, 
            params.output_mapping_color, 

            params.input_a_alpha, params.input_a_mapping_alpha, 
            params.input_b_alpha, params.input_b_mapping_alpha, 
            params.input_c_alpha, params.input_c_mapping_alpha, 
            params.input_d_alpha, params.input_d_mapping_alpha, 

            params.alpha_mux, 
            params.output_ab_alpha, 
            params.output_cd_alpha, 
            params.output_ab_cd_mux_sum_alpha, 
            params.output_mapping_alpha,

            params.is_fog_stage
        );

        char *macro = reinterpret_cast<char *>(GlobalAlloc(GMEM_FIXED, strlen(buffer) + 1));
        strcpy(macro, buffer);

        snprintf(buffer, 180, "S%d_CONFIGURATION", stage_index);
        char *name = reinterpret_cast<char *>(GlobalAlloc(GMEM_FIXED, strlen(buffer) + 1));
        strcpy(name, buffer);

        D3D_SHADER_MACRO result = {};
        result.Name = name;
        result.Definition = macro;

        return result;
    }

    // Hash the defines.
    char *generate_defines_hash(D3D_SHADER_MACRO *defines) noexcept {
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);

        for(size_t i = 0; i < NUM_OF_SHADER_COMPILE_DEFINES; i++) {
            // Not really any point hashing the name except for FIRST_MAP_IS_CUBE
            if(defines[i].Name != NULL && defines[i].Definition == NULL) {
                std::uint32_t size = strlen(defines[i].Name);
                blake3_hasher_update(&hasher, defines[i].Name, size);
            }
            if(defines[i].Definition != NULL) {
                std::uint32_t size = strlen(defines[i].Definition);
                blake3_hasher_update(&hasher, defines[i].Definition, size);
            }
        }
        std::uint8_t *output = reinterpret_cast<std::uint8_t *>(GlobalAlloc(GMEM_FIXED, BLAKE3_OUT_LEN));
        blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

        return reinterpret_cast<char *>(output);
    }

    static D3D_SHADER_MACRO *generate_defines(ShaderTransparentGeneric *shader_data) noexcept {
        D3D_SHADER_MACRO *defines = reinterpret_cast<D3D_SHADER_MACRO *>(GlobalAlloc(GPTR, sizeof(D3D_SHADER_MACRO) * 10));
        size_t defines_count = 0;
        ShaderStageParams params = {};

        // if there are no stages, set up the default stage
        if(shader_data->generic.stages.count == 0) {
            params.input_a = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_COLOR_0;
            params.input_b = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ONE;
            params.output_ab = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
            params.input_a_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_ALPHA_0;
            params.input_b_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_ONE;
            params.output_ab_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
            params.is_fog_stage = false;
            defines[defines_count] = generate_stage_define(defines_count, params);
            defines_count++;
        }
        else {
            for(size_t current_stage = 0; current_stage < shader_data->generic.stages.count && current_stage < 7; current_stage++) {
                ShaderTransparentGenericStage *stage = shader_transparent_generic_get_stage(shader_data, current_stage);

                params.input_a = stage->color_input_A;
                params.input_a_mapping = stage->color_input_A_mapping;
                params.input_b = stage->color_input_B;
                params.input_b_mapping = stage->color_input_B_mapping;
                params.input_c = stage->color_input_C;
                params.input_c_mapping = stage->color_input_C_mapping;
                params.input_d = stage->color_input_D;
                params.input_d_mapping = stage->color_input_D_mapping;

                params.color_mux = stage->flags.color_mux;
                params.output_ab = stage->color_output_AB;
                params.output_ab_function = stage->color_output_AB_function;
                params.output_cd = stage->color_output_CD;
                params.output_cd_function = stage->color_output_CD_function;
                params.output_ab_cd_mux_sum = stage->color_output_AB_CD;
                params.output_mapping_color = stage->color_output_mapping;

                params.input_a_alpha = stage->alpha_input_A;
                params.input_a_mapping_alpha = stage->alpha_input_A_mapping;
                params.input_b_alpha = stage->alpha_input_B;
                params.input_b_mapping_alpha = stage->alpha_input_B_mapping;
                params.input_c_alpha = stage->alpha_input_C;
                params.input_c_mapping_alpha = stage->alpha_input_C_mapping;
                params.input_d_alpha = stage->alpha_input_D;
                params.input_d_mapping_alpha = stage->alpha_input_D_mapping;

                params.alpha_mux = stage->flags.alpha_mux;
                params.output_ab_alpha = stage->alpha_output_AB;
                params.output_cd_alpha = stage->alpha_output_CD;
                params.output_ab_cd_mux_sum_alpha = stage->alpha_output_AB_CD;
                params.output_mapping_alpha = stage->alpha_output_mapping;

                params.is_fog_stage = false;

                defines[defines_count] = generate_stage_define(current_stage, params);
                defines_count++;

                // Wipe the params for the next iteration
                memset(&params, 0, sizeof(ShaderStageParams));
            }
        }

        FramebufferFadeMode framebuffer_mode = shader_data->generic.framebuffer_fade_mode;
        ShaderTransparentGenericStageColorInputs input_color = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ZERO;
        ShaderTransparentGenericStageAlphaInputs input_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_ZERO;
        switch(framebuffer_mode) {
            case SHADER_FRAMEBUFFER_FADE_MODE_NONE:
                input_color = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_ALPHA_0_FADE_NONE;
                input_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_ALPHA_0_FADE_NONE;
                break;
            case SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR:
                input_color = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_ALPHA_1_FADE_PERPENDICULAR;
                input_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_ALPHA_1_FADE_PERPENDICULAR;
                break;
            case SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL:
                input_color = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_COLOR_1_FADE_PERPENDICULAR;
                input_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_BLUE_1_FADE_PARALLEL;
                break;
            default:
                show_error_box("Error", "Nice tags (invalid framebuffer fade mode)");
                std::exit(1);
                break;
        }
        FramebufferBlendFunction framebuffer_blend_function = shader_data->generic.framebuffer_blend_function;
        switch(framebuffer_blend_function) {
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND:
                params.input_a_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_ALPHA_0;
                params.input_b_alpha = input_alpha;
                params.output_ab_cd_mux_sum_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                break;

            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MIN:
                params.input_a = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_0;
                params.input_b = input_color;
                params.input_c = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ONE;
                params.input_d = input_color;
                params.input_d_mapping = SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_UNSIGNED_INVERT;
                params.output_ab_cd_mux_sum = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                break;

            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY:
                params.input_a = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_0;
                params.input_b = input_color;
                params.input_c = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ONE_HALF;
                params.input_d = input_color;
                params.input_d_mapping = SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_UNSIGNED_INVERT;
                params.output_ab_cd_mux_sum = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                break;

            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ADD:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_REVERSE_SUBTRACT:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MAX:
                params.input_a = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_0;
                params.input_b = input_color;
                params.output_ab_cd_mux_sum = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                break;

            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD:
                params.input_a_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_ALPHA_0;
                params.input_b_alpha = input_alpha;
                params.output_ab_cd_mux_sum_alpha = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                params.input_a = SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_0;
                params.input_b = input_color;
                params.output_ab_cd_mux_sum = SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0;
                break;

            default:
                show_error_box("Error", "Nice tags (invalid framebuffer blend function)");
                std::exit(1);
                break;
        }
        params.is_fog_stage = true;
        defines[defines_count] = generate_stage_define(defines_count, params);
        defines_count++;

        if(shader_data->generic.type != SHADER_TRANSPARENT_GENERIC_TYPE_2D_MAP) {
            const char *first_map_is_cube = "FIRST_MAP_IS_CUBE";
            char *first_map_type_macro =reinterpret_cast<char *>(GlobalAlloc(GMEM_FIXED, strlen(first_map_is_cube) + 1));
            strcpy(first_map_type_macro, first_map_is_cube);
            defines[defines_count].Name = first_map_type_macro;
            defines[defines_count].Definition = NULL;
            defines_count++;
        }

        defines[defines_count].Name = NULL;
        defines[defines_count].Definition = NULL;

        return defines;
    }

    static void free_defines(D3D_SHADER_MACRO *defines) noexcept {
        for(size_t i = 0; i < NUM_OF_SHADER_COMPILE_DEFINES; i++) {
            if(defines[i].Name != NULL) {
                GlobalFree(const_cast<char *>(defines[i].Name));
            }
            if(defines[i].Definition != NULL) {
                GlobalFree(const_cast<char *>(defines[i].Definition));
            }
        }
        GlobalFree(defines);
    }

    void shader_transparent_generic_release_instances() noexcept {
        // Wipe the tag cache.
        memset(generic_tag_cache, 0, sizeof(generic_tag_cache));
        generic_tag_cache_index = 0;

        // Release pixel shaders.
        for(std::uint16_t i = 0; i < MAX_GENERIC_INSTANCE_COUNT; i++) {
            if(generic_instance_cache[i].shader) {
                IDirect3DPixelShader9_Release(generic_instance_cache[i].shader);
            }
        }

        // Wipe the instance cache.
        memset(generic_instance_cache, 0, sizeof(generic_instance_cache));
        generic_instance_index = 0;
    }

    ID3DBlob *shader_transparent_generic_compile_shader(D3D_SHADER_MACRO *defines) noexcept {
        ID3DBlob *compiled_shader = NULL;
        const char *buffer = reinterpret_cast<const char *>(shader_transparent_generic_source);
        if(!rasterizer_compile_shader(buffer, "main", "ps_3_0", defines, &compiled_shader)) {
            show_error_box("Error", "Generic pixel shader failed to compile");
            std::exit(1);
        }
        return compiled_shader;
    }

#ifdef WRITE_DEFINES_TO_FILE
    void write_defines_to_file( D3D_SHADER_MACRO *defines) noexcept {
        for(size_t i = 0; i < NUM_OF_SHADER_COMPILE_DEFINES; i++) {
            if(defines[i].Name != NULL) {
                shader_file << "/D " << defines[i].Name;
            }
            if(defines[i].Definition != NULL) {
                shader_file << "=" << '"' << defines[i].Definition << '"' << " ";
            }
        }
        shader_file << "\n";
    }
#endif

    std::uint32_t shader_transparent_generic_create_instance(D3D_SHADER_MACRO *defines) {
        char *hash = generate_defines_hash(defines);

        // Do we already have a shader for these defines?
        for(std::uint16_t i = 0; i < generic_instance_index && i < MAX_GENERIC_INSTANCE_COUNT; i++) {
            if(strncmp(generic_instance_cache[i].instance_hash, hash, 32) == 0) {
                return i;
            }
        }

        // Compile a new shader if we don't already have a valid one.
        if(generic_instance_index < MAX_GENERIC_INSTANCE_COUNT) {
            ID3DBlob *compiled_shader = shader_transparent_generic_compile_shader(defines);
            IDirect3DPixelShader9 *generic_ps = nullptr;
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(compiled_shader->GetBufferPointer()), &generic_ps);
            compiled_shader->Release();

#ifdef WRITE_DEFINES_TO_FILE
            const char *hex = "0x";
            shader_file << '"';
            for(int i = 0; i < 32; i++) {
                shader_file << hex << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(static_cast<unsigned char>(hash[i])) << ",";
            }
            shader_file <<'"' << " ";
            write_defines_to_file(defines);
#endif

            memcpy(generic_instance_cache[generic_instance_index].instance_hash, hash, 32);
            generic_instance_cache[generic_instance_index].shader = generic_ps;
            generic_instance_index++;
            GlobalFree(hash);

            return generic_instance_index - 1;
        }
        else {
            show_error_box("Error", "Exceeded max generic instance count");
            std::exit(1);
        }
    }

    IDirect3DPixelShader9 *shader_transparent_generic_create(ShaderTransparentGeneric *shader_data, bool on_map_load) noexcept {
        // Check if the tag has already been used to create a generic shader. This should never happen on map load,
        // so don't waste time checking this on map load.
        if(!on_map_load) {
            for(std::uint16_t i = 0; i < generic_tag_cache_index && i < MAX_GENERIC_TAG_COUNT; i++) {
                if(shader_data == generic_tag_cache[i].tag_address) {
                    return generic_instance_cache[generic_tag_cache[i].instance_index].shader;
                }
            }
        }

        if(generic_tag_cache_index == MAX_GENERIC_TAG_COUNT) {
            show_error_box("Error", "Max number of generic tags per map reached");
            std::exit(1);
        }

        // If the tag has not been used, check whether a valid generic instance already exists.
        // Reference that if possible, otherwise compile a new shader.
        D3D_SHADER_MACRO *defines = generate_defines(shader_data);
        generic_tag_cache[generic_tag_cache_index].instance_index = shader_transparent_generic_create_instance(defines);
        generic_tag_cache[generic_tag_cache_index].tag_address = shader_data;
        free_defines(defines);

        IDirect3DPixelShader9 *shader = generic_instance_cache[generic_tag_cache[generic_tag_cache_index].instance_index].shader;
        generic_tag_cache_index++;

        return shader;
    }

    void shader_transparent_generic_create_for_new_map() noexcept {
        // Wipe the tag cache.
        memset(generic_tag_cache, 0, sizeof(generic_tag_cache));
        generic_tag_cache_index = 0;
        bool instace_cache_flushed_this_map = false;

        auto &tag_data_header = get_tag_data_header();
        auto *tags = tag_data_header.tag_array;
        auto tag_count = tag_data_header.tag_count;
        std::uint32_t i = 0;
        while(i < tag_count) {
            auto &tag = tags[i];
            i++;
            // Flush the cache, start again.
            if(generic_instance_index == MAX_GENERIC_INSTANCE_COUNT) {
                if(!instace_cache_flushed_this_map) {
                    shader_transparent_generic_release_instances();
                    instace_cache_flushed_this_map = true;
                    i = 0;
                    continue;
                }
                else {
                    // No, we're actually out of room. Exit.
                    show_error_box("Error", "Max number of generic instances exceeded");
                    std::exit(1);
                }
            }
            if(tag.primary_class != TAG_CLASS_SHADER_TRANSPARENT_GENERIC) {
                continue;
            }

            if(!tag.data) {
                continue;
            }
            else {
                shader_transparent_generic_create(reinterpret_cast<ShaderTransparentGeneric *>(tag.data), true);
            }
        }
    }

    void shader_transparent_generic_update_for_new_map() noexcept {
        shader_transparent_generic_create_for_new_map();
    }

    void shader_transparent_generic_preload_shaders() noexcept {
        // Load precompiled shaders for stock tagset.
        preload_transparent_generic_blobs();
        for(int i = 0; i < NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS; i++) {
            if(generic_instance_index < MAX_GENERIC_INSTANCE_COUNT) {
                IDirect3DPixelShader9 *generic_ps = nullptr;
                IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(generic_blobs[i]), &generic_ps);

                memcpy(generic_instance_cache[generic_instance_index].instance_hash, generic_hash[i], 32);
                generic_instance_cache[generic_instance_index].shader = generic_ps;
                generic_instance_index++;
            }
        }
    }

    // The main thingy.
    extern "C" void rasterizer_shader_transparent_generic_draw(TransparentGeometryGroup *group, bool is_dirty) noexcept {
        // We can't draw generic if the d3d9 device doesn't support ps_3_0 and vs_3_0
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0300 || d3d9_device_caps->VertexShaderVersion < 0xfffe0300) {
            return;
        }

        ShaderTransparentGeneric *shader_data = shader_transparent_generic_get(group->shader);
        std::uint16_t bitmap_index = group->shader_permutation_index;

        for(std::uint16_t layer_index = 0; layer_index < shader_data->generic.extra_layers.count; layer_index++) {
            TransparentGeometryGroup group_layer;
            memcpy(&group_layer, group, sizeof(TransparentGeometryGroup));
            group_layer.sorted_index = -1;
            group_layer.shader = get_tag(shader_transparent_generic_get_layer(shader_data, layer_index)->tag_id)->data;

            rasterizer_transparent_geometry_group_draw(&group_layer, is_dirty);
        }

        short vertex_shader_permutation = shader_get_vertex_shader_permutation(reinterpret_cast<std::byte *>(group->shader));
        short vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);
        IDirect3DVertexShader9 *vertex_shader = rasterizer_get_vertex_shader_for_permutation(vertex_shader_permutation, vertex_type);
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, vertex_shader);
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(vertex_type));
        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, NULL);

        rasterizer_set_render_state(D3DRS_CULLMODE, shader_data->generic.flags.two_sided ? D3DCULL_NONE : D3DCULL_CCW);  
        rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
        rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
        rasterizer_set_render_state(D3DRS_ALPHATESTENABLE, shader_data->generic.flags.alpha_tested ? TRUE : FALSE);
        rasterizer_set_render_state(D3DRS_ALPHAREF, 0x0000007F);

        rasterizer_set_framebuffer_blend_function(shader_data->generic.framebuffer_blend_function);

        // Numeric bitmap indexing
        if(shader_data->generic.flags.numeric && group->animation && shader_data->generic.maps.count > 0) {
            ShaderTransparentGenericMap *map = shader_transparent_generic_get_map(shader_data, 0);
            
            short base = reinterpret_cast<Bitmap *>(get_tag(map->map.tag_id)->data)->bitmap_data.count;
            short numeric_counter_source_index = (base == 8) ? 3 : 0;

            short numeric_counter_limit = shader_data->generic.numeric_counter_limit;
            short numeric_counter = PIN(fast_ftol_floor(0.5f + group->animation->values[numeric_counter_source_index] * static_cast<float>(numeric_counter_limit)), 0, numeric_counter_limit);
            for(short i = 0; i < group->shader_permutation_index; i++) {
                numeric_counter /= base;
            }

            bitmap_index = numeric_counter % base;
        }

        // Maps
        if(shader_data->generic.maps.count > 0) {
            float vsh_constants_texanim[4 * 8] = {0};

            for(std::uint16_t map_index = 0; map_index < 4; map_index++) {
                if(map_index < shader_data->generic.maps.count) {
                    ShaderTransparentGenericMap *map = shader_transparent_generic_get_map(shader_data, map_index);
                    ShaderTransparentGenericType type = shader_data->generic.type;
                    BitmapDataType bitmap_type;
                    bool is_first_map = map_index == 0;

                    const BitmapDataType bitmap_type_array[4] = {BITMAP_TYPE_2D, BITMAP_TYPE_CUBE_MAP, BITMAP_TYPE_CUBE_MAP, BITMAP_TYPE_CUBE_MAP};
                    bitmap_type = is_first_map ? bitmap_type_array[type] : static_cast<BitmapDataType>(BITMAP_TYPE_2D);

                    DWORD u_address, v_address, w_address;

                    // Set texture
                    rasterizer_set_texture(map_index, bitmap_type, BITMAP_USAGE_ADDITIVE, bitmap_index, map->map.tag_id);

                    const DWORD bitmap_address_array[4] = {D3DTADDRESS_WRAP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP};
                    if(bitmap_type == BITMAP_TYPE_2D && map->flags.map_u_clamped) {
                        u_address = D3DTADDRESS_CLAMP;
                    }
                    else {
                        u_address = is_first_map ? bitmap_address_array[type] : static_cast<DWORD>(D3DTADDRESS_WRAP);
                    }
                    if(bitmap_type == BITMAP_TYPE_2D && map->flags.map_v_clamped) {
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

                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSU, u_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSV, v_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_ADDRESSW, w_address);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MAGFILTER, filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MINFILTER, map->flags.map_point_sampled ? static_cast<DWORD>(D3DTEXF_POINT) : filter_type);
                    rasterizer_set_sampler_state(map_index, D3DSAMP_MIPFILTER, map->flags.map_point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
                }

                if(map_index < shader_data->generic.maps.count && (map_index > 0 || shader_data->generic.type == SHADER_TRANSPARENT_GENERIC_TYPE_2D_MAP)) {
                    ShaderTransparentGenericMap *map = shader_transparent_generic_get_map(shader_data, map_index);
                    Point2D map_scale = map->scale;

                    if(map_index == 0 && shader_data->generic.flags.scale_first_map_with_distance) {
                        map_scale.x *= -group->z_sort;
                        map_scale.y *= -group->z_sort;
                    }

                    if(map_index > 0 || !shader_data->generic.flags.first_map_in_screenspace) {
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
                else if(map_index < shader_data->generic.maps.count && shader_data->generic.flags.first_map_in_screenspace) {
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
                    vsh_constants_texanim[map_index * 8 + 4] = 1.0f;
                    vsh_constants_texanim[map_index * 8 + 5] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 6] = 0.0f;
                    vsh_constants_texanim[map_index * 8 + 7] = 0.0f;
                }
            }
            IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, vsh_constants_texanim, 8);
        }

        
        IDirect3DPixelShader9 *shader = shader_transparent_generic_create(shader_data, false);

        if(shader_data->generic.stages.count > 0 || shader_data->generic.maps.count > 0) {
            float ps_constants_buffer[8*4 + 8*4 + 4] = {0};

            float *stage_color0 = &ps_constants_buffer[8 * 0];
            float *stage_color1 = &ps_constants_buffer[8 * 4];
            float *fog_config   = &ps_constants_buffer[8 * 8];

            if(*fog_enabled) {
                std::uint16_t fog_stage = (shader_data->generic.stages.count > 0) ? shader_data->generic.stages.count : 1;

                fog_config[0] = 1.0f;

                if(group->geometry_flags.sky && shader_data->generic.framebuffer_blend_function == SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND) {
                    Plane3D *plane = &global_window_parameters->fog.plane;
                    Point3D *camera = &global_window_parameters->camera.position;
                    float eye_distance_to_fog_plane = plane->i * camera->x + plane->j * camera->y + plane->k * camera->z;
                    float planar_eye_density = global_window_parameters->fog.planar_maximum_density * PIN(-eye_distance_to_fog_plane / global_window_parameters->fog.planar_maximum_depth, 0.0f, 1.0f);

                    stage_color0[fog_stage * 4 + 0] = global_window_parameters->fog.planar_color.red;
                    stage_color0[fog_stage * 4 + 1] = global_window_parameters->fog.planar_color.green;
                    stage_color0[fog_stage * 4 + 2] = global_window_parameters->fog.planar_color.blue;
                    stage_color0[fog_stage * 4 + 3] = global_window_parameters->fog.planar_maximum_density * planar_eye_density;

                    fog_config[1] = 1.0f;
                }
                else {
                    float vsh_constants_texscale[3 * 4] = {0};
                    vsh_constants_texscale[10] = 1.0f;

                    if(group->effect.type == RENDER_MODEL_EFFECT_TYPE_ACTIVE_CAMOUFLAGE) {
                        vsh_constants_texscale[10] *= PIN(1.0f - group->effect.intensity, 0.0f, 1.0f);
                    }

                    if(shader_data->generic.framebuffer_fade_source > 0 && group->animation != NULL && group->animation->values != NULL) {
                        vsh_constants_texscale[10] *= group->animation->values[shader_data->generic.framebuffer_fade_source - 1];
                    }

                    IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 10, vsh_constants_texscale, 3);
                }
            }

            if(shader_data->generic.stages.count > 0) {
                for(std::uint16_t i = 0; i < shader_data->generic.stages.count; i++) {
                    ShaderTransparentGenericStage *stage = shader_transparent_generic_get_stage(shader_data, i);
                    
                    float progress = 0.0f;
                    if(group->animation != NULL && group->animation->values != NULL && stage->flags.A_controls_color0_animation) {
                        progress = group->animation->values[0];
                    }
                    else {
                        progress = periodic_function_evaluate(global_frame_parameters->elapsed_time_sec / stage->constant_color0_animation_period, stage->constant_color0_animation_function);
                    }

                    ColorARGB constant_color0;
                    constant_color0.alpha = stage->constant_color0_upper_bound.alpha - stage->constant_color0_lower_bound.alpha;
                    constant_color0.red = stage->constant_color0_upper_bound.red - stage->constant_color0_lower_bound.red;
                    constant_color0.green = stage->constant_color0_upper_bound.green - stage->constant_color0_lower_bound.green;
                    constant_color0.blue = stage->constant_color0_upper_bound.blue - stage->constant_color0_lower_bound.blue;

                    constant_color0.alpha = stage->constant_color0_lower_bound.alpha + constant_color0.alpha * progress;
                    constant_color0.red = stage->constant_color0_lower_bound.red + constant_color0.red * progress;
                    constant_color0.green = stage->constant_color0_lower_bound.green + constant_color0.green * progress;
                    constant_color0.blue = stage->constant_color0_lower_bound.blue + constant_color0.blue * progress;

                    if(stage->constant_color0_source > 0 && stage->constant_color0_source < 5 && group->animation != NULL && group->animation->colors != NULL) {
                        ColorRGB *external_color = &group->animation->colors[stage->constant_color0_source - 1];
                        constant_color0.red *= external_color->red;
                        constant_color0.green *= external_color->green;
                        constant_color0.blue *= external_color->blue;
                    }

                    stage_color0[i * 4 + 0] = constant_color0.red;
                    stage_color0[i * 4 + 1] = constant_color0.green;
                    stage_color0[i * 4 + 2] = constant_color0.blue;
                    stage_color0[i * 4 + 3] = constant_color0.alpha;
                    stage_color1[i * 4 + 0] = stage->constant_color1.red;
                    stage_color1[i * 4 + 1] = stage->constant_color1.green;
                    stage_color1[i * 4 + 2] = stage->constant_color1.blue;
                    stage_color1[i * 4 + 3] = stage->constant_color1.alpha;
                }
            }

            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, stage_color0, 8);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 8, stage_color1, 8);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 16, fog_config, 1);
        }

        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, shader);
        rasterizer_transparent_geometry_group_draw_vertices(group, FALSE);
        IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
    }

#ifdef WRITE_DEFINES_TO_FILE
    void close_shader_defines() noexcept {
        shader_file.close();
    }
#endif

    void set_up_shader_transparent_generic() noexcept {
        if(chimera_rasterizer_enabled) {
            add_game_exit_event(shader_transparent_generic_release_instances);
            add_map_load_event(shader_transparent_generic_update_for_new_map);
            add_game_start_event(shader_transparent_generic_preload_shaders);

            auto *switch_ptr = reinterpret_cast<std::uint32_t *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("transparent_shader_draw_switch_sig").data() + 3) + 4 * 4);
            generic_switch_return = *reinterpret_cast<std::byte **>(switch_ptr);
            overwrite(switch_ptr, reinterpret_cast<uint32_t>(shader_transparent_generic_switch_function));

#ifdef WRITE_DEFINES_TO_FILE
            shader_file.open ("shader_defines.txt");
            add_game_exit_event(close_shader_defines);
#endif
        }
    }

}