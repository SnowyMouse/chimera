#include "decompress.hpp"
#include "memory.hpp"
#include <lzma.h>
#include <windows.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include "header.hpp"

namespace Chimera {
    void read_compressed_block(std::size_t decompressed_file_offset, std::size_t decompressed_file_size, std::byte *compressed_data, std::size_t compressed_data_size, std::mutex *mutex) {
        lzma_stream stream = LZMA_STREAM_INIT;
        stream.next_in = reinterpret_cast<const std::uint8_t *>(compressed_data);
        stream.avail_in = compressed_data_size;

        stream.next_out = reinterpret_cast<std::uint8_t *>(get_memory() + decompressed_file_offset);
        stream.avail_out = decompressed_file_size;
        if(lzma_auto_decoder(&stream, UINT64_MAX, 0) != LZMA_OK || lzma_code(&stream, LZMA_FINISH) != LZMA_STREAM_END) {
            // error
        }
        lzma_end(&stream);
        mutex->unlock();
    }

    #define MAX_COMPRESSED_BLOCKS 64
    #define DECOMPRESSION_MAX_THREADS 32
    #define MAX_COMPRESSION_MEMORY_USAGE (256 * 1024 * 1024)

    #define DURATION_SECONDS(duration) ((duration).count() / static_cast<double>(decltype(duration)::period::den))

    static bool show_benchmark = false;
    static std::size_t max_threads_ini = DECOMPRESSION_MAX_THREADS;

    void set_show_decompression_benchmark(bool show_it) noexcept {
        show_benchmark = show_it;
    }

    void set_decompression_threads(std::size_t threads) noexcept {
        max_threads_ini = threads;
    }

    std::size_t decompress_map_file(std::FILE *f, const void *header_data) {
        using clock = std::chrono::steady_clock;
        auto start = clock::now();

        const auto &header = *reinterpret_cast<const VapMap::VAPFileHeader *>(header_data);

        std::fseek(f, sizeof(header), SEEK_SET);

        VapMap::VAPBlockIndex blocks[MAX_COMPRESSED_BLOCKS] = {};
        std::mutex mutexes[DECOMPRESSION_MAX_THREADS];

        std::size_t block_count = header.vap.block_count;
        if(block_count > MAX_COMPRESSED_BLOCKS) {
            std::terminate();
        }
        std::fread(blocks, sizeof(blocks[0]) * block_count, 1, f);

        // Then see how many threads we can do it with
        std::size_t max_threads = std::thread::hardware_concurrency();
        if(max_threads > DECOMPRESSION_MAX_THREADS) {
            max_threads = DECOMPRESSION_MAX_THREADS;
        }
        if(max_threads > block_count) {
            max_threads = block_count;
        }
        if(max_threads == 0) {
            max_threads = 1;
        }
        if(max_threads > max_threads_ini) {
            max_threads = max_threads_ini;
        }

        // Check how much memory we can deal with per thread
        std::size_t largest_block_size = 1;
        if(block_count == 0) {
            std::size_t current_offset = std::ftell(f);
            std::fseek(f, 0, SEEK_END);
            largest_block_size = std::ftell(f) - sizeof(header);
            std::fseek(f, current_offset, SEEK_SET);
        }
        else {
            for(std::size_t i = 0; i < block_count; i++) {
                if(blocks[i].file_size > largest_block_size) {
                    largest_block_size = blocks[i].file_size;
                }
            }
            std::size_t max_threads_memory = MAX_COMPRESSION_MEMORY_USAGE / largest_block_size;
            if(max_threads_memory > 0 && max_threads > max_threads_memory) {
                max_threads = max_threads_memory;
            }
        }

        // Allocate decompression thread data
        std::byte *decompression_threads[DECOMPRESSION_MAX_THREADS];
        for(std::size_t i = 0; i < max_threads; i++) {
            decompression_threads[i] = reinterpret_cast<std::byte *>(VirtualAlloc(0, largest_block_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        }

        std::size_t loaded_map_size = sizeof(VapMap::VAPFileHeader);
        for(std::size_t i = 0; i < block_count || i < 1; i++) {
            // Find a thread
            std::size_t t = 0;
            for(;;) {
                bool thread_found = false;
                for(t = 0; t < max_threads; t++) {
                    if(mutexes[t].try_lock()) {
                        thread_found = true;
                        break;
                    }
                }
                if(thread_found) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }

            std::size_t compressed_offset;
            std::size_t compressed_size;
            std::size_t decompressed_size;

            if(block_count == 0) {
                compressed_offset = sizeof(header);
                compressed_size = largest_block_size;
                decompressed_size = header.vap.decompressed_file_size - sizeof(header);
            }
            else {
                auto &block = blocks[i];
                compressed_offset = block.file_offset;
                compressed_size = block.file_size;
                decompressed_size = block.decompressed_size;
            }

            // Read data
            auto *addr = decompression_threads[t];
            std::fseek(f, compressed_offset, SEEK_SET);
            std::fread(addr, compressed_size, 1, f);

            std::thread a(read_compressed_block, loaded_map_size, decompressed_size, addr, compressed_size, mutexes + t);
            a.detach();

            loaded_map_size += decompressed_size;
        }

        // Make sure everything is done
        for(auto &m : mutexes) {
            m.lock();
        }

        // Free anything used
        for(std::size_t i = 0; i < max_threads; i++) {
            VirtualFree(decompression_threads[i], largest_block_size, MEM_RELEASE);
        }

        auto finish = clock::now();

        // Show benchmark (if needed)
        if(show_benchmark) {
            double total_time = DURATION_SECONDS(finish - start);
            char decompression_benchmark[256];
            std::snprintf(
                decompression_benchmark,
                sizeof(decompression_benchmark),
                "Map name: %s\nLoad time: %.04f s\nMax threads: %u\nMax block size: %zu\nBlock count: %zu\n%-100s",
                header.name,
                total_time,
                max_threads,
                largest_block_size,
                block_count,
                ""
            );
            MessageBox(nullptr, decompression_benchmark, "Decompression Benchmark", MB_OK);
        }

        return loaded_map_size;
    }
}
