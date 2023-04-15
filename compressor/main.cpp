/*
 * chimera-compress (c) Snowy Mouse
 *
 * Compresses/decompresses a Halo: Combat Evolved cache file for use with Chimera and other compatible map readers.
 *
 * Requires zstandard @ http://facebook.github.io/zstd/
 *
 * This software is licensed under version 3 of the GNU GPL (https://www.gnu.org/licenses/gpl-3.0.en.html)
 */

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <cstring>

#include <zstd.h>

static constexpr const std::uint32_t DEMO_HEAD = 'Ehed';
static constexpr const std::uint32_t DEMO_FOOT = 'Gfot';

static constexpr const std::uint32_t HEAD = 'head';
static constexpr const std::uint32_t FOOT = 'foot';

int main(int argc, const char **argv) {
    if(argc != 4 && argc != 3) {
        std::printf("Usage: %s <compress/decompress> <map>\n", argv[0]);
        std::printf("       %s <compress/decompress> <input-map> <output-map>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // What are we doing?
    bool compress;
    if(std::strcmp(argv[1], "compress") == 0) {
        compress = true;
    }
    else if(std::strcmp(argv[1], "decompress") == 0) {
        compress = false;
    }
    else {
        std::fprintf(stderr, "%s must be 'compress' or 'decompress'\n", argv[1]);
        return EXIT_FAILURE;
    }

    std::filesystem::path input_path = argv[2];
    std::filesystem::path output_path = argc == 3 ? argv[2] : argv[3];

    // Open
    auto *input_f = std::fopen(input_path.string().c_str(), "rb");
    if(input_f == nullptr) {
        std::fprintf(stderr, "Can't open input %s\n", input_path.string().c_str());

        // Does it exist??
        std::error_code ec;
        if(!std::filesystem::is_regular_file(input_path, ec)) {
            std::fprintf(stderr, "Input does not appear to be a file?\n");
        }

        return EXIT_FAILURE;
    }

    // Get the size
    std::fseek(input_f, 0, SEEK_END);
    std::vector<std::byte> data_to_read(std::ftell(input_f));
    auto *data_to_read_data = data_to_read.data();
    auto data_to_read_size = data_to_read.size();
    std::fseek(input_f, 0, SEEK_SET);

    // Read it
    auto read_result = std::fread(data_to_read_data, data_to_read_size, 1, input_f);
    std::fclose(input_f);
    if(read_result != 1) {
        std::fprintf(stderr, "Can't read %s\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Is there even a complete header?
    if(data_to_read_size < 0x800) {
        std::fprintf(stderr, "%s is not a cache file\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    auto head_foot_valid = *reinterpret_cast<std::uint32_t *>(data_to_read_data) == HEAD && *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x7FC) == FOOT;
    auto head_foot_valid_demo = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x2C0) == DEMO_HEAD && *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x5F0) == DEMO_FOOT;

    // old compressed map may have this.
    auto head_foot_valid_demo_retail_fourcc = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x2C0) == HEAD && *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x5F0) == FOOT;

    // Is it valid?
    if(!head_foot_valid && !head_foot_valid_demo && !head_foot_valid_demo_retail_fourcc) {
        std::fprintf(stderr, "%s does not appear to be a valid cache file\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Get the cache version
    std::uint32_t cache_version, uncompressed_size, tag_data_offset, tag_data_length, map_type, lua_script_data, lua_script_size, crc32;
    const char *name, *build;
    if(head_foot_valid) {
        cache_version = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x4);
        uncompressed_size = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x8);
        tag_data_offset = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x10);
        tag_data_length = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x14);
        name = reinterpret_cast<const char *>(data_to_read_data + 0x20);
        build = reinterpret_cast<const char *>(data_to_read_data + 0x40);
        map_type = *reinterpret_cast<std::uint16_t *>(data_to_read_data + 0x60);
        lua_script_data = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x310);
        lua_script_size = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x314);
        crc32 = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x64);
    }
    else {
        cache_version = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x588);
        uncompressed_size = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x5E8);
        tag_data_offset = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x5EC);
        tag_data_length = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x2C4);
        name = reinterpret_cast<const char *>(data_to_read_data + 0x58C);
        build = reinterpret_cast<const char *>(data_to_read_data + 0x2C8);
        map_type = *reinterpret_cast<std::uint16_t *>(data_to_read_data + 0x2);
        lua_script_data = 0; // Unsupported
        lua_script_size = 0; // Unsupported
        crc32 = *reinterpret_cast<std::uint32_t *>(data_to_read_data + 0x5B0);
    }

    // The uncompressed size in the header is bullshit
    if(compress) {
        uncompressed_size = data_to_read.size();
    }

    // Are we too big?
    if(data_to_read.size() > UINT32_MAX / 2 || uncompressed_size > UINT32_MAX / 2) {
        std::fprintf(stderr, "%s uncompressed is too large (>= 2 GiB)\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Are we too small?
    if(uncompressed_size <= 0x800) {
        std::fprintf(stderr, "%s has an invalid uncompressed size and its real uncompressed size cannot be determined\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Can we handle it?
    bool use_demo_header = false;
    std::uint32_t head = HEAD;
    std::uint32_t foot = FOOT;
    switch(cache_version) {
        case 6:
            use_demo_header = true;
            head = DEMO_HEAD;
            foot = DEMO_FOOT;
            [[fallthrough]];
        case 7:
        case 609:
            if(!compress) {
                std::fprintf(stderr, "%s does not appear to be compressed\n", input_path.string().c_str());
                return EXIT_FAILURE;
            }
            break;
        case 0x861A0006:
            use_demo_header = true;
            head = DEMO_HEAD;
            foot = DEMO_FOOT;
            [[fallthrough]];
        case 0x861A0007:
        case 0x861A0261:
            if(compress) {
                std::fprintf(stderr, "%s appears to be compressed\n", input_path.string().c_str());
                return EXIT_FAILURE;
            }
            break;
        default:
            std::fprintf(stderr, "%s has an unknown version %llu\n", input_path.string().c_str(), static_cast<unsigned long long>(cache_version));
            return EXIT_FAILURE;
    }

    std::vector<std::byte> file_to_write;

    if(compress) {
        file_to_write.resize(ZSTD_compressBound(uncompressed_size));

        std::byte *compressed_data = file_to_write.data() + 0x800;
        std::size_t compressed_size = file_to_write.size() - 0x800;
        const std::byte *decompressed_data = data_to_read_data + 0x800;
        std::size_t decompressed_size = data_to_read_size - 0x800;

        std::size_t actual_compressed_length = ZSTD_compress(compressed_data, compressed_size, decompressed_data, decompressed_size, 19);

        if(ZSTD_isError(actual_compressed_length)) {
            std::fprintf(stderr, "%s could not be compressed\n", input_path.string().c_str());
            return EXIT_FAILURE;
        }

        std::printf("%s: Compressed %zu bytes --> %zu bytes\n", input_path.string().c_str(), decompressed_size + 0x800, actual_compressed_length + 0x800);

        cache_version |= 0x861A0000;

        file_to_write.resize(actual_compressed_length + 0x800);
    }
    else {
        const std::byte *compressed_data = data_to_read_data + 0x800;
        std::size_t compressed_size = data_to_read_size - 0x800;

        file_to_write.resize(uncompressed_size);
        std::byte *decompressed_data = file_to_write.data() + 0x800;
        std::size_t decompressed_size = file_to_write.size() - 0x800;

        if(ZSTD_decompress(decompressed_data, decompressed_size, compressed_data, compressed_size) != decompressed_size) {
            std::fprintf(stderr, "%s could not be decompressed\n", input_path.string().c_str());
            return EXIT_FAILURE;
        }

        std::printf("%s: Decompressed %zu bytes --> %zu bytes\n", input_path.string().c_str(), compressed_size + 0x800, decompressed_size + 0x800);

        cache_version &= ~0x861A0000;
    }

    auto *file_to_write_data = file_to_write.data();
    if(!use_demo_header) {
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x0) = head;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x4) = cache_version;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x8) = uncompressed_size;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x10) = tag_data_offset;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x14) = tag_data_length;
        std::strncpy(reinterpret_cast<char *>(file_to_write_data + 0x20), name, 0x20);
        std::strncpy(reinterpret_cast<char *>(file_to_write_data + 0x40), build, 0x20);
        *reinterpret_cast<std::uint16_t *>(file_to_write_data + 0x60) = map_type;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x64) = crc32;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x310) = lua_script_data;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x314) = lua_script_size;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x7FC) = foot;
    }
    else {
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x2C0) = head;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x588) = cache_version;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x5E8) = uncompressed_size;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x5EC) = tag_data_offset;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x2C4) = tag_data_length;
        std::strncpy(reinterpret_cast<char *>(file_to_write_data + 0x58C), name, 0x20);
        std::strncpy(reinterpret_cast<char *>(file_to_write_data + 0x2C8), build, 0x20);
        *reinterpret_cast<std::uint16_t *>(file_to_write_data + 0x2) = map_type;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x5B0) = crc32;
        *reinterpret_cast<std::uint32_t *>(file_to_write_data + 0x5F0) = foot;
    }

    // Write it
    auto *output_f = std::fopen(output_path.string().c_str(), "wb");
    if(!output_f) {
        std::fprintf(stderr, "Failed to open output %s for writing\n", output_path.string().c_str());
        return EXIT_FAILURE;
    }
    auto result = std::fwrite(file_to_write.data(), file_to_write.size(), 1, output_f);
    std::fclose(output_f);
    if(result != 1) {
        std::fprintf(stderr, "Failed to write to output %s\n", output_path.string().c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
