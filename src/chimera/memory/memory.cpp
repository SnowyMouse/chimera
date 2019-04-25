#include <algorithm>
#include <windows.h>
#include <shlwapi.h>
#include "memory.hpp"
#include "header.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../output/output.hpp"
#include "../halo_data/tag_class.hpp"
#include "../halo_data/resource_map.hpp"
#include "../event/frame.hpp"
#include "../halo_data/tag.hpp"
#include "../fast_load/fast_load.hpp"
#include "decompress.hpp"
#include "bitmap.hpp"
#include "../localization/localization.hpp"
#include "../config/ini.hpp"

#include <mutex>
#include <thread>

extern "C" {
    void on_read_map_data() noexcept;
    void on_select_map() noexcept;
    const void *on_read_map_data_actually_do_it_now;
}

namespace Chimera {
    static char map_path[MAX_PATH] = "%s%s%s.map";
    static std::byte *data_allocation = nullptr;

    void free_chimera_memory() noexcept {
        if(data_allocation) {
            VirtualFree(data_allocation, CHIMERA_MEMORY_ALLOCATION_SIZE, MEM_RELEASE);
            data_allocation = nullptr;
        }
    }

    static std::uint16_t *loaded_map_index;
    static std::byte *loaded_map_ptr;

    static void failed_message() noexcept;

    static void actually_set_up_chimera_memory() noexcept {
        const char *decompression_benchmark = get_chimera().get_ini()->get_value("decompression.show_benchmark");
        const char *decompression_threads = get_chimera().get_ini()->get_value("decompression.max_threads");

        if(decompression_benchmark) {
            set_show_decompression_benchmark(STR_TO_BOOL(decompression_benchmark));
        }

        if(decompression_threads) {
            int max_decompression_threads = std::stoi(decompression_threads);
            if(max_decompression_threads < 1) {
                max_decompression_threads = 1;
            }
            set_decompression_threads(static_cast<std::size_t>(max_decompression_threads));
        }

        const char *enable_this = get_chimera().get_ini()->get_value("memory.enable_map_memory_buffer");
        if(enable_this && !STR_TO_BOOL(enable_this)) {
            return;
        }

        data_allocation = reinterpret_cast<std::byte *>(VirtualAlloc(0, CHIMERA_MEMORY_ALLOCATION_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if(!data_allocation) {
            add_frame_event(failed_message);
            return;
        }

        static Hook hook;
        write_function_override(get_chimera().get_signature("read_file_ex_map_sig").data(), hook, reinterpret_cast<const void *>(on_read_map_data), &on_read_map_data_actually_do_it_now);

        auto *loaded_map_data = get_chimera().get_signature("loaded_header_sig").data();
        loaded_map_index = *reinterpret_cast<std::uint16_t **>(loaded_map_data + 3);
        loaded_map_ptr = *reinterpret_cast<std::byte **>(loaded_map_data + 15);

        const static SigByte nop6[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        const std::uint16_t nop2 = 0x9090;

        write_code_s(get_chimera().get_signature("ce_check_load_sig").data() + 10, nop6);
        write_code_s(get_chimera().get_signature("ce_check_startup_sig").data() + 10, nop6);
        overwrite(get_chimera().get_signature("ce_check_preload_map_list_sig").data() + 7, nop2);
        overwrite(get_chimera().get_signature("ce_check_loading_sig").data() + 7, static_cast<std::uint8_t>(0xEB));

        // make a hook and overwrite the map path
        if(get_chimera().feature_present("client")) {
            static Hook path_hook;
            auto *map_path_1_addr = get_chimera().get_signature("map_client_path_1_sig").data();
            overwrite(map_path_1_addr + 15, reinterpret_cast<const char *>(map_path));
            write_jmp_call(map_path_1_addr, path_hook, reinterpret_cast<const void *>(on_select_map));
            overwrite(get_chimera().get_signature("map_client_path_2_sig").data() + 16, reinterpret_cast<const char *>(map_path));
        }
        else {
            static Hook path_hook;
            auto *map_path_1_addr = get_chimera().get_signature("map_server_path_1_sig").data();
            overwrite(map_path_1_addr + 11, reinterpret_cast<const char *>(map_path));
            write_jmp_call(map_path_1_addr, path_hook, reinterpret_cast<const void *>(on_select_map));
            overwrite(get_chimera().get_signature("map_server_path_2_sig").data() + 1, reinterpret_cast<const char *>(map_path));
        }
        overwrite(get_chimera().get_signature("map_path_3_sig").data() + 1, reinterpret_cast<const char *>(map_path));
    }

    void set_up_chimera_memory() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("after_memory_allocated_sig").data() + 6, hook, reinterpret_cast<const void *>(actually_set_up_chimera_memory));
    }

    extern "C" void handle_select_map(const char *map) {
        std::strcpy(map_path, path_for_map(map));
    }

    static char last_map[256] = {};
    static HANDLE last_map_handle = 0;
    static bool in_memory = false;

    static void load_asset_data() noexcept;

    // Size of the currently loaded map
    static std::size_t loaded_map_size = 0;

    extern "C" void override_file_read(HANDLE file, LPVOID buffer, DWORD &number_of_bytes_to_read, LPOVERLAPPED lpOverlapped) {
        // Check if there is no map loaded?
        if(*loaded_map_index == 0xFFFF) {
            return;
        }

        // Get the handle otherwise
        auto *ptr = loaded_map_ptr + *loaded_map_index * 0x80C;
        HANDLE f = *reinterpret_cast<HANDLE *>(ptr);

        // Check if this is something we can save
        if(file != f) {
            return;
        }

        // And if we don't know what to do with this, give up
        if(!lpOverlapped) {
            return;
        }

        // Check what map we're loading
        const char *name = reinterpret_cast<VapMap::VAPFileHeader *>(ptr + 0xC)->name;

        // Do NOT load ui
        if(std::strcmp(name, "ui") == 0) {
            return;
        }

        // Check if we're changing maps
        if(std::strcmp(name, last_map) != 0) {
            std::strncpy(last_map, name, sizeof(last_map));
            last_map_handle = file;
            read_map(path_for_map(name));
            in_memory = true;
        }

        // Check if we couldn't do it
        if(!in_memory) {
            return;
        }

        if(get_chimera().feature_present("client") && buffer == reinterpret_cast<LPVOID>(0x40440000)) {
            add_preframe_event(load_asset_data);
        }

        read_memory(reinterpret_cast<std::byte *>(buffer), lpOverlapped->Offset, number_of_bytes_to_read);
        number_of_bytes_to_read = 0;
    }

    std::byte *get_memory() noexcept {
        return data_allocation;
    }

    void read_memory(std::byte *output, std::size_t offset, std::size_t size) noexcept {
        std::copy(data_allocation + offset, data_allocation + offset + size, output);
    }

    void write_memory(const std::byte *input, std::size_t offset, std::size_t size) noexcept {
        std::copy(input, input + size, data_allocation + offset);
    }

    /**
     * Write data to memory from a file
     * @param  data_offset offset of the data
     * @param  file_offset offset of the file
     * @param  file_size   size of the file data
     * @param  file        file descriptor
     * @return             true if successful
     */
    static void write_chunk_data_from_file(std::size_t data_offset, std::size_t file_offset, std::size_t file_size, FILE *file) {
        std::fseek(file, file_offset, SEEK_SET);
        std::fread(data_allocation + data_offset, file_size, 1, file);
    }

    #define MAX_COMPRESSED_BLOCKS 64

    char last_map_loaded[MAX_PATH] = {};

    // Asset offsets (so I don't have to reload the same offsets again)
    // It's offset, size
    static std::vector<std::pair<std::uint32_t, std::uint32_t>> asset_offsets;

    void read_map(const char *path) {
        // Check if we loaded it previously. If not, invalidate it. If so, return true.
        if(std::strcmp(last_map_loaded, path) == 0) {
            return;
        }
        asset_offsets.clear();
        last_map_loaded[0] = 0;

        FILE *f = std::fopen(path, "rb");
        if(f) {
            // Get the size
            std::fseek(f, 0, SEEK_END);
            loaded_map_size = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            if(!std::fread(data_allocation, sizeof(VapMap::VAPFileHeader), 1, f)) {
                std::fclose(f);
                return;
            }

            VapMap::VAPFileHeader &header = *reinterpret_cast<VapMap::VAPFileHeader *>(data_allocation);

            // Try to figure out how to read this map into memory
            if(header.engine == VapMap::VAPFileHeader::VAP_ENGINE_VAP) {
                if(header.vap.compression_type == VapMap::VAPStruct::VAP_COMPRESSION_LZMA && header.vap.block_count < MAX_COMPRESSED_BLOCKS) {
                    loaded_map_size = decompress_map_file(f, &header);
                }
                else if(header.vap.compression_type == VapMap::VAPStruct::VAP_COMPRESSION_UNCOMPRESSED) {
                    write_chunk_data_from_file(sizeof(header), sizeof(header), loaded_map_size - sizeof(header), f);
                }
            }
            else {
                write_chunk_data_from_file(sizeof(header), sizeof(header), loaded_map_size - sizeof(header), f);
            }

            std::fclose(f);
            std::strncpy(last_map_loaded, path, sizeof(last_map_loaded) - 1);
        }
    }

    // See if we can load any bitmaps/sounds into stuff so Halo doesn't have to do it later
    static void load_asset_data() noexcept {
        remove_preframe_event(load_asset_data);

        // If we haven't loaded these assets previously, now is the time to do that
        FILE *bf = nullptr, *sf = nullptr;
        bool already_loaded = asset_offsets.size() > 0;
        std::size_t already_loaded_offset = 0;

        if(!already_loaded) {
            bf = std::fopen("maps\\bitmaps.map", "rb");
            sf = std::fopen("maps\\sounds.map", "rb");
        }

        bool successful = (bf != nullptr && sf != nullptr) || already_loaded;

        #define CAN_CONTINUE(boolean) (boolean && successful)

        auto &header = get_tag_data_header();
        for(auto *tag = header.tag_array; CAN_CONTINUE(tag < (header.tag_array + header.tag_count)); tag++) {
            // Get the bitmaps from the bitmaps.map file
            if(tag->primary_class == TagClassInt::TAG_CLASS_BITMAP) {
                auto *data = tag->data;
                std::uint32_t bitmap_count = *reinterpret_cast<std::uint32_t *>(data + 0x60);
                auto *bitmap_addr = *reinterpret_cast<std::byte **>(data + 0x60 + 4);
                for(std::size_t b = 0; CAN_CONTINUE(b < bitmap_count); b++) {
                    // Get bitmap info. Make sure it's not already in the map
                    auto *bitmap = bitmap_addr + b * 0x30;
                    auto &flag = *reinterpret_cast<std::uint8_t *>(bitmap + 0xF);
                    std::uint32_t &bitmap_offset = *reinterpret_cast<std::uint32_t *>(bitmap + 0x18);
                    std::uint32_t &bitmap_size = *reinterpret_cast<std::uint32_t *>(bitmap + 0x1C);
                    if(flag) {
                        if(already_loaded) {
                            bitmap_offset = asset_offsets[already_loaded_offset].first;
                            bitmap_size = asset_offsets[already_loaded_offset].second;
                            flag = 0;
                        }
                        else {
                            write_chunk_data_from_file(loaded_map_size, bitmap_offset, bitmap_size, bf);
                            bitmap_offset = loaded_map_size;
                            flag = 0;
                            loaded_map_size += bitmap_size;

                            asset_offsets.emplace_back(bitmap_offset, bitmap_size);
                        }

                        already_loaded_offset++;
                    }

                    // Re-encode something if needed?
                    std::uint16_t &format = *reinterpret_cast<std::uint16_t *>(bitmap + 0xC);
                    if(format < 4 || format == 17) {
                        std::uint32_t offset_to_write_to, size_written;

                        if(already_loaded) {
                            offset_to_write_to = asset_offsets[already_loaded_offset].first;
                            size_written = asset_offsets[already_loaded_offset].second;
                        }
                        else {
                            if(loaded_map_size % 4 != 0) {
                                loaded_map_size += 4 - (loaded_map_size % 4);
                            }

                            size_written = bitmap_size;
                            if(format == 3) {
                                size_written *= 2;
                            }
                            else {
                                size_written *= 4;
                            }

                            offset_to_write_to = loaded_map_size;
                            write_encoded_chunk_data_from_memory(offset_to_write_to, bitmap_offset, bitmap_size, static_cast<ReEncode>(format));
                            asset_offsets.emplace_back(offset_to_write_to, size_written);
                            loaded_map_size += size_written;
                        }
                        already_loaded_offset++;

                        bitmap_size = size_written;
                        bitmap_offset = offset_to_write_to;
                        if(format == 1) {
                            format = 10;
                        }
                        else {
                            format = 11;
                        }
                    }
                }
            }

            // Get the sounds from the sounds.map file
            else if(tag->primary_class == TagClassInt::TAG_CLASS_SOUND) {
                auto *data = tag->data;
                std::uint32_t range_count = *reinterpret_cast<std::uint32_t *>(data + 0x98);
                auto *range_addr = *reinterpret_cast<std::byte **>(data + 0x9C);
                for(std::size_t r = 0; CAN_CONTINUE(r < range_count); r++) {
                    auto *range = range_addr + r * 0x48;
                    std::uint32_t perm_count = *reinterpret_cast<std::uint32_t *>(range + 0x3C);
                    auto *perm_addr = *reinterpret_cast<std::byte **>(range + 0x3C + 4);
                    for(std::size_t p = 0; CAN_CONTINUE(p < perm_count); p++) {
                        // Load sound permutation info, making sure it's not already in the map
                        auto *perm = perm_addr + p * 0x7C;
                        auto *sound_data = perm + 0x7C - 0x14 * 3;
                        std::uint32_t &external = *reinterpret_cast<std::uint32_t *>(sound_data + 0x4);

                        if((external & 1) == 0) {
                            continue;
                        }

                        std::uint32_t sample_size = *reinterpret_cast<std::uint32_t *>(sound_data + 0x0);
                        std::uint32_t &sample_offset = *reinterpret_cast<std::uint32_t *>(sound_data + 0x8);

                        if(already_loaded) {
                            sample_offset = asset_offsets[already_loaded_offset].first;
                            external = 0;
                        }
                        else {
                            write_chunk_data_from_file(loaded_map_size, sample_offset, sample_size, sf);
                            sample_offset = loaded_map_size;
                            external = 0;
                            loaded_map_size += sample_size;

                            asset_offsets.emplace_back(sample_offset, sample_size);
                        }

                        already_loaded_offset++;
                    }
                }
            }
        }

        if(!already_loaded) {
            std::fclose(bf);
            std::fclose(sf);
        }
    }

    static void failed_message() noexcept {
        remove_frame_event(failed_message);
        console_error(localize("chimera_error_failed_allocation"));
    }
}
