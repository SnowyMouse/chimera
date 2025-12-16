// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SHADER_DEFS_HPP
#define CHIMERA_SHADER_DEFS_HPP

#include <cstddef>
#include <cstdint>

#include "pad.hpp"
#include "tag.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

    /**
    * Constants
    */
    enum ShaderType : short {
        SHADER_TYPE_SCREEN,
        SHADER_TYPE_EFFECT,
        SHADER_TYPE_DECAL,
        SHADER_TYPE_ENVIRONMENT,
        SHADER_TYPE_MODEL,
        SHADER_TYPE_TRANSPARENT_GENERIC,
        SHADER_TYPE_TRANSPARENT_CHICAGO,
        SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED,
        SHADER_TYPE_TRANSPARENT_WATER,
        SHADER_TYPE_TRANSPARENT_GLASS,
        SHADER_TYPE_TRANSPARENT_METER,
        SHADER_TYPE_TRANSPARENT_PLASMA,
        NUMBER_OF_SHADER_TYPES
    };

    enum FunctionOut : short {
        FUNCTION_OUT_NONE = 0,
        FUNCTION_OUT_A,
        FUNCTION_OUT_B,
        FUNCTION_OUT_C,
        FUNCTION_OUT_D,
    };

    enum WaveFunction : short {
        WAVE_FUNCTION_ONE = 0,
        WAVE_FUNCTION_ZERO,
        WAVE_FUNCTION_COSINE,
        WAVE_FUNCTION_COSINE_VARIABLE_PERIOD,
        WAVE_FUNCTION_DIAGONAL_WAVE,
        WAVE_FUNCTION_DIAGONAL_WAVE_VARIABLE_PERIOD,
        WAVE_FUNCTION_SLIDE,
        WAVE_FUNCTION_SLIDE_VARIABLE_PERIOD,
        WAVE_FUNCTION_NOISE,
        WAVE_FUNCTION_JITTER,
        WAVE_FUNCTION_WANDER,
        WAVE_FUNCTION_SPARK
    };

    struct ShaderTextureAnimation {
        // animated u-offset
        FunctionOut u_source;
        WaveFunction u_function;
        float  u_period;
        float  u_phase;
        float  u_scale;

        // animated v-offset
        FunctionOut v_source;
        WaveFunction v_function;
        float  v_period;
        float  v_phase;
        float  v_scale;

        // animated rotation
        FunctionOut r_source;
        WaveFunction r_function;
        float  r_period;
        float  r_phase;
        float  r_scale;
        Point2D r_center;
    };
    static_assert(sizeof(ShaderTextureAnimation) == 0x38);


    /**
    * Shader
    */
    enum FramebufferBlendFunction : std::uint16_t {
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_ADD,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_REVERSE_SUBTRACT,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_MIN,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_MAX,
        SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD,
        NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS
    };

    enum FramebufferFadeMode : std::uint16_t {
        SHADER_FRAMEBUFFER_FADE_MODE_NONE,
        SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PERPENDICULAR,
        SHADER_FRAMEBUFFER_FADE_MODE_FADE_WHEN_PARALLEL,
        NUMBER_OF_SHADER_FRAMEBUFFER_FADE_MODES
    };

    enum RadiosityFlags {
        SHADER_RADIOSITY_FLAGS_SIMPLE_PARAMETERIZATION_BIT,
        SHADER_RADIOSITY_FLAGS_IGNORE_NORMALS_BIT,
        SHADER_RADIOSITY_FLAGS_TRANSPARENT_LIT_BIT,
        NUMBER_OF_SHADER_RADIOSITY_FLAGS
    };

    enum RadiosityDetail : std::uint16_t {
        RADIOSITY_DETAIL_HIGH,
        RADIOSITY_DETAIL_MEDIUM,
        RADIOSITY_DETAIL_LOW,
        RADIOSITY_DETAIL_TURD,
        NUMBER_OF_SHADER_RADIOSITY_DETAIL_LEVELS
    };

    struct ShaderRadiosityProperties {
        std::uint16_t flags;
        RadiosityDetail detail_level;
        float power;
        ColorRGB color;
        ColorRGB tint_color;
    };
    static_assert(sizeof(ShaderRadiosityProperties) == 0x20);

    struct ShaderPhysicsProperties {
        std::uint16_t flags;
        short material_type;
    };
    static_assert(sizeof(ShaderPhysicsProperties) == 0x4);

    struct _shader {
        ShaderRadiosityProperties radiosity;
        ShaderPhysicsProperties physics;

        ShaderType type;
        std::uint16_t pad;
    };
    static_assert(sizeof(_shader) == 0x28);


    /**
    * shader_transparent_generic
    */
    enum ShaderTransparentGenericConstants {
        MAXIMUM_MAPS_PER_SHADER_TRANSPARENT_GENERIC=   4,
        MAXIMUM_STAGES_PER_SHADER_TRANSPARENT_GENERIC= 7,
    };

    enum ShaderTransparentGenericMapFlags {
        SHADER_TRANSPARENT_GENERIC_MAP_FLAGS_POINT_SAMPLED_BIT,
        SHADER_TRANSPARENT_GENERIC_MAP_FLAGS_U_CLAMPED_BIT,
        SHADER_TRANSPARENT_GENERIC_MAP_FLAGS_V_CLAMPED_BIT,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_MAP_FLAGS
    };

    struct ShaderTransparentGenericMap {
        std::uint16_t flags;
        short type;
        Point2D scale;
        Point2D offset;
        float rotation;
        float mipmap_bias;
        TagReference map;
        ShaderTextureAnimation animation;
    };
    static_assert(sizeof(ShaderTransparentGenericMap) == 0x64);

    enum ShaderTransparentGenericStageFlags {
        SHADER_TRANSPARENT_GENERIC_STAGE_FLAGS_COLOR_MUX_BIT,
        SHADER_TRANSPARENT_GENERIC_STAGE_FLAGS_ALPHA_MUX_BIT,
        SHADER_TRANSPARENT_GENERIC_STAGE_FLAGS_A_CONTROLS_COLOR0_ANIMATION_BIT,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_FLAGS
    };

    enum ShaderTransparentGenericStageColorInputs : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ZERO,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_NEGATIVE_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_NEGATIVE_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_COLOR_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_COLOR_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_COLOR_0_DIFFUSE_LIGHT,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_COLOR_1_FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_CONSTANT_COLOR_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_CONSTANT_COLOR_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_ALPHA_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_MAP_ALPHA_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_ALPHA_0_FADE_NONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_VERTEX_ALPHA_1_FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_SCRATCH_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_CONSTANT_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUT_CONSTANT_ALPHA_1,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_COLOR_INPUTS
    };

    enum ShaderTransparentGenericStageAlphaInputs : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_ZERO,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_NEGATIVE_ONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_NEGATIVE_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_ALPHA_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_ALPHA_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_ALPHA_0_FADE_NONE,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_ALPHA_1_FADE_PERPENDICULAR,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_CONSTANT_ALPHA_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_CONSTANT_ALPHA_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_BLUE_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_BLUE_2,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_MAP_BLUE_3,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_BLUE_0_BLUE_LIGHT,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_VERTEX_BLUE_1_FADE_PARALLEL,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_SCRATCH_BLUE_1,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_CONSTANT_BLUE_0,
        SHADER_TRANSPARENT_GENERIC_STAGE_ALPHA_INPUT_CONSTANT_BLUE,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_APHA_INPUTS
    };

    enum ShaderTransparentGenericStageOutputs : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_DISCARD,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH0,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_SCRATCH1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_VERTEX_COLOR0,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_VERTEX_COLOR1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_TEXTURE0,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_TEXTURE1,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_TEXTURE2,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_TEXTURE3,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS
    };

    enum ShaderTransparentGenericStageOutputFunctions : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTION_MULTIPLY,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTION_DOT_PRODUCT,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS
    };

    enum ShaderTransparentGenericStageInputMappings : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_UNSIGNED_IDENTITY,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_UNSIGNED_INVERT,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_EXPAND_NORMAL,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_EXPAND_NEGATE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_HALFBIAS_NORMAL,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_HALFBIAS_NEGATE,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_SIGNED_IDENTITY,
        SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPING_SIGNED_NEGATE,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS
    };

    enum ShaderTransparentGenericStageOutputMappings : short {
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_IDENTITY,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_SCALE_BY_ONE_HALF,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_SCALE_BY_TWO,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_SCALE_BY_FOUR,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_BIAS,
        SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPING_BIAS_AND_SCALE_BY_TWO,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS
    };

    struct ShaderTransparentGenericStage {
        std::uint16_t flags;
        short type;

        short constant_color0_source;
        WaveFunction constant_color0_animation_function;
        float constant_color0_animation_period;
        ColorARGB constant_color0_lower_bound;
        ColorARGB constant_color0_upper_bound;
        ColorARGB constant_color1;

        ShaderTransparentGenericStageColorInputs color_input_A;
        ShaderTransparentGenericStageInputMappings color_input_A_mapping;
        ShaderTransparentGenericStageColorInputs color_input_B;
        ShaderTransparentGenericStageInputMappings color_input_B_mapping;
        ShaderTransparentGenericStageColorInputs color_input_C;
        ShaderTransparentGenericStageInputMappings color_input_C_mapping;
        ShaderTransparentGenericStageColorInputs color_input_D;
        ShaderTransparentGenericStageInputMappings color_input_D_mapping;
        ShaderTransparentGenericStageOutputs color_output_AB;
        ShaderTransparentGenericStageOutputFunctions color_output_AB_function;
        ShaderTransparentGenericStageOutputs color_output_CD;
        ShaderTransparentGenericStageOutputFunctions color_output_CD_function;
        ShaderTransparentGenericStageOutputs color_output_AB_CD;
        ShaderTransparentGenericStageOutputMappings color_output_mapping;

        ShaderTransparentGenericStageAlphaInputs alpha_input_A;
        ShaderTransparentGenericStageInputMappings alpha_input_A_mapping;
        ShaderTransparentGenericStageAlphaInputs alpha_input_B;
        ShaderTransparentGenericStageInputMappings alpha_input_B_mapping;
        ShaderTransparentGenericStageAlphaInputs alpha_input_C;
        ShaderTransparentGenericStageInputMappings alpha_input_C_mapping;
        ShaderTransparentGenericStageAlphaInputs alpha_input_D;
        ShaderTransparentGenericStageInputMappings alpha_input_D_mapping;
        ShaderTransparentGenericStageOutputs alpha_output_AB;
        ShaderTransparentGenericStageOutputs alpha_output_CD;
        ShaderTransparentGenericStageOutputs alpha_output_AB_CD;
        ShaderTransparentGenericStageOutputMappings alpha_output_mapping;
    };
    static_assert(sizeof(ShaderTransparentGenericStage) == 0x70);

    enum ShaderTransparentGenericType : short {
        SHADER_TRANSPARENT_GENERIC_TYPE_2D_MAP,
        SHADER_TRANSPARENT_GENERIC_TYPE_FIRST_MAP_IS_REFLECTION_CUBE_MAP,
        SHADER_TRANSPARENT_GENERIC_TYPE_FIRST_MAP_IS_OBJECT_CENTERED_CUBE_MAP,
        SHADER_TRANSPARENT_GENERIC_TYPE_FIRST_MAP_IS_VIEWER_CENTERED_CUBE_MAP,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_TYPES
    };

    enum ShaderTransparentGenericFlags {
        SHADER_TRANSPARENT_GENERIC_FLAGS_ALPHA_TESTED_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_DECAL_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_TWO_SIDED_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_FIRST_MAP_IS_IN_SCREENSPACE_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_DRAW_BEFORE_WATER_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_IGNORE_EFFECT_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_SCALE_FIRST_MAP_WITH_DISTANCE_BIT,
        SHADER_TRANSPARENT_GENERIC_FLAGS_NUMERIC_BIT,
        NUMBER_OF_SHADER_TRANSPARENT_GENERIC_FLAGS
    };

    struct _shader_transparent_generic {
        std::uint8_t numeric_counter_limit;
        std::uint8_t flags;

        ShaderTransparentGenericType type;
        FramebufferBlendFunction framebuffer_blend_function;
        FramebufferFadeMode framebuffer_fade_mode;
        short framebuffer_fade_source;
        short framebuffer_fade_unused;

        float lens_flare_spacing;
        TagReference lens_flare;

        TagBlock extra_layers;
        TagBlock maps;
        TagBlock stages;
    };
    static_assert(sizeof(_shader_transparent_generic) == 0x44);

    struct ShaderTransparentGeneric {
        _shader shader;
        _shader_transparent_generic generic;
    };
    static_assert(sizeof(ShaderTransparentGeneric) == 0x6C);


    /**
    * shader_transparent_chicago - basically generic without the stages
    */
    enum ShaderTransparentChicagoMapFlags {
        SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_POINT_SAMPLED_BIT,
        SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_ALPHA_REPLICATE,
        SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_U_CLAMPED_BIT,
        SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS_V_CLAMPED_BIT,
        NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_MAP_FLAGS
    };

    enum ShaderTransparentChicagoExtraFlags {
        SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_DONT_FADE_ACTIVE_CAMOUFLAGE,
        SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_NUMERIC_COUNTDOWN_TIMER,
        SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING,
    };

    struct ShaderTransparentChicagoMap {
        std::uint16_t flags;
        PAD(0x2);
        PAD(0x28);
        short color_function;
        short alpha_function;
        PAD(0x24);

        Point2D scale;
        Point2D offset;
        float rotation;
        float mipmap_bias;
        TagReference map;
        PAD(0x28);
        ShaderTextureAnimation animation;
    };
    static_assert(sizeof(ShaderTransparentChicagoMap) == 0xDC);

    struct _shader_transparent_chicago {
        std::uint8_t numeric_counter_limit;
        std::uint8_t flags;

        ShaderTransparentGenericType type;
        FramebufferBlendFunction framebuffer_blend_function;
        FramebufferFadeMode framebuffer_fade_mode;
        short framebuffer_fade_source;
        short framebuffer_fade_unused;

        float lens_flare_spacing;
        TagReference lens_flare;

        TagBlock extra_layers;
        TagBlock maps;
        std::uint32_t extra_flags;
    };
    static_assert(sizeof(_shader_transparent_chicago) == 0x3C);

    struct ShaderTransparentChicago {
        _shader shader;
        _shader_transparent_chicago chicago;
    };
    static_assert(sizeof(ShaderTransparentChicago) == 0x64);


    /**
    * shader_transparent_chicago_extended - basically chicago with added cringe
    */
    struct _shader_transparent_chicago_extended {
        std::uint8_t numeric_counter_limit;
        std::uint8_t flags;

        ShaderTransparentGenericType type;
        FramebufferBlendFunction framebuffer_blend_function;
        FramebufferFadeMode framebuffer_fade_mode;
        short framebuffer_fade_source;
        short framebuffer_fade_unused;

        float lens_flare_spacing;
        TagReference lens_flare;

        TagBlock extra_layers;
        TagBlock maps_4_stage;
        TagBlock maps_2_stage;
        std::uint32_t extra_flags;
        PAD(0x8);
    };
    static_assert(sizeof(_shader_transparent_chicago_extended) == 0x50);

    struct ShaderTransparentChicagoExtended {
        _shader shader;
        _shader_transparent_chicago_extended chicago_extended;
    };
    static_assert(sizeof(ShaderTransparentChicagoExtended) == 0x78);


    /**
    * shader_transparent_glass
    */
    enum ShaderTransparentGlassFlags : std::uint16_t {
        SHADER_TRANSPARENT_GLASS_FLAGS_ALPHA_TESTED_BIT,
        SHADER_TRANSPARENT_GLASS_FLAGS_DECAL_BIT,
        SHADER_TRANSPARENT_GLASS_FLAGS_TWO_SIDED_BIT,
        SHADER_TRANSPARENT_GLASS_FLAGS_BUMP_MAP_IS_SPECULAR_MASK_BIT,
        NUMBER_OF_SHADER_TRANSPARENT_GLASS_FLAGS
    };

    enum ShaderTransparentGlassReflectionType : short {
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_BUMPED,
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_FLAT,
        SHADER_TRANSPARENT_GLASS_REFLECTION_TYPE_MIRROR,
        NUMBER_OF_SHADER_TRANSPARENT_GLASS_REFLECTION_TYPES
    };

    struct _shader_transparent_glass {
        std::uint16_t flags;
        short type;
        
        // tint pass
        PAD(0x28);
        ColorRGB tint_color;
        float tint_map_scale;
        TagReference tint_map;
        PAD(0x14);
        
        // reflection pass
        PAD(0x2);
        short reflection_type;
        float reflection_view_perpendicular_brightness;
        ColorRGB reflection_view_perpendicular_color;
        float reflection_view_parallel_brightness;
        ColorRGB reflection_view_parallel_color;
        TagReference reflection_map;
        float reflection_bump_map_scale;
        TagReference reflection_bump_map;
        
        PAD(0x80);
        
        // diffuse pass
        PAD(0x4);
        float diffuse_map_scale;
        TagReference diffuse_map;
        float diffuse_detail_map_scale;
        TagReference diffuse_detail_map;
        PAD(0x1C);
        
        // alpha-blended specular pass
        PAD(0x4);
        float specular_map_scale;
        TagReference specular_map;
        float specular_detail_map_scale;
        TagReference specular_detail_map;
        PAD(0x1C);
    };
    static_assert(sizeof(_shader_transparent_glass) == 0x1B8);

    struct ShaderTransparentGlass {
        _shader shader;
        _shader_transparent_glass glass;
    };
    static_assert(sizeof(ShaderTransparentGlass) == 0x1E0);

}

#endif
