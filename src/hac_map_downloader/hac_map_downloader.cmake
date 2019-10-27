# Copyright (c) 2019 Kavawuvi
#
# This code is licensed under the GNU GPL v3.0 or any later version
# See LICENSE for more information.

add_library(hac_map_downloader STATIC
    src/hac_map_downloader/hac_map_downloader.cpp
)

add_executable(hac_map_downloader_test
    src/hac_map_downloader/test/test.cpp
)

target_link_libraries(hac_map_downloader_test hac_map_downloader ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcurl.a ws2_32)
set_target_properties(hac_map_downloader_test PROPERTIES LINK_FLAGS "-m32 -static-libgcc -static-libstdc++ -static -lwinpthread")
