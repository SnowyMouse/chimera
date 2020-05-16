# SPDX-License-Identifier: GPL-3.0-only

import sys
import os
import math

if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <colorfile> <output>".format(sys.argv[0]))
        sys.exit(1)

    codes = []
    with open(sys.argv[1]) as file:
        line_number = 0
        while True:
            line_number = line_number + 1
            line = file.readline()
            if not line:
                break
            if len(line) == 0:
                continue
            if line.startswith("##") or line[0].isspace():
                continue

            # Get the key code
            code = line[0]

            # Make sure we have more than just that
            if len(line) == 1:
                eprint("Invalid line {} on line {}".format(line, line_number))
                sys.exit(1)

            # Ignore spaces until we get something else
            line = line[1:]
            ignore_count = 0
            for c in line:
                if c.isspace():
                    ignore_count = ignore_count + 1
                    continue
                break
            line = line[ignore_count:]

            # Ignore spaces after the code
            ignore_count = 0
            for c in reversed(line):
                if c.isspace():
                    ignore_count = ignore_count + 1
                    continue
                break
            line = line[:-ignore_count]

            if len(line) < 6:
                eprint("Invalid code {} on line {}".format(line, line_number))
                sys.exit(1)

            # Convert to lowercase
            line = line.lower()

            # Get a color
            color = []

            # Check if it's xxxxxx (reset)
            if line == "xxxxxx":
                continue
            else:
                color_int = 0
                try:
                    color_int = int(line, 16)
                except ValueError:
                    eprint("Invalid code {} on line {}".format(line, line_number))
                    sys.exit(1)
                def rounded_hex(value):
                    return math.floor(value / 255.0 * 100.0 + 0.5) / 100.0
                color.append(rounded_hex((color_int & 0xFF0000) >> 16))
                color.append(rounded_hex((color_int & 0xFF00) >> 8))
                color.append(rounded_hex(color_int & 0xFF))

            # Escape the code if needed
            if code == "\\" or code == "'":
                code = "\\" + code

            # Add to the list
            codes.append([code, color])

    # Write the codes
    with open(sys.argv[2], "w") as file:
        file.write("// DON'T EDIT THIS FILE\n// Edit color_codes and then regenerate this file with the color_codes_generator.py script\n\n")
        file.write("inline void color_for_code(char c, ColorARGB &color) {\n")
        file.write("    switch(c) {\n")
        for code in codes:
            file.write("    case \'{}\':\n".format(code[0]))
            file.write("        color = ColorARGB {{color.alpha, {}, {}, {}}};\n".format(code[1][0], code[1][1], code[1][2]))
            file.write("        break;\n")
        file.write("    default: break;\n")
        file.write("    }\n}")
