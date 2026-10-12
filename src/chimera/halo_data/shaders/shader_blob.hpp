// SPDX-License-Identifier: GPL-3.0-only

#include <cstddef>

// Vertex Shaders
extern unsigned char vsh_collection[];
extern const size_t vsh_collection_size;

// We need to have seperate vs_3_0 versions to ensure shader model 3 compliance.
extern unsigned char vsh_transparent_generic[];
extern unsigned char vsh_transparent_generic_lit_m[];
extern unsigned char vsh_transparent_generic_m[];
extern unsigned char vsh_transparent_generic_object_centered[];
extern unsigned char vsh_transparent_generic_object_centered_m[];
extern unsigned char vsh_transparent_generic_reflection[];
extern unsigned char vsh_transparent_generic_reflection_m[];
extern unsigned char vsh_transparent_generic_screenspace[];
extern unsigned char vsh_transparent_generic_screenspace_m[];
extern unsigned char vsh_transparent_generic_viewer_centered[];
extern unsigned char vsh_transparent_generic_viewer_centered_m[];

extern const size_t vsh_transparent_generic_size;
extern const size_t vsh_transparent_generic_lit_m_size;
extern const size_t vsh_transparent_generic_m_size;
extern const size_t vsh_transparent_generic_object_centered_size;
extern const size_t vsh_transparent_generic_object_centered_m_size;
extern const size_t vsh_transparent_generic_reflection_size;
extern const size_t vsh_transparent_generic_reflection_m_size;
extern const size_t vsh_transparent_generic_screenspace_size;
extern const size_t vsh_transparent_generic_screenspace_m_size;
extern const size_t vsh_transparent_generic_viewer_centered_size;
extern const size_t vsh_transparent_generic_viewer_centered_m_size;

// D3DX Effects
extern unsigned char fx_collection[];
extern const size_t fx_collection_size;

// Effects Collection
extern unsigned char ce_effects_collection[];
extern const size_t ce_effects_collection_sze;

// Pixel Shaders
extern unsigned char white[];
extern const size_t white_size;

extern unsigned char white_1_1[];
extern const size_t white_1_1_size;

extern unsigned char hud_meters[];
extern const size_t hud_meters_size;

extern unsigned char shader_transparent_generic_source[];
extern const size_t shader_transparent_generic_source_size;

extern unsigned char shader_transparent_generic_2_0_source[];
extern const size_t shader_transparent_generic_2_0_source_size;

extern unsigned char fog[];
extern unsigned char fog_akill[];
extern unsigned char fog_screen[];
extern const size_t fog_size;
extern const size_t fog_akill_size;
extern const size_t fog_screen_size;

extern unsigned char black[];
extern const size_t black_size;

extern unsigned char eff_normal_tint_alpha_blend_z[];
extern const size_t eff_normal_tint_alpha_blend_z_size;

extern unsigned char eff_nlin_tint_add_z[];
extern unsigned char eff_nlin_tint_alpha_blend_z[];
extern unsigned char eff_nlin_tint_double_mul_z[];
extern unsigned char eff_nlin_tint_mul_add_z[];
extern unsigned char eff_nlin_tint_mul_z[];
extern unsigned char eff_nlin_tint_z[];
extern unsigned char eff_normal_tint_add_z[];
extern unsigned char eff_normal_tint_alpha_blend_z[];
extern unsigned char eff_normal_tint_double_mul_z[];
extern unsigned char eff_normal_tint_mul_add_z[];
extern unsigned char eff_normal_tint_mul_z[];
extern unsigned char eff_normal_tint_z[];

extern const size_t eff_nlin_tint_add_z_size;
extern const size_t eff_nlin_tint_alpha_blend_z_size;
extern const size_t eff_nlin_tint_double_mul_z_size;
extern const size_t eff_nlin_tint_mul_add_z_size;
extern const size_t eff_nlin_tint_mul_z_size;
extern const size_t eff_nlin_tint_z_size;
extern const size_t eff_normal_tint_add_z_size;
extern const size_t eff_normal_tint_alpha_blend_z_size;
extern const size_t eff_normal_tint_double_mul_z_size;
extern const size_t eff_normal_tint_mul_add_z_size;
extern const size_t eff_normal_tint_mul_z_size;
extern const size_t eff_normal_tint_z_size;

extern unsigned char decal_add[];
extern unsigned char decal_multiply[];
extern unsigned char decal_multiply2x[];
extern unsigned char decal_alpha_blend[];
extern unsigned char decal_alpha_madd[];

extern const size_t decal_add_size;
extern const size_t decal_multiply_size;
extern const size_t decal_multiply2x_size;
extern const size_t decal_alpha_blend_size;
extern const size_t decal_alpha_madd_size;

extern unsigned char transparent_chicago[];
extern const size_t transparent_chicago_size;

extern unsigned char transparent_chicago_cube[];
extern const size_t transparent_chicago_cube_size;
