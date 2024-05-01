# SPDX-License-Identifier: GPL-3.0-only

# Define the source files
add_library(monolith MODULE
    src/monolith/monolith.c
    src/monolith/strings/strings.rc
)

# Set the name
set_target_properties(monolith PROPERTIES PREFIX "")
set_target_properties(monolith PROPERTIES OUTPUT_NAME "strings")

# lol
add_dependencies(monolith chimera)
set_target_properties(monolith PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
target_link_libraries(monolith chimera ${WINXP_COMPATIBILITY_LIBRARIES} shlwapi map_downloader local_curl ws2_32 local_zstd)
