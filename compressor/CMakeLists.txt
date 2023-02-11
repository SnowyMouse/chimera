cmake_minimum_required(VERSION 3.12)

project("chimera-compress")

add_executable(chimera-compress
    main.cpp
)

target_link_libraries(chimera-compress zstd)
