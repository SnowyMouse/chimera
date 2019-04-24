#!/bin/python3

import sys
import os

def make_a8_hex(i):
    return "0x{:02X}{:02X}{:02X}{:02X}".format(i,255,255,255)
def make_y8_hex(i):
    return "0x{:02X}{:02X}{:02X}{:02X}".format(255,i,i,i)
def make_ay8_hex(i):
    return "0x{:02X}{:02X}{:02X}{:02X}".format(i,i,i,i)
def make_a8y8_hex(i,k):
    return "0x{:02X}{:02X}{:02X}{:02X}".format(i,k,k,k)

def generate_tables():
    output = ""

    # Make the a8 table
    output = output + "const static std::uint32_t lookup_a8[] = {\n"
    for i in range(0,64):
        output = output + "    {}, {}, {}, {},\n".format(make_a8_hex(i*4), make_a8_hex(i*4+1), make_a8_hex(i*4+2), make_a8_hex(i*4+3))
    output = output + "};\n"

    # Make the y8 table
    output = output + "const static std::uint32_t lookup_y8[] = {\n"
    for i in range(0,64):
        output = output + "    {}, {}, {}, {},\n".format(make_y8_hex(i*4), make_y8_hex(i*4+1), make_y8_hex(i*4+2), make_y8_hex(i*4+3))
    output = output + "};\n"

    # Make the ay8 table
    output = output + "const static std::uint32_t lookup_ay8[] = {\n"
    for i in range(0,64):
        output = output + "    {}, {}, {}, {},\n".format(make_ay8_hex(i*4), make_ay8_hex(i*4+1), make_ay8_hex(i*4+2), make_ay8_hex(i*4+3))
    output = output + "};\n"

    # Make the a8y8 table
    output = output + "const static std::uint32_t lookup_a8y8[] = {\n"
    for i in range(0,256):
        for j in range(0,64):
            output = output + "    {}, {}, {}, {},\n".format(make_a8y8_hex(i,j*4), make_a8y8_hex(i,j*4+1), make_a8y8_hex(i,j*4+2), make_a8y8_hex(i,j*4+3))
    output = output + "};\n"

    return output

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Syntax: {} <output>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
    with open(sys.argv[1], "w") as output:
        output.write("namespace Chimera {{\n{}\n}}\n".format(generate_tables()))
