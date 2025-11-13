# SPDX-License-Identifier: GPL-3.0-only

import sys
import os

def generate_shader_blobs(name, binary):
    shaders = open(binary, "rb")
    collection_size = 0
    binary_size = os.path.getsize(binary)

    collection = open(sys.argv[2] + "shader_transparent_generic_blobs.cpp", "a")
    collection_size = os.path.getsize(sys.argv[2] + "shader_transparent_generic_blobs.cpp")

    # write the include if file is new
    if collection_size == 0:
        collection.write("#include <cstddef>\n" + "#include " + '"' + sys.argv[1] + 'shader_transparent_generic_blobs.hpp"')
        collection.write("\n\n")
        collection.write("namespace Chimera {\n")
        collection.write("unsigned char *generic_blobs[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS];\n")
        collection.write("std::uint32_t generic_crc[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS];\n")

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
    if os.path.exists(sys.argv[2] + "shader_transparent_generic_blobs.cpp"):
        os.remove(sys.argv[2] + "shader_transparent_generic_blobs.cpp")


    # Create new empty cpp files
    with open(sys.argv[2] + "shader_transparent_generic_blobs.cpp", "w") as fp:
        pass

    # Pixel Shaders
    for filename in os.listdir(sys.argv[1] + "pixel/generic_blobs"):
        generate_shader_blobs("blob_" + filename, sys.argv[1] + "pixel/generic_blobs/" + filename)

    collection = open(sys.argv[2] + "shader_transparent_generic_blobs.cpp", "a")

    collection.write("\n\n  void preload_transparent_generic_blobs() noexcept {\n")

    i = 0
    for filename in os.listdir(sys.argv[1] + "pixel/generic_blobs"):
        collection.write("generic_blobs[" + str(i) + "] = blob_" + filename + ";\n")
        collection.write("generic_crc[" + str(i) + "] = " + filename + ";\n\n")
        i = i + 1

    collection.write("}\n}")
    collection.close()
