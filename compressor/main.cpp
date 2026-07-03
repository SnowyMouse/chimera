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
#include <utility>
#include <vector>
#include <cstring>

#include <zstd.h>

#define HEADER_SIZE 2048
#define COMPRESSION_IDENTIFIER 0x861A0000

static constexpr const std::uint32_t DEMO_HEAD = 'Ehed';
static constexpr const std::uint32_t DEMO_FOOT = 'Gfot';

static constexpr const std::uint32_t HEAD = 'head';
static constexpr const std::uint32_t FOOT = 'foot';

#define COPY_HEADER_FIELDS() \
    header.version = this->version; \
    header.size = this->size; \
    header.tags_offset = this->tags_offset; \
    header.tags_size = this->tags_size; \
    std::strncpy(header.name, this->name, 32); \
    header.name[31] = '\0'; \
    std::strncpy(header.build_number, this->build_number, 32); \
    header.build_number[31] = '\0'; \
    header.scenario_type = this->scenario_type; \
    header.checksum = this->checksum

struct CacheFileHeaderDemo;

struct CacheFileHeader {
    std::uint32_t header_signature;
    std::uint32_t version; // was int32_t
    std::int32_t size;
    std::int32_t compressed_file_padding;
    std::int32_t tags_offset;
    std::int32_t tags_size;
    std::int32_t index_buffer_count;
    std::int32_t index_buffers_offset;
    char name[32];
    char build_number[32];
    std::int16_t scenario_type;
    std::uint16_t pad;
    std::uint32_t checksum;
    std::uint32_t flags; // added in mcc cea
    std::uint32_t unused1[169];
    std::uint32_t lua_script_pointer;
    std::uint32_t lua_script_size;
    std::uint32_t unused2[313];
    std::uint32_t footer_signature;

    void set_compression_identifier() {
        this->version |= COMPRESSION_IDENTIFIER;
    }

    void unset_compression_identifier() {
        this->version &= ~COMPRESSION_IDENTIFIER;
    }

    [[nodiscard]] auto base_version() const {
        return this->version & ~COMPRESSION_IDENTIFIER;
    }

    [[nodiscard]] bool data_compressed() const {
        return (this->version | COMPRESSION_IDENTIFIER) == this->version;
    }

    [[nodiscard]] CacheFileHeader clean() const {
        CacheFileHeader header = {};
        header.header_signature = HEAD;
        header.footer_signature = FOOT;
        COPY_HEADER_FIELDS();

        // preserve these because we might want them later
        header.flags = this->flags;

        // these are bad but we can't take it back
        header.lua_script_pointer = this->lua_script_pointer;
        header.lua_script_size = this->lua_script_size;

        return header;
    }

    [[nodiscard]] CacheFileHeaderDemo demo_header() const;

    [[nodiscard]] bool valid() const {
        return this->header_signature == HEAD && this->footer_signature == FOOT;
    }
};
static_assert(sizeof(CacheFileHeader) == HEADER_SIZE);

struct CacheFileHeaderDemo {
    std::uint16_t pad;
    std::int16_t scenario_type;
    std::uint32_t unused1[175];
    std::uint32_t header_signature;
    std::int32_t tags_size;
    char build_number[32];
    std::uint32_t unused2[168];
    std::uint32_t version; // was int32_t
    char name[32];
    std::uint32_t unused3;
    std::uint32_t checksum;
    std::uint32_t unused4[13];
    std::int32_t size;
    std::int32_t tags_offset;
    std::uint32_t footer_signature;
    std::uint32_t unused5[131];

    [[nodiscard]] bool valid() const {
        return this->header_signature == DEMO_HEAD && this->footer_signature == DEMO_FOOT;
    }

    [[nodiscard]] CacheFileHeader full_header() const {
        CacheFileHeader header = {};
        header.header_signature = HEAD;
        header.footer_signature = FOOT;
        COPY_HEADER_FIELDS();

        return header;
    }
};
static_assert(sizeof(CacheFileHeaderDemo) == HEADER_SIZE);

CacheFileHeaderDemo CacheFileHeader::demo_header() const {
    CacheFileHeaderDemo header = {};
    header.header_signature = DEMO_HEAD;
    header.footer_signature = DEMO_FOOT;
    COPY_HEADER_FIELDS();
    // flags and Lua data can't be copied

    return header;
}

int main(int argc, const char **argv) {
    if(argc != 4 && argc != 3) {
        std::filesystem::path exe_path = argv[0];
        std::printf("Usage: %s <compress/decompress> <map>\n", exe_path.filename().string().c_str());
        std::printf("       %s <compress/decompress> <input-map> <output-map>\n", exe_path.filename().string().c_str());
        return EXIT_FAILURE;
    }

    // What are we doing?
    bool compress;
    if(std::strcmp(argv[1], "compress") == 0) {
        compress = true;
    }
    else if((std::strcmp(argv[1], "decompress") == 0) || (std::strcmp(argv[1], "uncompress") == 0)) {
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
    std::fseek(input_f, 0, SEEK_SET);

    // Read it
    auto read_result = std::fread(data_to_read.data(), data_to_read.size(), 1, input_f);
    std::fclose(input_f);
    if(read_result != 1) {
        std::fprintf(stderr, "Can't read %s\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Is there even a complete header?
    if(data_to_read.size() < HEADER_SIZE) {
        std::fprintf(stderr, "%s is not a cache file\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Can't be larger than this
    if(std::cmp_greater(data_to_read.size(), std::numeric_limits<std::int32_t>::max())) {
        std::fprintf(stderr, "%s is too large to be a valid cache file\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    bool demo = false;
    static CacheFileHeader header;
    memcpy(&header, data_to_read.data(), sizeof(CacheFileHeader));

    // Is it valid?
    if(header.valid()) {
        header = header.clean();
    }
    else {
        // Try demo
        auto header_demo = std::bit_cast<CacheFileHeaderDemo>(header);
        if(!header_demo.valid()) {
            std::fprintf(stderr, "%s does not appear to be a valid cache file\n", input_path.string().c_str());
            return EXIT_FAILURE;
        }

        header = header_demo.full_header();
        demo = true;
    }

    // Is it supported?
    switch(header.base_version()) {
        case 5:
            std::fprintf(stderr, "%s appears to be an Xbox map and is not supported\n", input_path.string().c_str());
            return EXIT_FAILURE;
        case 6:
            if(!demo) {
                std::fprintf(stderr, "%s has cache version 6 and is not demo format, this is not supported\n", input_path.string().c_str());
                return EXIT_FAILURE;
            }
            break;
        case 7:
        case 609:
            if(demo) {
                std::fprintf(stderr, "%s has cache version %u and is demo format, this is not supported\n", input_path.string().c_str(), header.base_version());
                return EXIT_FAILURE;
            }
            // these are supported
            break;
        case 13:
            std::fprintf(stderr, "%s appears to be an MCC CEA map and is not supported\n", input_path.string().c_str());
            return EXIT_FAILURE;
        default:
            std::fprintf(stderr, "%s has an unknown version %u\n", input_path.string().c_str(), header.version);
            return EXIT_FAILURE;
    }

    // Can we handle it?
    if(compress == header.data_compressed()) {
        std::fprintf(stderr, "%s %s to be compressed\n", input_path.string().c_str(), compress ? "appears" : "does not appear");
        return EXIT_FAILURE;
    }

    // Warn about these
    if(header.lua_script_pointer) {
        std::fprintf(stderr, "Warning: %s appears to have embedded Lua scripts, this is deprecated\n", input_path.string().c_str());
    }

    // The uncompressed size in the header is bullshit
    if(compress) {
        header.size = data_to_read.size();
    }

    if(header.size <= HEADER_SIZE) {
        std::fprintf(stderr, "%s has an invalid uncompressed size and its real uncompressed size cannot be determined\n", input_path.string().c_str());
        return EXIT_FAILURE;
    }

    std::vector<std::byte> file_to_write;
    if(compress) {
        file_to_write.resize(ZSTD_compressBound(header.size));

        std::byte *compressed_data = file_to_write.data() + HEADER_SIZE;
        std::size_t compressed_size = file_to_write.size() - HEADER_SIZE;
        const std::byte *decompressed_data = data_to_read.data() + HEADER_SIZE;
        std::size_t decompressed_size = data_to_read.size() - HEADER_SIZE;

        std::size_t actual_compressed_length = ZSTD_compress(compressed_data, compressed_size, decompressed_data, decompressed_size, 19);

        if(ZSTD_isError(actual_compressed_length)) {
            std::fprintf(stderr, "%s could not be compressed\n", input_path.string().c_str());
            return EXIT_FAILURE;
        }

        std::printf("%s: Compressed %zu bytes --> %zu bytes\n", input_path.string().c_str(), decompressed_size + HEADER_SIZE, actual_compressed_length + HEADER_SIZE);

        header.set_compression_identifier();

        file_to_write.resize(actual_compressed_length + HEADER_SIZE);
    }
    else {
        const std::byte *compressed_data = data_to_read.data() + HEADER_SIZE;
        std::size_t compressed_size = data_to_read.size() - HEADER_SIZE;

        file_to_write.resize(header.size);
        std::byte *decompressed_data = file_to_write.data() + HEADER_SIZE;
        std::size_t decompressed_size = file_to_write.size() - HEADER_SIZE;

        if(ZSTD_decompress(decompressed_data, decompressed_size, compressed_data, compressed_size) != decompressed_size) {
            std::fprintf(stderr, "%s could not be decompressed\n", input_path.string().c_str());
            return EXIT_FAILURE;
        }

        std::printf("%s: Decompressed %zu bytes --> %zu bytes\n", input_path.string().c_str(), compressed_size + HEADER_SIZE, decompressed_size + HEADER_SIZE);

        header.unset_compression_identifier();
    }

    if(!demo) {
        memcpy(file_to_write.data(), &header, sizeof(CacheFileHeader));
    }
    else {
        auto demo_header = header.demo_header();
        memcpy(file_to_write.data(), &demo_header, sizeof(CacheFileHeader));
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
