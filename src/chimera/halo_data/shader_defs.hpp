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
    * shader_effect
    */
    enum ShaderEffectFlags
    {
        SHADER_EFFECT_FLAGS_SORT_BIAS_BIT,
        SHADER_EFFECT_FLAGS_USES_NONLINEAR_TINT_BIT,
        SHADER_EFFECT_FLAGS_FLAGS_DONT_OVERDRAW_FIRST_PERSON_WEAPON_BIT,
        NUMBER_OF_SHADER_EFFECT_FLAGS
    };

    enum ShaderEffectParticleAnchor {
        SHADER_EFFECT_PARTICLE_ANCHOR_WITH_PRIMARY,
        SHADER_EFFECT_PARTICLE_ANCHOR_SCREEN_SPACE,
        SHADER_EFFECT_PARTICLE_ANCHOR_ZSPRITE,
        NUMBER_OF_SHADER_EFFECT_SECONDARY_MAP_ANCHORS
    };

    enum ShaderEffectMapFlags
    {
        SHADER_EFFECT_MAP_FLAGS_POINT_SAMPLED_BIT,
        SHADER_EFFECT_MAP_U_CLAMP_BIT,
        SHADER_EFFECT_MAP_V_CLAMP_BIT,
        NUMBER_OF_SHADER_EFFECT_MAP_FLAGS
    };

    struct _shader_effect {
        std::uint16_t flags;

        FramebufferBlendFunction framebuffer_blend_function;
        FramebufferFadeMode framebuffer_fade_mode;

        std::uint16_t primary_map_flags;
        PAD(0x1C);

        TagReference secondary_map;
        short secondary_map_anchor;
        std::uint16_t secondary_map_flags;
        ShaderTextureAnimation secondary_map_animation;

        float secondary_map_radius;
        float zsprite_radius_scale;
        PAD(0x14);
    };
    static_assert(sizeof(_shader_effect) == 0x8C);

    struct ShaderEffect {
        _shader shader;
        _shader_effect effect;
    };
    static_assert(sizeof(ShaderEffect) == 0xB4);


    /**
    * shader_environment
    */
    enum ShaderEnvironmentDiffuseFlags {
        SHADER_ENVIRONMENT_DIFFUSE_FLAGS_RESCALE_DETAIL_MAPS_BIT,
        SHADER_ENVIRONMENT_DIFFUSE_FLAGS_RESCALE_BUMP_MAP_BIT,
        NUMBER_OF_SHADER_ENVIRONMENT_DIFFUSE_FLAGS
    };

    enum ShaderEnvironmentDetailFunction : std::uint16_t {
        SHADER_ENVIRONMENT_DETAIL_FUNCTION_BIASED_MULTIPLY,
        SHADER_ENVIRONMENT_DETAIL_FUNCTION_MULTIPLY,
        SHADER_ENVIRONMENT_DETAIL_FUNCTION_BIASED_ADD,
        NUMBER_OF_SHADER_ENVIRONMENT_DETAIL_FUNCTIONS
    };

    struct ShaderEnvironmentDiffuseProperties {
        std::uint16_t flags;
        std::int16_t type;
        PAD(0x18);
        TagReference base_map;
        PAD(0x18);
        ShaderEnvironmentDetailFunction detail_map_function;
        PAD(0x2);
        float primary_detail_map_scale;
        TagReference primary_detail_map;
        float secondary_detail_map_scale;
        TagReference secondary_detail_map;
        PAD(0x18);
        ShaderEnvironmentDetailFunction micro_detail_map_function;
        PAD(0x2);
        float micro_detail_map_scale;
        TagReference micro_detail_map;
        ColorRGB material_color;
        PAD(0xC);
        float bump_map_scale;
        TagReference bump_map;
        Vector2D runtime_bump_map_scale;
        PAD(0x10);
        WaveFunction u_animation_function;
        PAD(0x2);
        float u_animation_period;
        float u_animation_scale;
        WaveFunction v_animation_function;
        PAD(0x2);
        float v_animation_period;
        float v_animation_scale;
        PAD(0x18);
    };
    static_assert(sizeof(ShaderEnvironmentDiffuseProperties) == 0x114);

    enum ShaderEnvironmentSelfIlluminationFlags {
        SHADER_ENVIRONMENT_SELF_ILLUMINATION_FLAGS_MAP_POINT_SAMPLED_BIT,
        NUMBER_OF_SHADER_ENVIRONMENT_SELF_ILLUMINATION_FLAGS
    };

    struct ShaderEnvironmentSelfIlluminationProperties {
        std::uint16_t flags;
        std::int16_t type;
        PAD(0x18);
        ColorRGB primary_on_color;
        ColorRGB primary_off_color;
        WaveFunction primary_animation_function;
        PAD(0x2);
        float primary_animation_period;
        float primary_animation_phase;
        PAD(0x18);
        ColorRGB secondary_on_color;
        ColorRGB secondary_off_color;
        WaveFunction secondary_animation_function;
        PAD(0x2);
        float secondary_animation_period;
        float secondary_animation_phase;
        PAD(0x18);
        ColorRGB plasma_on_color;
        ColorRGB plasma_off_color;
        WaveFunction plasma_animation_function;
        PAD(0x2);
        float plasma_animation_period;
        float plasma_animation_phase;
        PAD(0x18);
        float map_scale;
        TagReference map;
        PAD(0x18);
    };
    static_assert(sizeof(ShaderEnvironmentSelfIlluminationProperties) == 0xFC);

    enum ShaderEnvironmentSpecularFlags {
        SHADER_ENVIRONMENT_SPECULAR_FLAGS_OVERBRIGHT_BIT,
        SHADER_ENVIRONMENT_SPECULAR_FLAGS_EXTRA_SHINY_BIT,
        SHADER_ENVIRONMENT_SPECULAR_FLAGS_LIGHTMAP_BIT,
        NUMBER_OF_SHADER_ENVIRONMENT_SPECULAR_FLAGS
    };

    struct ShaderEnvironmentSpecularProperties {
        std::uint16_t flags;
        std::int16_t type;
        PAD(0x10);
        float brightness;
        PAD(0x14);
        ColorRGB view_perpendicular_color;
        ColorRGB view_parallel_color;
        PAD(0x10);
    };
    static_assert(sizeof(ShaderEnvironmentSpecularProperties) == 0x54);

    enum ShaderEnvironmentReflectionFlags {
        SHADER_ENVIRONMENT_REFLECTION_FLAGS_MIRROR_BIT,
        NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_FLAGS
    };

    enum ShaderEnvironmentReflectionType : std::uint16_t {
        SHADER_ENVIRONMENT_REFLECTION_TYPE_BUMPED,
        SHADER_ENVIRONMENT_REFLECTION_TYPE_FLAT,
        SHADER_ENVIRONMENT_REFLECTION_TYPE_RADIOSITY,
        NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES
    };

    struct ShaderEnvironmentReflectionProperties {
        std::uint16_t flags;
        ShaderEnvironmentReflectionType type;
        float lightmap_brightness_scale;
        PAD(0x1C);
        float view_perpendicular_brightness;
        float view_parallel_brightness;
        PAD(0x10);
        float mirror_index_of_refraction;
        float mirror_depth;
        PAD(0x10);
        TagReference map;
        PAD(0x10);
    };
    static_assert(sizeof(ShaderEnvironmentReflectionProperties) == 0x74);

    enum ShaderEnvironmentType : std::uint16_t {
        SHADER_ENVIRONMENT_TYPE_NORMAL,
        SHADER_ENVIRONMENT_TYPE_BLENDED,
        SHADER_ENVIRONMENT_TYPE_BLENDED_BASE_SPECULAR_MASK,
        NUMBER_OF_SHADER_ENVIRONMENT_TYPES
    };

    enum ShaderEnvironmentFlags {
        SHADER_ENVIRONMENT_FLAGS_ALPHA_TESTED_BIT,
        SHADER_ENVIRONMENT_FLAGS_BUMP_MAP_IS_SPECULAR_MASK_BIT,
        SHADER_ENVIRONMENT_FLAGS_TRUE_ATMOSPHERIC_FOG_BIT,
        SHADER_ENVIRONMENT_FLAGS_USE_ALTERNATE_BUMP_ATTENUATION_BIT,
        NUMBER_OF_SHADER_ENVIRONMENT_FLAGS
    };

    struct _shader_environment {
        std::uint16_t flags;
        ShaderEnvironmentType type;
        float lens_flare_spacing;
        TagReference lens_flare;
        PAD(0x2C);
        ShaderEnvironmentDiffuseProperties diffuse;
        ShaderEnvironmentSelfIlluminationProperties self_illumination;
        ShaderEnvironmentSpecularProperties specular;
        ShaderEnvironmentReflectionProperties reflection;
    };

    struct ShaderEnvironment {
        struct _shader shader;
        struct _shader_environment environment;
    };
    static_assert(sizeof(ShaderEnvironment) == 0x344);


    /**
    * shader_model
    */
    enum ShaderModelFlags {
        SHADER_MODEL_FLAGS_DETAIL_AFTER_REFLECTION_BIT,
        SHADER_MODEL_FLAGS_TWO_SIDED_BIT,
        SHADER_MODEL_FLAGS_NOT_ALPHA_TESTED_BIT,
        SHADER_MODEL_FLAGS_ALPHA_BLENDED_DECAL_BIT,
        SHADER_MODEL_FLAGS_TRUE_ATMOSPHERIC_FOG_BIT,
        SHADER_MODEL_FLAGS_DISABLE_TWO_SIDED_CULLING_BIT,
        SHADER_MODEL_FLAGS_USE_XBOX_MULTIPURPOSE_CHANNEL_ORDER_BIT,
        NUMBER_OF_SHADER_MODEL_FLAGS
    };

    enum ShaderModelDetailMask : short {
        SHADER_MODEL_DETAIL_MASK_NONE,
        SHADER_MODEL_DETAIL_MASK_REFLECTION_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_REFLECTION_MASK,
        SHADER_MODEL_DETAIL_MASK_SELF_ILLUMINATION_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_SELF_ILLUMINATION_MASK,
        SHADER_MODEL_DETAIL_MASK_CHANGE_COLOR_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_CHANGE_COLOR_MASK,
        SHADER_MODEL_DETAIL_MASK_AUXILIARY_MASK_INVERSE,
        SHADER_MODEL_DETAIL_MASK_AUXILIARY_MASK,
        NUMBER_OF_SHADER_MODEL_DETAIL_MASKS
    };

    enum ShaderModelDetailFunctions : short {
        SHADER_MODEL_DETAIL_FUNCTION_BIASED_MULTIPLY,
        SHADER_MODEL_DETAIL_FUNCTION_MULTIPLY,
        SHADER_MODEL_DETAIL_FUNCTION_BIASED_ADD,
        NUMBER_OF_SHADER_MODEL_DETAIL_FUNCTIONS
    };

    enum ShaderModelSelfIlluminationFlags {
        SHADER_MODEL_SELF_ILLUMINATION_NO_RANDOM_PHASE_BIT,
        NUMBER_OF_SHADER_MODEL_SELF_ILLUMINATION_FLAGS
    };

    struct _shader_model {
        std::uint16_t flags;
        PAD(0x2);
        PAD(0xC);

        float translucency;
        PAD(0x10);

        FunctionOut diffuse_change_color_source;
        PAD(0x2);
        PAD(0x1C);

        std::uint16_t self_illumination_flags;
        PAD(0x2);
        FunctionOut self_illumination_color_source;
        short self_illumination_animation_function;
        float self_illumination_animation_period;
        ColorRGB self_illumination_animation_color_lower_bound;
        ColorRGB self_illumination_animation_color_upper_bound;
        PAD(0xC);

        Point2D map_scale;
        TagReference base_map;
        PAD(0x8);
        TagReference multipurpose_map;
        PAD(0x8);
        ShaderModelDetailFunctions detail_function;
        ShaderModelDetailMask detail_mask;
        float detail_map_scale;
        TagReference detail_map;
        float detail_map_v_scale;
        PAD(0xC);

        ShaderTextureAnimation animation;
        PAD(0x8);

        float reflection_falloff_distance;
        float reflection_cutoff_distance;
        ColorARGB reflection_view_perpendicular_color;
        ColorARGB reflection_view_parallel_color;
        TagReference reflection_map;
        PAD(0x10);

        float reflection_bump_map_scale;
        TagReference reflection_bump_map;
        PAD(0x20);
    };
    static_assert(sizeof(_shader_model) == 0x190);

    struct ShaderModel {
        _shader shader;
        _shader_model model;
    };
    static_assert(sizeof(ShaderModel) == 0x1B8);


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
