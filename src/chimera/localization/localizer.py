# SPDX-License-Identifier: GPL-3.0-only

import sys
import os

EASTER_EGGS = [2]

if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <localization-dir> <output>".format(sys.argv[0]))
        sys.exit(1)

    loc_dir = sys.argv[1]

    # Make sure the path given is valid
    if not os.path.isdir(loc_dir):
        eprint("Error: {} is not a valid directory".format(loc_dir))
        sys.exit(1)

    localization = []
    key_list = []

    while True:
        # Get the path and check if it is a file
        fpath = os.path.join(sys.argv[1], str(len(localization)))
        if not os.path.isfile(fpath):
            break

        # Open
        with open(fpath, "r") as f:
            localization_file = {}
            while True:
                line = f.readline()
                if not line:
                    break

                # Get the key
                key = ""
                for d in line:
                    if d.isspace():
                        break
                    key = key + d

                # If the key is empty, next line
                if len(key) == 0:
                    continue

                # Ignore any leading spaces after this
                line = line[len(key):]
                ignore_count = 0
                for d in line:
                    if not d.isspace():
                        break
                    ignore_count = ignore_count + 1
                line = line[ignore_count:]

                # If there is no value, warn
                if len(line) == 0:
                    eprint("Warning: Localization file {} has empty key {}".format(len(localization), key))

                # Remove trailing whitespaces at the end of the line to get the value
                ignore_count = 0
                for d in reversed(line):
                    if not d.isspace():
                        break
                    ignore_count = ignore_count + 1
                if ignore_count > 0:
                    line = line[:-ignore_count]

                # Add key to key list
                if key not in key_list:
                    key_list.append(key)

                if key not in localization_file:
                    localization_file[key] = line
                else:
                    eprint("Error: Localization file {} has duplicate key {}".format(len(localization), key))
                    exit(1)

        localization.append(localization_file)

    # Sort keys
    key_list.sort()

    # Merge everything
    merged_localization = {}
    for key in key_list:
        data = []
        for loc in range(len(localization)):
            if key in localization[loc]:
                data.append(localization[loc][key])
            else:
                if loc in EASTER_EGGS:
                    data.append(localization[0][key])
                else:
                    eprint("Warning: Localization file {} is missing key {}.".format(loc, key))
                    data.append("")
        merged_localization[key] = data

    # Make the output file
    with open(sys.argv[2], "w") as output:
        output.write("namespace Chimera {\n    static const char *LOCALIZATION_DATA[][CHIMERA_LANGUAGE_COUNT + 1] = {\n")
        for key in key_list:
            output.write("        {\n")
            output.write("            \"{}\",\n".format(key))
            for loc in merged_localization[key]:
                output.write("            \"{}\",\n".format(loc))
            output.write("        },\n")
        output.write("    };\n}\n")
        output.flush()
