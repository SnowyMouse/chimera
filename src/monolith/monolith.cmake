# Copyright (c) 2018 Kavawuvi
#
# This code is licensed under the GNU GPL v3.0 or any later version
# See LICENSE for more information.

# Define the source files
add_library(monolith MODULE
    src/monolith/monolith.c
    src/monolith/version.rc
)

# Set the name
set_target_properties(monolith PROPERTIES PREFIX "")
set_target_properties(monolith PROPERTIES OUTPUT_NAME "strings")

# lol
set_target_properties(monolith PROPERTIES COMPILE_FLAGS "-m32 -fno-stack-protector")
set_target_properties(monolith PROPERTIES LINK_FLAGS "-m32 -nostdlib")
target_link_libraries(monolith shlwapi)
