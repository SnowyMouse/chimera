# SPDX-License-Identifier: GPL-3.0-only

import sys
import os

def generate_shader_blobs(name, binary, type):
    shaders = open(binary, "rb")
    collection_size = 0
    binary_size = os.path.getsize(binary)

    if type == 0:
        collection = open(sys.argv[2] + "d3dx_effects.cpp", "a")
        collection_size = os.path.getsize(sys.argv[2] + "d3dx_effects.cpp")
    elif type == 1:
        collection = open(sys.argv[2] + "effects_collection.cpp", "a")
        collection_size = os.path.getsize(sys.argv[2] + "effects_collection.cpp")
    elif type == 2:
        collection = open(sys.argv[2] + "vertex_shaders.cpp", "a")
        collection_size = os.path.getsize(sys.argv[2] + "vertex_shaders.cpp")
    elif type == 3:
        collection = open(sys.argv[2] + "pixel_shaders.cpp", "a")
        collection_size = os.path.getsize(sys.argv[2] + "pixel_shaders.cpp")

    # write the include if file is new
    if collection_size == 0:
        collection.write("#include <cstddef>\n" + "#include " + '"' + sys.argv[1] + 'shader_blob.hpp"')

    collection.write("\n\n")
    
    collection.write("const size_t " + name +"_size = " + str(binary_size) + ";\n\n")
    collection.write("unsigned char " + name +"[" + str(binary_size) + "] = {\n")

    i = 0
    byte = shaders.read(1)
    while byte:
        i = i+1
        if i != 1:
            collection.write(", ")
        collection.write("0x" + byte.hex())
        if i == 20:
            collection.write(", \n")
            i = 0
        byte = shaders.read(1)
    
    collection.write("\n" + "};" + "\n")

    shaders.close()
    collection.close()

if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <shader_files> <output>".format(sys.argv[0]))
        sys.exit(1)

    # Delete the shader cpp files if they already exist
    if os.path.exists(sys.argv[2] + "d3dx_effects.cpp"):
        os.remove(sys.argv[2] + "d3dx_effects.cpp")
    if os.path.exists(sys.argv[2] + "effects_collection.cpp"):
        os.remove(sys.argv[2] + "effects_collection.cpp")
    if os.path.exists(sys.argv[2] + "vertex_shaders.cpp"):
        os.remove(sys.argv[2] + "vertex_shaders.cpp")
    if os.path.exists(sys.argv[2] + "pixel_shaders.cpp"):
        os.remove(sys.argv[2] + "pixel_shaders.cpp")

    # Create new empty cpp files
    with open(sys.argv[2] + "d3dx_effects.cpp", "w") as fp:
        pass
    with open(sys.argv[2] + "effects_collection.cpp", "w") as fp:
        pass
    with open(sys.argv[2] + "vertex_shaders.cpp", "w") as fp:
        pass
    with open(sys.argv[2] + "pixel_shaders.cpp", "w") as fp:
        pass

    # Retail effects collection
    generate_shader_blobs("fx_collection", sys.argv[1] + "fx/fx.bin", 0)

    # Custom edition pixel shader collection
    generate_shader_blobs("ce_effects_collection", sys.argv[1] + "fx/EffectCollection_ps_2_0.bin", 1)

    # Vertex Shaders
    generate_shader_blobs("vsh_collection", sys.argv[1] + "vertex/vsh.bin", 2)

    generate_shader_blobs("vsh_transparent_generic", sys.argv[1] + "vertex/transparent_generic.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_lit_m", sys.argv[1] + "vertex/transparent_generic_lit_m.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_m", sys.argv[1] + "vertex/transparent_generic_m.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_object_centered", sys.argv[1] + "vertex/transparent_generic_object_centered.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_object_centered_m", sys.argv[1] + "vertex/transparent_generic_object_centered_m.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_reflection", sys.argv[1] + "vertex/transparent_generic_reflection.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_reflection_m", sys.argv[1] + "vertex/transparent_generic_reflection_m.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_screenspace", sys.argv[1] + "vertex/transparent_generic_screenspace.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_screenspace_m", sys.argv[1] + "vertex/transparent_generic_screenspace_m.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_viewer_centered", sys.argv[1] + "vertex/transparent_generic_viewer_centered.cso", 2)
    generate_shader_blobs("vsh_transparent_generic_viewer_centered_m", sys.argv[1] + "vertex/transparent_generic_viewer_centered_m.cso", 2)

    # Pixel Shaders
    generate_shader_blobs("shader_transparent_generic_source", sys.argv[1] + "pixel/shader_transparent_generic.hlsl", 3)
    generate_shader_blobs("shader_transparent_generic_2_0_source", sys.argv[1] + "pixel/shader_transparent_generic_2_0.hlsl", 3)

    generate_shader_blobs("disabled_shader", sys.argv[1] + "pixel/disabled.cso", 3)
    generate_shader_blobs("disabled_shader_1_1", sys.argv[1] + "pixel/disabled_1_1.cso", 3)
    generate_shader_blobs("hud_meters", sys.argv[1] + "pixel/hud_meters.cso", 3)