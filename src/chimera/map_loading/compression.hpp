// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MAP_COMPRESSION_HPP
#define CHIMERA_MAP_COMPRESSION_HPP

#include <cstddef>

namespace Chimera {
    std::size_t decompress_map_file(const char *input, const char *output);
    std::size_t decompress_map_file(const char *input, std::byte *output, std::size_t output_size);
}

#endif