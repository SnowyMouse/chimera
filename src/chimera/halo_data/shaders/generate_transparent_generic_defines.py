# SPDX-License-Identifier: GPL-3.0-only

import sys
import os


if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <shader_defines> <output>".format(sys.argv[0]))
        sys.exit(1)

    # Delete the shader cpp files if they already exist
    if os.path.exists(sys.argv[2] + "shader_transparent_generic_defines.cpp"):
        os.remove(sys.argv[2] + "shader_transparent_generic_defines.cpp")

    shaders = open( sys.argv[1] + "/generic_defines/shader_defines.txt", "rt")
    collection = open(sys.argv[2] + "shader_transparent_generic_defines.cpp", "w")

    collection.write("#include <cstddef>\n" + "#include " + '"' + sys.argv[1] + 'shader_transparent_generic_defines.hpp"')
    collection.write("\n\n")
    collection.write("namespace Chimera {\n\n")
    collection.write("  D3D_SHADER_MACRO generic_defines[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADERS][10] ={NULL, NULL};\n\n")
    collection.write("  void preload_generic_shader_defines() noexcept {\n")

    i = 0
    line = shaders.readline()
    while line:
        j = 0
        while line != "\n":
            collection.write("      LPCSTR define_" + str(i) + "_name_" + str(j) + ' = { "' + line[:-1] + '" };\n')
            collection.write("      generic_defines[" + str(i) + "][" + str(j) + "].Name = define_" + str(i) + "_name_" + str(j) + ";\n")
            line = shaders.readline()
            if line != "\n":
                collection.write("      LPCSTR define_" + str(i) + "_def_" + str(j) + ' = { "' + line[:-1] + '" };\n')
                collection.write("      generic_defines[" + str(i) + "][" + str(j) + "].Definition = define_" + str(i) + "_def_" + str(j) + ";\n")
                line = shaders.readline()

            j = j+1

        i = i + 1
        line = shaders.readline()
        line = shaders.readline()

    collection.write("  }\n}")

    collection.close()
    shaders.close()





