sampler TS0 : register(s0);
sampler TS1 : register(s1);
sampler TS2 : register(s2);
sampler TS3 : register(s3);
float4 stage_color0[8] : register(c0);
float4 stage_color1[8] : register(c8);
float2 fog_config : register(c16);

#define color_input_zero                                    0
#define color_input_one                                     1
#define color_input_one_half                                2
#define color_input_negative_one                            3
#define color_input_negative_one_half                       4
#define color_input_texture0                                5
#define color_input_texture1                                6
#define color_input_texture2                                7
#define color_input_texture3                                8
#define color_input_vertex_color0                           9
#define color_input_vertex_color1                           10
#define color_input_scratch0                                11
#define color_input_scratch1                                12
#define color_input_constant0                               13
#define color_input_constant1                               14
#define color_input_texture0_alpha                          15
#define color_input_texture1_alpha                          16
#define color_input_texture2_alpha                          17
#define color_input_texture3_alpha                          18
#define color_input_vertex_color0_alpha                     19
#define color_input_vertex_color1_alpha                     20
#define color_input_scratch0_alpha                          21
#define color_input_scratch1_alpha                          22
#define color_input_constant0_alpha                         23
#define color_input_constant1_alpha                         24

#define SELECT_COLOR_INPUT(v, i, m)                         \
    if(i == color_input_zero)                               \
        v = 0;                                              \
    if(i == color_input_one)                                \
        v = 1;                                              \
    if(i == color_input_one_half)                           \
        v = 0.5;                                            \
    if(i == color_input_negative_one)                       \
        v = -1;                                             \
    if(i == color_input_negative_one_half)                  \
        v = -0.5;                                           \
    if(i == color_input_texture0)                           \
        v = t0.rgb;                                         \
    if(i == color_input_texture1)                           \
        v = t1.rgb;                                         \
    if(i == color_input_texture2)                           \
        v = t2.rgb;                                         \
    if(i == color_input_texture3)                           \
        v = t3.rgb;                                         \
    if(i == color_input_vertex_color0)                      \
        v = v0.rgb;                                         \
    if(i == color_input_vertex_color1)                      \
        v = v1.rgb;                                         \
    if(i == color_input_scratch0)                           \
        v = r0.rgb;                                         \
    if(i == color_input_scratch1)                           \
        v = r1.rgb;                                         \
    if(i == color_input_constant0)                          \
        v = c0.rgb;                                         \
    if(i == color_input_constant1)                          \
        v = c1.rgb;                                         \
    if(i == color_input_texture0_alpha)                     \
        v = t0.aaa;                                         \
    if(i == color_input_texture1_alpha)                     \
        v = t1.aaa;                                         \
    if(i == color_input_texture2_alpha)                     \
        v = t2.aaa;                                         \
    if(i == color_input_texture3_alpha)                     \
        v = t3.aaa;                                         \
    if(i == color_input_vertex_color0_alpha)                \
        v = v0.aaa;                                         \
    if(i == color_input_vertex_color1_alpha)                \
        v = v1.aaa;                                         \
    if(i == color_input_scratch0_alpha)                     \
        v = r0.aaa;                                         \
    if(i == color_input_scratch1_alpha)                     \
        v = r1.aaa;                                         \
    if(i == color_input_constant0_alpha)                    \
        v = c0.aaa;                                         \
    if(i == color_input_constant1_alpha)                    \
        v = c1.aaa;                                         \
    APPLY_MAPPING(v, m)


#define alpha_input_zero                                    0
#define alpha_input_one                                     1
#define alpha_input_one_half                                2
#define alpha_input_negative_one                            3
#define alpha_input_negative_one_half                       4
#define alpha_input_texture0_alpha                          5
#define alpha_input_texture1_alpha                          6
#define alpha_input_texture2_alpha                          7
#define alpha_input_texture3_alpha                          8
#define alpha_input_vertex_color0_alpha                     9
#define alpha_input_vertex_color1_alpha                     10
#define alpha_input_scratch0_alpha                          11
#define alpha_input_scratch1_alpha                          12
#define alpha_input_constant0_alpha                         13
#define alpha_input_constant1_alpha                         14
#define alpha_input_texture0_blue                           15
#define alpha_input_texture1_blue                           16
#define alpha_input_texture2_blue                           17
#define alpha_input_texture3_blue                           18
#define alpha_input_vertex_color0_blue                      19
#define alpha_input_vertex_color1_blue                      20
#define alpha_input_scratch0_blue                           21
#define alpha_input_scratch1_blue                           22
#define alpha_input_constant0_blue                          23
#define alpha_input_constant1_blue                          24

#define SELECT_ALPHA_INPUT(v, i, m)                         \
    if(i == alpha_input_zero)                               \
        v = 0;                                              \
    if(i == alpha_input_one)                                \
        v = 1;                                              \
    if(i == alpha_input_one_half)                           \
        v = 0.5;                                            \
    if(i == alpha_input_negative_one)                       \
        v = -1;                                             \
    if(i == alpha_input_negative_one_half)                  \
        v = -0.5;                                           \
    if(i == alpha_input_texture0_alpha)                     \
        v = t0.a;                                           \
    if(i == alpha_input_texture1_alpha)                     \
        v = t1.a;                                           \
    if(i == alpha_input_texture2_alpha)                     \
        v = t2.a;                                           \
    if(i == alpha_input_texture3_alpha)                     \
        v = t3.a;                                           \
    if(i == alpha_input_vertex_color0_alpha)                \
        v = v0.a;                                           \
    if(i == alpha_input_vertex_color1_alpha)                \
        v = v1.a;                                           \
    if(i == alpha_input_scratch0_alpha)                     \
        v = r0.a;                                           \
    if(i == alpha_input_scratch1_alpha)                     \
        v = r1.a;                                           \
    if(i == alpha_input_constant0_alpha)                    \
        v = c0.a;                                           \
    if(i == alpha_input_constant1_alpha)                    \
        v = c1.a;                                           \
    if(i == alpha_input_texture0_blue)                      \
        v = t0.b;                                           \
    if(i == alpha_input_texture1_blue)                      \
        v = t1.b;                                           \
    if(i == alpha_input_texture2_blue)                      \
        v = t2.b;                                           \
    if(i == alpha_input_texture3_blue)                      \
        v = t3.b;                                           \
    if(i == alpha_input_vertex_color0_blue)                 \
        v = v0.b;                                           \
    if(i == alpha_input_vertex_color1_blue)                 \
        v = v1.b;                                           \
    if(i == alpha_input_scratch0_blue)                      \
        v = r0.b;                                           \
    if(i == alpha_input_scratch1_blue)                      \
        v = r1.b;                                           \
    if(i == alpha_input_constant0_blue)                     \
        v = c0.b;                                           \
    if(i == alpha_input_constant1_blue)                     \
        v = c1.b;                                           \
    APPLY_MAPPING(v, m)


#define input_mapping_unsigned_identity                     0
#define input_mapping_unsigned_invert                       1
#define input_mapping_expand_normal                         2
#define input_mapping_expand_negate                         3
#define input_mapping_halfbias_normal                       4
#define input_mapping_halfbias_negate                       5
#define input_mapping_signed_identity                       6
#define input_mapping_signed_negate                         7

#define APPLY_MAPPING(v, m)                                 \
    if(input_mapping_unsigned_identity == m)                \
        v = max(v, 0.0);                                    \
    if(input_mapping_unsigned_invert == m)                  \
        v = 1.0 - clamp(v, 0.0, 1.0);                       \
    if(input_mapping_expand_normal == m)                    \
        v = 2.0 * max(v, 0.0) - 1.0;                        \
    if(input_mapping_expand_negate == m)                    \
        v = -2.0 * max(v, 0.0) + 1.0;                       \
    if(input_mapping_halfbias_normal == m)                  \
        v = max(v, 0.0) - 0.5;                              \
    if(input_mapping_halfbias_negate == m)                  \
        v = -max(v, 0.0) + 0.5;                             \
    if(input_mapping_signed_identity == m)                  \
        v = v;                                              \
    if(input_mapping_signed_negate == m)                    \
        v = -v;


#define stage_output_discard                                0
#define stage_output_scratch0                               1
#define stage_output_scratch1                               2
#define stage_output_vertex_color0                          3
#define stage_output_vertex_color1                          4
#define stage_output_texture0                               5
#define stage_output_texture1                               6
#define stage_output_texture2                               7
#define stage_output_texture3                               8

#define STAGE_COLOR_OUTPUT(v, o)                            \
    if(stage_output_scratch0 == o)                          \
        r0.rgb = v;                                         \
    if(stage_output_scratch1 == o)                          \
        r1.rgb = v;                                         \
    if(stage_output_vertex_color0 == o)                     \
        v0.rgb = v;                                         \
    if(stage_output_vertex_color1 == o)                     \
        v1.rgb = v;                                         \
    if(stage_output_texture0 == o)                          \
        t0.rgb = v;                                         \
    if(stage_output_texture1 == o)                          \
        t1.rgb = v;                                         \
    if(stage_output_texture2 == o)                          \
        t2.rgb = v;                                         \
    if(stage_output_texture3 == o)                          \
        t3.rgb = v;

#define STAGE_ALPHA_OUTPUT(v, o)                            \
    if(stage_output_scratch0 == o)                          \
        r0.a = v;                                           \
    if(stage_output_scratch1 == o)                          \
        r1.a = v;                                           \
    if(stage_output_vertex_color0 == o)                     \
        v0.a = v;                                           \
    if(stage_output_vertex_color1 == o)                     \
        v1.a = v;                                           \
    if(stage_output_texture0 == o)                          \
        t0.a = v;                                           \
    if(stage_output_texture1 == o)                          \
        t1.a = v;                                           \
    if(stage_output_texture2 == o)                          \
        t2.a = v;                                           \
    if(stage_output_texture3 == o)                          \
        t3.a = v;


#define output_function_multiply                            0
#define output_function_dot_product                         1

#define OUTPUT_FUNCTION(v, x, y, f)                         \
    if(output_function_multiply == f)                       \
        v = clamp(x * y, -1.0, 1.0);                        \
    if(output_function_dot_product == f)                    \
        v = dot(x, y);


#define output_mapping_identity                             0
#define output_mapping_scale_by_one_half                    1
#define output_mapping_scale_by_two                         2
#define output_mapping_scale_by_four                        3
#define output_mapping_bias                                 4
#define output_mapping_bias_and_scale_by_two                5

#define OUTPUT_MAPPING(v, m)                                \
    if(output_mapping_identity == m)                        \
        v = v;                                              \
    if(output_mapping_scale_by_one_half == m)               \
        v = v / 2.0;                                        \
    if(output_mapping_scale_by_two == m)                    \
        v = v * 2.0;                                        \
    if(output_mapping_scale_by_four == m)                   \
        v = v * 4.0;                                        \
    if(output_mapping_bias == m)                            \
        v = v - 0.5;                                        \
    if(output_mapping_bias_and_scale_by_two == m)           \
        v = (v - 0.5) * 2.0;

#define MUX(ab, cd) r0.a >= 0.5 ? cd : ab

#define STAGE(                                                                                  \
    stage_index,                                                                                \
    input_a, input_a_mapping,                                                                   \
    input_b, input_b_mapping,                                                                   \
    input_c, input_c_mapping,                                                                   \
    input_d, input_d_mapping,                                                                   \
                                                                                                \
    color_mux,                                                                                  \
    output_ab, output_ab_function,                                                              \
    output_cd, output_cd_function,                                                              \
    output_ab_cd_mux_sum,                                                                       \
    output_mapping_color,                                                                       \
                                                                                                \
    input_a_alpha, input_a_mapping_alpha,                                                       \
    input_b_alpha, input_b_mapping_alpha,                                                       \
    input_c_alpha, input_c_mapping_alpha,                                                       \
    input_d_alpha, input_d_mapping_alpha,                                                       \
                                                                                                \
    alpha_mux,                                                                                  \
    output_ab_alpha,                                                                            \
    output_cd_alpha,                                                                            \
    output_ab_cd_mux_sum_alpha,                                                                 \
    output_mapping_alpha,                                                                       \
                                                                                                \
    is_fog_stage                                                                                \
) {                                                                                             \
    c0 = stage_color0[stage_index];                                                             \
    c1 = stage_color1[stage_index];                                                             \
                                                                                                \
    if(!is_fog_stage || (is_fog_stage && fog_config.x > 0.5 && fog_config.y < 0.5)) {	        \
        SELECT_COLOR_INPUT(a, input_a, input_a_mapping)                                         \
        SELECT_COLOR_INPUT(b, input_b, input_b_mapping)                                         \
        SELECT_COLOR_INPUT(c, input_c, input_c_mapping)                                         \
        SELECT_COLOR_INPUT(d, input_d, input_d_mapping)                                         \
                                                                                                \
        OUTPUT_FUNCTION(ab, a, b, output_ab_function)                                           \
        OUTPUT_FUNCTION(cd, c, d, output_cd_function)                                           \
        ab_cd = color_mux ? MUX(ab, cd) : ab + cd;                                              \
        ab_cd = clamp(ab_cd, -1.0, 1.0);                                                        \
                                                                                                \
        OUTPUT_MAPPING(ab, output_mapping_color)                                                \
        OUTPUT_MAPPING(cd, output_mapping_color)                                                \
        OUTPUT_MAPPING(ab_cd, output_mapping_color)                                             \
                                                                                                \
        SELECT_ALPHA_INPUT(a_a, input_a_alpha, input_a_mapping_alpha)                           \
        SELECT_ALPHA_INPUT(b_a, input_b_alpha, input_b_mapping_alpha)                           \
        SELECT_ALPHA_INPUT(c_a, input_c_alpha, input_c_mapping_alpha)                           \
        SELECT_ALPHA_INPUT(d_a, input_d_alpha, input_d_mapping_alpha)                           \
                                                                                                \
        OUTPUT_FUNCTION(ab_a, a_a, b_a, output_function_multiply)                               \
        OUTPUT_FUNCTION(cd_a, c_a, d_a, output_function_multiply)                               \
        ab_cd_a = alpha_mux ? MUX(ab_a, cd_a) : ab_a + cd_a;                                    \
        ab_cd_a = clamp(ab_cd_a, -1.0, 1.0);                                                    \
                                                                                                \
        OUTPUT_MAPPING(ab_a, output_mapping_alpha)                                              \
        OUTPUT_MAPPING(cd_a, output_mapping_alpha)                                              \
        OUTPUT_MAPPING(ab_cd_a, output_mapping_alpha)                                           \
                                                                                                \
        STAGE_COLOR_OUTPUT(ab, output_ab)                                                       \
        STAGE_COLOR_OUTPUT(cd, output_cd)                                                       \
        STAGE_COLOR_OUTPUT(ab_cd, output_ab_cd_mux_sum)                                         \
        STAGE_ALPHA_OUTPUT(ab_a, output_ab_alpha)                                               \
        STAGE_ALPHA_OUTPUT(cd_a, output_cd_alpha)                                               \
        STAGE_ALPHA_OUTPUT(ab_cd_a, output_ab_cd_mux_sum_alpha)                                 \
    }	                                                                                        \
    else if(is_fog_stage && fog_config.x > 0.5 && fog_config.y > 0.5) {						    \
        SELECT_COLOR_INPUT(a, color_input_constant0_alpha,	input_mapping_unsigned_invert)		\
        SELECT_COLOR_INPUT(b, color_input_scratch0,			input_mapping_unsigned_identity)	\
        SELECT_COLOR_INPUT(c, color_input_constant0_alpha,	input_mapping_unsigned_identity)	\
        SELECT_COLOR_INPUT(d, color_input_constant0,		input_mapping_unsigned_identity)	\
                                                                                                \
        ab_cd = (a * b) + (c * d);																\
        ab_cd = clamp(ab_cd, -1.0, 1.0);														\
                                                                                                \
        STAGE_COLOR_OUTPUT(ab_cd, stage_output_scratch0)										\
    }	                                                                                        \
}

struct PSInput {
    float4 Pos : SV_POSITION;
    float4 D0 : COLOR0;
    float4 D1 : COLOR1;
    float4 T0 : TEXCOORD0;
    float4 T1 : TEXCOORD1;
    float4 T2 : TEXCOORD2;
    float4 T3 : TEXCOORD3;
};

half4 main(PSInput i) : COLOR {
    #ifdef FIRST_MAP_IS_CUBE
    float4 t0 = texCUBE(TS0, i.T0);
    #else
    float4 t0 = tex2Dproj(TS0, i.T0);
    #endif
    float4 t1 = tex2D(TS1, i.T1);
    float4 t2 = tex2D(TS2, i.T2);
    float4 t3 = tex2D(TS3, i.T3);
    float4 c0 = 0;
    float4 c1 = 0;

    float4 r0 = float4(0, 0, 0, t0.a);
    float4 r1 = 0;
    float4 v0 = i.D0;
    float4 v1 = i.D1;

    float3 a, b, c, d, ab, cd, ab_cd;
    float a_a, b_a, c_a, d_a, ab_a, cd_a, ab_cd_a;

    #ifdef S0_CONFIGURATION
    STAGE(S0_CONFIGURATION)
    #endif

    #ifdef S1_CONFIGURATION
    STAGE(S1_CONFIGURATION)
    #endif

    #ifdef S2_CONFIGURATION
    STAGE(S2_CONFIGURATION)
    #endif

    #ifdef S3_CONFIGURATION
    STAGE(S3_CONFIGURATION)
    #endif

    #ifdef S4_CONFIGURATION
    STAGE(S4_CONFIGURATION)
    #endif

    #ifdef S5_CONFIGURATION
    STAGE(S5_CONFIGURATION)
    #endif

    #ifdef S6_CONFIGURATION
    STAGE(S6_CONFIGURATION)
    #endif

    #ifdef S7_CONFIGURATION
    STAGE(S7_CONFIGURATION)
    #endif

    return r0;
};
