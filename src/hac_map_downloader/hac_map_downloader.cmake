# SPDX-License-Identifier: GPL-3.0-only

add_library(hac_map_downloader STATIC
    src/hac_map_downloader/hac_map_downloader.cpp
)

# Target this
target_include_directories(hac_map_downloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl/include)

add_executable(hac_map_downloader_test
    src/hac_map_downloader/test/test.cpp
)

target_link_libraries(hac_map_downloader_test hac_map_downloader ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl/lib/libcurl.a ws2_32)
set_target_properties(hac_map_downloader_test PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
