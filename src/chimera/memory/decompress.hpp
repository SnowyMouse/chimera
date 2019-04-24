#ifndef CHIMERA_DECOMPRESS_HEADER_H
#define CHIMERA_DECOMPRESS_HEADER_H

#include <cstdint>
#include <cstddef>
#include <mutex>

namespace Chimera {
    /**
     * Read the LZMA compressed data
     * @param decompressed_file_offset offset of the file data when decompressed
     * @param decompressed_file_size   size of the file data when decompressed
     * @param compressed_data          address to hold compressed data
     * @param compressed_data_size     amount compressed data to read
     * @param mutex                    locked mutex to unlock when done
     */
    void read_compressed_block(std::size_t decompressed_file_offset, std::size_t decompressed_file_size, std::byte *compressed_data, std::size_t compressed_data_size, std::mutex *mutex);

    /**
     * Decompress the .vap map file
     * @param  f           File to decompress
     * @param  header_data Pointer to the header data to read from
     * @return             size of the map in bytes
     */
    std::size_t decompress_map_file(std::FILE *f, const void *header_data);

    /**
     * Set whether or not the decompression benchmark should be shown
     * @param show_it true if it should be shown
     */
    void set_show_decompression_benchmark(bool show_it) noexcept;

    /**
     * Set maximum decompression threads
     * @param threads max threads to decompress with
     */
    void set_decompression_threads(std::size_t threads) noexcept;
}

#endif
