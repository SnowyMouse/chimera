// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_VERTEX_DEFS_HPP
#define CHIMERA_VERTEX_DEFS_HPP

#include <cstddef>
#include <cstdint>
#include <d3d9.h>

#include "pad.hpp"
#include "tag.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {

     // Defs
    enum VertexTypes : std::uint16_t {
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_UNCOMPRESSED,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_COMPRESSED,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_LIGHTMAP_UNCOMPRESSED,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_LIGHTMAP_COMPRESSED,
        RASTERIZER_VERTEX_TYPE_MODEL_UNCOMPRESSED,
        RASTERIZER_VERTEX_TYPE_MODEL_COMPRESSED,
        RASTERIZER_VERTEX_TYPE_DYNAMIC_UNLIT,
        RASTERIZER_VERTEX_TYPE_DYNAMIC_LIT,
        RASTERIZER_VERTEX_TYPE_DYNAMIC_SCREEN,
        RASTERIZER_VERTEX_TYPE_DEBUG,
        RASTERIZER_VERTEX_TYPE_DECAL,
        RASTERIZER_VERTEX_TYPE_DETAIL_OBJECT,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_UNCOMPRESSED_FF,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_LIGHTMAP_UNCOMPRESSED_FF,
        RASTERIZER_VERTEX_TYPE_MODEL_UNCOMPRESSED_FF,
        RASTERIZER_VERTEX_TYPE_MODEL_PROCESSED,
        RASTERIZER_VERTEX_TYPE_UNLIT_ZSPRITE,
        RASTERIZER_VERTEX_TYPE_SCREEN_TRANSFORMED_LIT,
        RASTERIZER_VERTEX_TYPE_SCREEN_TRANSFORMED_LIT_SPECULAR,
        RASTERIZER_VERTEX_TYPE_ENVIRONMENT_SINGLE_STREAM_FF,
        NUMBER_OF_RASTERIZER_VERTEX_TYPES
    };

    enum VertexDeclarationIndex : std::uint16_t {
        VERTEX_DECLARATION_ENVIRONMENT_UNCOMPRESSED,
        VERTEX_DECLARATION_ENVIRONMENT_COMPRESSED,
        VERTEX_DECLARATION_ENVIRONMENT_LIGHTMAP_UNCOMPRESSED,
        VERTEX_DECLARATION_ENVIRONMENT_LIGHTMAP_COMPRESSED,
        VERTEX_DECLARATION_MODEL_UNCOMPRESSED,
        VERTEX_DECLARATION_MODEL_COMPRESSED,
        VERTEX_DECLARATION_UNLIT,
        VERTEX_DECLARATION_DYNAMIC_UNLIT,
        VERTEX_DECLARATION_DYNAMIC_SCREEN,
        VERTEX_DECLARATION_DEBUG,
        VERTEX_DECLARATION_DECAL,
        VERTEX_DECLARATION_DETAIL_OBJECT,
        VERTEX_DECLARATION_ENVIRONMENT_UNCOMPRESSED_FF,
        VERTEX_DECLARATION_ENVIRONMENT_LIGHTMAP_UNCOMPRESSED_FF,
        VERTEX_DECLARATION_MODEL_UNCOMPRESSED_FF,
        VERTEX_DECLARATION_MODEL_PROCESSED,
        VERTEX_DECLARATION_UNLIT_ZSPRITE,
        VERTEX_DECLARATION_SCREEN_TRANSFORMED_LIT,
        VERTEX_DECLARATION_SCREEN_TRANSFORMED_LIT_SPECULAR,
        VERTEX_DECLARATION_ENVIRONMENT_SINGLE_STREAM_FF,
        NUM_OF_VERTEX_DECLARATIONS,
    };

    struct DynamicVertexBuffer {
        short type;
        PAD(0x2);
        long vertex_start_index;
        long vertex_count;
        void *data;
    };
    static_assert(sizeof(DynamicVertexBuffer) == 0x10);
    
    struct DynamicVertexBufferGroup {
        long vertex_count;
        long max_vertex_count;
        IDirect3DVertexBuffer9 *d3d_vertex_buffer;
    };
    static_assert(sizeof(DynamicVertexBufferGroup) == 0xC);

    struct DynamicVertices {
        DynamicVertexBufferGroup groups[NUMBER_OF_RASTERIZER_VERTEX_TYPES];
        DynamicVertexBuffer buffers[1024];
        long buffer_count;
    };

    struct VertexBuffer {
        short type;
        std::uint16_t pad;
        
        long count;
        long offset;
        
        void *base_address;
        void *hardware_format;
    };
    static_assert(sizeof(VertexBuffer) == 0x14);

    struct VertexDeclaration {
        IDirect3DVertexDeclaration9 *declaration;
        std::uint32_t fvf;
        std::uint32_t vertex_processing_method;
    };
    static_assert(sizeof(VertexDeclaration) == 0xC);

    struct VertexShader {
        IDirect3DVertexShader9 *shader;
        const char *something;
    };
    static_assert(sizeof(VertexShader) == 0x8);

}

#endif
