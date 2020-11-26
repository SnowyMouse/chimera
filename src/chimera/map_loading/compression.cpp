// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstdio>
#include <filesystem>
#include <cstring>
#include <zstd.h>

#include "../halo_data/map.hpp"
#include "compression.hpp"

namespace Chimera {
    static void decompress_header(const std::byte *header_input, std::byte *header_output) {
        // Check to see if we can't even fit the header
        auto header_copy = *reinterpret_cast<const MapHeader *>(header_input);

        // Figure out the new engine version
        auto new_engine_version = header_copy.engine_type;
        bool invader_compression = false;
        bool stores_uncompressed_size;
        switch(header_copy.engine_type) {
            case CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                invader_compression = true;
                break;
            case CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = CacheFileEngine::CACHE_FILE_RETAIL;
                invader_compression = true;
                break;
            case CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = CacheFileEngine::CACHE_FILE_DEMO;
                invader_compression = true;
                break;
            default:
                throw std::exception();
        }

        // Determine if the file size isn't set correctly
        if(invader_compression && header_copy.file_size < sizeof(header_copy)) {
            throw std::exception();
        }

        // Set the file size to either the original decompressed size or 0 (if needed) and the engine to the new thing
        header_copy.file_size = stores_uncompressed_size ? header_copy.file_size : 0;
        header_copy.engine_type = new_engine_version;

        // if demo, convert the header, otherwise copy the header
        if(new_engine_version == CacheFileEngine::CACHE_FILE_DEMO) {
            auto &demo_header = *reinterpret_cast<MapHeaderDemo *>(header_output);
            demo_header = {};
            std::memcpy(demo_header.name, header_copy.name, sizeof(demo_header.name));
            std::memcpy(demo_header.build, header_copy.build, sizeof(demo_header.build));
            demo_header.engine_type = header_copy.engine_type;
            demo_header.tag_data_offset = header_copy.tag_data_offset;
            demo_header.tag_data_size = header_copy.tag_data_size;
            demo_header.game_type = header_copy.game_type;
            demo_header.crc32 = header_copy.crc32;
            demo_header.head = 0x45686564;
            demo_header.foot = 0x47666F74;
        }
        else {
            *reinterpret_cast<MapHeader *>(header_output) = header_copy;
        }
    }

    constexpr std::size_t HEADER_SIZE = sizeof(MapHeader);

    struct LowMemoryDecompression {
        /**
         * Callback for when a decompression occurs
         * @param decompressed_data decompressed data to write
         * @param size              size of decompressed data
         * @param user_data         user data to pass
         * @return                  true if successful
         */
        bool (*write_callback)(const std::byte *decompressed_data, std::size_t size, void *user_data) = nullptr;

        /**
         * Decompress the map file
         * @param path path to the map file
         */
        void decompress_map_file(const char *input, void *user_data) {
            // Open the input file
            std::FILE *input_file = std::fopen(input, "rb");
            if(!input_file) {
                throw std::exception();
            }

            // Get the size
            std::size_t total_size = std::filesystem::file_size(input);

            // Read the input file header
            MapHeader header_input;
            if(std::fread(&header_input, sizeof(header_input), 1, input_file) != 1) {
                std::fclose(input_file);
                throw std::exception();
            }

            // Make the output header and write it
            std::byte header_output[HEADER_SIZE];
            try {
                decompress_header(reinterpret_cast<std::byte *>(&header_input), header_output);
            }
            catch (std::exception &) {
                std::fclose(input_file);
                throw;
            }

            // Write the header
            if(!write_callback(header_output, sizeof(header_output), user_data)) {
                std::fclose(input_file);
                throw std::exception();
            }

            // Allocate and init a stream
            auto decompression_stream = ZSTD_createDStream();
            const std::size_t init = ZSTD_initDStream(decompression_stream);

            std::size_t total_read = HEADER_SIZE;
            auto read_data = [&input_file, &decompression_stream, &total_read](std::byte *where, std::size_t size) {
                if(std::fread(where, size, 1, input_file) != 1) {
                    std::fclose(input_file);
                    ZSTD_freeDStream(decompression_stream);
                    throw std::exception();
                }
                total_read += size;
            };

            auto &write_callback = this->write_callback;
            auto write_data = [&input_file, &write_callback, &decompression_stream, &user_data](const std::byte *where, std::size_t size) {
                if(!write_callback(where, size, user_data)) {
                    std::fclose(input_file);
                    ZSTD_freeDStream(decompression_stream);
                    throw std::exception();
                }
            };

            while(total_read < total_size) {
                // Make some input/output data thingy
                std::vector<std::byte> input_data(init);
                std::vector<std::byte> output_data(ZSTD_DStreamOutSize());

                // Read the first bit
                read_data(input_data.data(), input_data.size());

                for(;;) {
                    ZSTD_inBuffer_s input_buffer = {};
                    ZSTD_outBuffer_s output_buffer = {};
                    input_buffer.src = input_data.data();
                    input_buffer.size = input_data.size();
                    output_buffer.dst = output_data.data();
                    output_buffer.size = output_data.size();

                    // Get the output
                    std::size_t q = ZSTD_decompressStream(decompression_stream, &output_buffer, &input_buffer);
                    if(ZSTD_isError(q)) {
                        std::fclose(input_file);
                        ZSTD_freeDStream(decompression_stream);
                        throw std::exception();
                    }

                    // Write it
                    if(output_buffer.pos) {
                        write_data(reinterpret_cast<std::byte *>(output_buffer.dst), output_buffer.pos);
                    }

                    // If it's > 0, we need more data
                    if(q > 0) {
                        input_data.clear();
                        input_data.insert(input_data.end(), q, std::byte());
                        read_data(input_data.data(), q);
                    }
                    else {
                        break;
                    }
                }
            }

            // Close the stream and the files
            std::fclose(input_file);
            ZSTD_freeDStream(decompression_stream);
        }
    };

    std::size_t decompress_map_file(const char *input, const char *output) {
        struct OutputWriter {
            std::FILE *output_file;
            std::size_t output_position = 0;
        } output_writer = { std::fopen(output, "wb") };
 
        if(!output_writer.output_file) {
            std::fclose(output_writer.output_file);
            throw std::exception();
        }
 
        LowMemoryDecompression decomp;
        decomp.write_callback = [](const std::byte *decompressed_data, std::size_t size, void *user_data) -> bool {
            auto &output_writer = *reinterpret_cast<OutputWriter *>(user_data);
            output_writer.output_position += size;
            return std::fwrite(decompressed_data, size, 1, reinterpret_cast<std::FILE *>(output_writer.output_file));
        };
 
        try {
            decomp.decompress_map_file(input, &output_writer);
        }
        catch (std::exception &e) {
            std::fclose(output_writer.output_file);
            throw;
        }
        std::fclose(output_writer.output_file);
 
        return output_writer.output_position;
    }

    std::size_t decompress_map_file(const char *input, std::byte *output, std::size_t output_size) {
        struct OutputWriter {
            std::byte *output;
            std::size_t output_size;
            std::size_t output_position = 0;
        } output_writer = { output, output_size };
 
        LowMemoryDecompression decomp;
        decomp.write_callback = [](const std::byte *decompressed_data, std::size_t size, void *user_data) -> bool {
            OutputWriter &output_writer = *reinterpret_cast<OutputWriter *>(user_data);
            std::size_t new_position = output_writer.output_position + size;
            if(new_position > output_writer.output_size) {
                return false;
            }
            std::copy(decompressed_data, decompressed_data + size, output_writer.output + output_writer.output_position);
            output_writer.output_position = new_position;
            return true;
        };
 
        try {
            decomp.decompress_map_file(input, &output_writer);
        }
        catch (std::exception &e) {
            throw;
        }
 
        return output_writer.output_position;
    }
}