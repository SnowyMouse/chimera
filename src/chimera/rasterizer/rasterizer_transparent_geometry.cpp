// SPDX-License-Identifier: GPL-3.0-only

#include "rasterizer_transparent_geometry.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/game_variables.hpp"


namespace Chimera {

    extern DynamicVertices *dynamic_vertices;

    short rasterizer_dynamic_vertices_get_type(long dynamic_vertex_buffer_index) noexcept {
        short vertex_type = -1;
        if(dynamic_vertex_buffer_index != -1) {
            DynamicVertexBuffer *dynamic_vertex_buffer;

            throw_error(dynamic_vertex_buffer_index >= 0, "dynamic vertex buffer index invalid");
            throw_error(dynamic_vertex_buffer_index < dynamic_vertices->buffer_count, "dynamic vertex buffer index out of bounds");

            dynamic_vertex_buffer = &dynamic_vertices->buffers[dynamic_vertex_buffer_index];
            vertex_type = dynamic_vertex_buffer->type;
        }
        else {
            show_error_box("Error", "Invalid dynamic vertices");
            std::abort();
        }

        return vertex_type;
    }

    short rasterizer_transparent_geometry_get_primary_vertex_type(TransparentGeometryGroup *group) noexcept {
        short vertex_type = -1;
        throw_error(group, "transparent geometry group bad pointer");

        if(group->vertex_buffers) {
            vertex_type = group->vertex_buffers[0].type;
        }
        else if(group->dynamic_vertex_buffer_index != -1) {
            vertex_type= rasterizer_dynamic_vertices_get_type(group->dynamic_vertex_buffer_index);
        }
        else {
            show_error_box("Error", "Invalid group");
            std::abort();
        }

        return vertex_type;
    }

}
