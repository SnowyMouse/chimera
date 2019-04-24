#ifndef CHIMERA_MEMORY_BITMAP_HPP
#define CHIMERA_MEMORY_BITMAP_HPP

#include <cstdint>
#include <cstddef>
#include <mutex>

namespace Chimera {
    enum ReEncode {
        RE_ENCODE_A8_TO_32_BIT = 0,
        RE_ENCODE_Y8_TO_32_BIT = 1,
        RE_ENCODE_AY8_TO_32_BIT = 2,
        RE_ENCODE_A8Y8_TO_32_BIT = 3,
        RE_ENCODE_P8_TO_32_BIT = 17
    };

    /**
     * Re-encode bitmap data to something Halo understands
     * @param  data_offset   offset in memory to write to
     * @param  memory_offset offset in memory to read from
     * @param  memory_size   amount of data in bytes to read
     * @param  encoding      encoding conversion to use
     */
    void write_encoded_chunk_data_from_memory(std::size_t data_offset, std::size_t memory_offset, std::size_t memory_size, ReEncode encoding);
}

#endif
