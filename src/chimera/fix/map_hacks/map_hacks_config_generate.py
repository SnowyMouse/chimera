# SPDX-License-Identifier: GPL-3.0-only

import json
import sys
import os

if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <map_hacks_config.json> <output>".format(sys.argv[0]))
        sys.exit(1)

    # Delete the cpp file if it already exist
    if os.path.exists(sys.argv[2] + "map_hacks_config.cpp"):
        os.remove(sys.argv[2] + "map_hacks_config.cpp")

    # Create new empty cpp file
    fix_config = open(sys.argv[2] + "map_hacks_config.cpp", "w")
    fix_config.write("#include <cstring>\n")
    fix_config.write("#include " + '"' + sys.argv[1] + 'map_hacks.hpp"')

    fix_config.write("\n\n")

    fix_config.write("namespace Chimera {\n")
    fix_config.write("MapFixConfig chimera_fix_blacklist[] = {\n")

    f = open(sys.argv[1] + "map_hacks_config.json", "r")
    map_config = json.loads(f.read())
    count = 0
    for config_dict in map_config['per_map_config']:
        if "map_name" in config_dict:
            if "tags_checksum" in config_dict:
                fix_config.write("{ " + '"' + str(config_dict["map_name"]).lower() + '"' + " ,")
                fix_config.write(" " + str(config_dict["tags_checksum"]) + ", ")
            else:
                continue
        else:
            continue

        if "gearbox_chicago_multiply" in config_dict:
            fix_config.write(" " + str(config_dict["gearbox_chicago_multiply"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "gearbox_meters" in config_dict:
            fix_config.write(" " + str(config_dict["gearbox_meters"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "gearbox_multitexture_blend_modes" in config_dict:
            fix_config.write(" " + str(config_dict["gearbox_multitexture_blend_modes"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "alternate_bump_attenuation" in config_dict:
            fix_config.write(" " + str(config_dict["alternate_bump_attenuation"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "gearbox_bump_attenuation" in config_dict:
            fix_config.write(" " + str(config_dict["gearbox_bump_attenuation"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "invert_detail_after_reflection" in config_dict:
            fix_config.write(" " + str(config_dict["invert_detail_after_reflection"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "embedded_lua" in config_dict:
            fix_config.write(" " + str(config_dict["embedded_lua"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "hud_number_scale" in config_dict:
            fix_config.write(" " + str(config_dict["hud_number_scale"]).lower() + ", ")
        else:
            fix_config.write(" false, ")
        if "disable_bitmap_hud_scale_flags" in config_dict:
            fix_config.write(" " + str(config_dict["disable_bitmap_hud_scale_flags"]).lower() + "}, ")
        else:
            fix_config.write(" false }, ")

        fix_config.write("\n")
        count = count + 1

    fix_config.write(" }; \n\n")
    fix_config.write("std::uint32_t blacklist_map_count = " + str(count) + ";\n")
    fix_config.write("}\n")

    fix_config.close()
    f.close()
