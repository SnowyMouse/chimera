#include <windows.h>
#include "bitmap.hpp"
#include "memory.hpp"

#include "lookup_p8.hpp"
#include <color_lookup_table.hpp>

namespace Chimera {
    void write_encoded_chunk_data_from_memory(std::size_t data_offset, std::size_t memory_offset, std::size_t memory_size, ReEncode encoding) {
        // Get how big a pixel is
        std::size_t size_per_pixel;
        if(encoding == ReEncode::RE_ENCODE_A8Y8_TO_32_BIT) {
            size_per_pixel = 2;
        }
        else {
            size_per_pixel = 1;
        }

        // Pick the right lookup table
        const std::uint32_t *lookup_table;
        switch(encoding) {
            case ReEncode::RE_ENCODE_A8_TO_32_BIT:
                lookup_table = lookup_a8;
                break;
            case ReEncode::RE_ENCODE_Y8_TO_32_BIT:
                lookup_table = lookup_y8;
                break;
            case ReEncode::RE_ENCODE_AY8_TO_32_BIT:
                lookup_table = lookup_ay8;
                break;
            case ReEncode::RE_ENCODE_A8Y8_TO_32_BIT:
                lookup_table = lookup_a8y8;
                break;
            case ReEncode::RE_ENCODE_P8_TO_32_BIT:
                lookup_table = lookup_p8;
                break;
            default:
                lookup_table = lookup_ay8;
                break;
        }

        // Get the address to the first pixel we're read/writing to
        std::uint32_t *pixel_to_write = reinterpret_cast<std::uint32_t *>(get_memory() + data_offset);
        std::byte *pixel_to_read = get_memory() + memory_offset;

        // Make a function for writing stuff
        auto write_stuff = [&pixel_to_write, &lookup_table, &memory_size](const auto *from) {
            for(std::size_t i = 0; i < memory_size / sizeof(*from); i++) {
                pixel_to_write[i] = lookup_table[from[i]];
            }
        };

        // Depending on the size of pixel, we use 8-bit or 16-bit integers
        if(size_per_pixel == 1) {
            write_stuff(reinterpret_cast<std::uint8_t *>(pixel_to_read));
        }
        else if(size_per_pixel == 2) {
            write_stuff(reinterpret_cast<std::uint16_t *>(pixel_to_read));
        }
    }
}
