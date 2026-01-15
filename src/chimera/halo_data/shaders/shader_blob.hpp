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
extern unsigned char disabled_shader[];
extern const size_t disabled_shader_size;

extern unsigned char disabled_shader_1_1[];
extern const size_t disabled_shader_1_1_size;

extern unsigned char hud_meters[];
extern const size_t hud_meters_size;

extern unsigned char shader_transparent_generic_source[];
extern const size_t shader_transparent_generic_source_size;

extern unsigned char shader_transparent_generic_2_0_source[];
extern const size_t shader_transparent_generic_2_0_source_size;