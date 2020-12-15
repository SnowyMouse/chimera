# SPDX-License-Identifier: GPL-3.0-only

# Define the source files
add_library(monolith MODULE
    src/monolith/monolith.c
    src/chimera/version.rc
)

# Set the name
set_target_properties(monolith PROPERTIES PREFIX "")
set_target_properties(monolith PROPERTIES OUTPUT_NAME "strings")

# lol
set_target_properties(monolith PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
target_link_libraries(monolith chimera shlwapi hac_map_downloader ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl/lib/libcurl.a ws2_32 ${CMAKE_CURRENT_SOURCE_DIR}/ext/zstd/lib/libzstd.a)
