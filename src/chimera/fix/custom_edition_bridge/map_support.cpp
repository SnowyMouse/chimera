// SPDX-License-Identifier: GPL-3.0-only

#include <cstring>
#include <cstdio>
#include <vector>
#include <memory>
#include "map_support.hpp"
#include "../../halo_data/tag.hpp"
#include "../../halo_data/map.hpp"
#include "../../event/map_load.hpp"
#include "../../chimera.hpp"
#include "../../signature/hook.hpp"
#include "../../signature/signature.hpp"
#include "../../map_loading/map_loading.hpp"

namespace Chimera {
    extern "C" {
        void override_ting_volume_set_asm() noexcept;
        void override_ting_volume_write_asm() noexcept;
    }

    static std::vector<std::unique_ptr<std::byte []>> bitmaps_custom, sounds_custom, loc_custom;
    static std::vector<std::string> sounds_custom_index;
    static std::FILE *bitmaps_custom_rsc = nullptr, *sounds_custom_rsc = nullptr, *loc_custom_rsc = nullptr;

    static void jason_jones_the_map() noexcept {
        auto &header = get_map_header();
        if(
            std::strcmp("beavercreek",header.name) == 0 ||
            std::strcmp("bloodgulch",header.name) == 0 ||
            std::strcmp("boardingaction",header.name) == 0 ||
            std::strcmp("carousel",header.name) == 0 ||
            std::strcmp("chillout",header.name) == 0 ||
            std::strcmp("damnation",header.name) == 0 ||
            std::strcmp("dangercanyon",header.name) == 0 ||
            std::strcmp("deathisland",header.name) == 0 ||
            std::strcmp("gephyrophobia",header.name) == 0 ||
            std::strcmp("hangemhigh",header.name) == 0 ||
            std::strcmp("icefields",header.name) == 0 ||
            std::strcmp("infinity",header.name) == 0 ||
            std::strcmp("longest",header.name) == 0 ||
            std::strcmp("prisoner",header.name) == 0 ||
            std::strcmp("putput",header.name) == 0 ||
            std::strcmp("ratrace",header.name) == 0 ||
            std::strcmp("sidewinder",header.name) == 0 ||
            std::strcmp("timberland",header.name) == 0 ||
            std::strcmp("wizard",header.name) == 0
        ) {
            bool in_custom_edition_server = false;

            // Fix the stun values
            auto jj_stun = [&in_custom_edition_server](const char *path) {
                auto *tag = get_tag(path, TagClassInt::TAG_CLASS_DAMAGE_EFFECT);
                if(tag) {
                    float new_damage_stun = in_custom_edition_server ? 0.0F : 1.0F;
                    float damage_maximum_stun = in_custom_edition_server ? 0.0F : 1.0F;
                    float damage_stun_time = in_custom_edition_server ? 0.0F : 0.15F;
                    *reinterpret_cast<float *>(tag->data + 0x1E4) = new_damage_stun;
                    *reinterpret_cast<float *>(tag->data + 0x1E8) = damage_maximum_stun;
                    *reinterpret_cast<float *>(tag->data + 0x1EC) = damage_stun_time;
                }
            };
            jj_stun("vehicles\\banshee\\banshee bolt");
            jj_stun("vehicles\\ghost\\ghost bolt");

            // Fix the rwarthog's angles
            auto jj_rwarthog = [&in_custom_edition_server](const char *path) {
                auto *tag = get_tag(path, TagClassInt::TAG_CLASS_WEAPON);
                if(tag) {
                    float new_autoaim_angle = in_custom_edition_server ? DEGREES_TO_RADIANS(6.0F) : DEGREES_TO_RADIANS(1.0F);
                    float new_deviation_angle = in_custom_edition_server ? DEGREES_TO_RADIANS(12.0F) : DEGREES_TO_RADIANS(1.0F);
                    *reinterpret_cast<float *>(tag->data + 0x3E4) = new_autoaim_angle;
                    *reinterpret_cast<float *>(tag->data + 0x3F4) = new_deviation_angle;
                }
            };
            jj_rwarthog("vehicles\\rwarthog\\rwarthog_gun");
        }
    }

    static void load_custom_edition_tags_into_retail_finally() noexcept {
        auto &header = get_map_header();
        auto engine_type = header.engine_type;
        jason_jones_the_map(); // make sure the map is compatible with the current netcode

        if(engine_type == CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
            for(std::size_t i = 0; i < get_tag_data_header().tag_count; i++) {
                Tag *tag = get_tag(i);

                if(tag->indexed) {
                    std::byte *base = nullptr, *sound_header = nullptr;
                    if(tag->primary_class == TagClassInt::TAG_CLASS_SOUND) {
                        for(std::size_t i = 0; i < sounds_custom_index.size(); i++) {
                            if(sounds_custom_index[i] == tag->path) {
                                sound_header = sounds_custom[i].get();
                                base = sound_header + 0xA4;
                                *reinterpret_cast<std::byte **>(tag->data + 0x98 + 0x4) = base;
                                break;
                            }
                        }
                    }
                    else {
                        std::uint32_t index = reinterpret_cast<std::uint32_t>(tag->data);
                        auto &thing_to_use = tag->primary_class == TagClassInt::TAG_CLASS_BITMAP ? bitmaps_custom : loc_custom;
                        if(thing_to_use.size() > index) {
                            base = thing_to_use[index].get();
                            tag->data = base;
                        }
                    }

                    // If none was found, give up
                    if(!base) {
                        continue;
                    }

                    // Offset to work with
                    auto base_offset = reinterpret_cast<std::uintptr_t>(base);

                    #define INCREMENT_IF_NECESSARY(what) { \
                        auto &ptr = *reinterpret_cast<std::byte **>(what); \
                        if(ptr < base && ptr != 0) { \
                            ptr += base_offset; \
                        } \
                    }

                    // Now fix the pointers
                    switch(tag->primary_class) {
                        case TagClassInt::TAG_CLASS_BITMAP: {
                            INCREMENT_IF_NECESSARY(base + 0x54 + 0x4);
                            INCREMENT_IF_NECESSARY(base + 0x60 + 0x4);
                            auto sequence_count = *reinterpret_cast<std::uint32_t *>(base + 0x54);
                            auto *sequences = *reinterpret_cast<std::byte **>(base + 0x54 + 0x4);
                            for(std::uint32_t s = 0; s < sequence_count; s++) {
                                INCREMENT_IF_NECESSARY(sequences + s * 64 + 0x34 + 0x4);
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_SOUND: {
                            auto pitch_range_count = *reinterpret_cast<std::uint32_t *>(tag->data + 0x98);

                            // Copy over channel count and format
                            *reinterpret_cast<std::uint16_t *>(tag->data + 0x6C) = *reinterpret_cast<std::uint16_t *>(sound_header + 0x6C);
                            *reinterpret_cast<std::uint16_t *>(tag->data + 0x6E) = *reinterpret_cast<std::uint16_t *>(sound_header + 0x6E);

                            // Copy over sample rate
                            *reinterpret_cast<std::uint16_t *>(tag->data + 0x6) = *reinterpret_cast<std::uint16_t *>(sound_header + 0x6);

                            // Copy over longest permutation length
                            *reinterpret_cast<std::uint32_t *>(tag->data + 0x84) = *reinterpret_cast<std::uint32_t *>(sound_header + 0x84);

                            for(std::uint32_t p = 0; p < pitch_range_count; p++) {
                                auto *pitch_range = base + p * 72;
                                INCREMENT_IF_NECESSARY(pitch_range + 0x3C + 0x4);
                                auto permutation_count = *reinterpret_cast<std::uint32_t *>(pitch_range + 0x3C);
                                auto permutations = *reinterpret_cast<std::byte **>(pitch_range + 0x3C + 0x4);

                                *reinterpret_cast<std::uint32_t *>(pitch_range + 0x34) = 0xFFFFFFFF;
                                *reinterpret_cast<std::uint32_t *>(pitch_range + 0x38) = 0xFFFFFFFF;

                                for(std::uint32_t pe = 0; pe < permutation_count; pe++) {
                                    auto *permutation = permutations + pe * 124;

                                    *reinterpret_cast<std::uint32_t *>(permutation + 0x2C) = 0xFFFFFFFF;
                                    *reinterpret_cast<std::uint32_t *>(permutation + 0x30) = 0;

                                    INCREMENT_IF_NECESSARY(permutation + 0x54 + 0xC);
                                    INCREMENT_IF_NECESSARY(permutation + 0x68 + 0xC);
                                    *reinterpret_cast<TagID *>(permutation + 0x34) = tag->id;
                                    *reinterpret_cast<TagID *>(permutation + 0x3C) = tag->id;
                                    *reinterpret_cast<std::uint32_t *>(permutation + 0x2C) = 0xFFFFFFFF;
                                }
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_FONT: {
                            INCREMENT_IF_NECESSARY(base + 0x7C + 0x4);
                            INCREMENT_IF_NECESSARY(base + 0x30 + 0x4);
                            INCREMENT_IF_NECESSARY(base + 0x88 + 0xC);
                            std::uint32_t table_count = *reinterpret_cast<std::uint32_t *>(base + 0x30);
                            auto *tables = *reinterpret_cast<std::byte **>(base + 0x30 + 0x4);
                            for(std::uint32_t t = 0; t < table_count; t++) {
                                INCREMENT_IF_NECESSARY(tables + t * 12 + 0x0 + 0x4);
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST: {
                            INCREMENT_IF_NECESSARY(base + 0x0 + 0x4);
                            std::uint32_t string_count = *reinterpret_cast<std::uint32_t *>(base + 0x0);
                            std::byte *strings = *reinterpret_cast<std::byte **>(base + 0x0 + 0x4);
                            for(std::uint32_t s = 0; s < string_count; s++) {
                                INCREMENT_IF_NECESSARY(strings + s * 20 + 0x0 + 0xC);
                            }
                            break;
                        }
                        case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT: {
                            INCREMENT_IF_NECESSARY(base + 0x0 + 0xC);
                            INCREMENT_IF_NECESSARY(base + 0x14 + 0x4);
                            INCREMENT_IF_NECESSARY(base + 0x20 + 0x4);
                            break;
                        }
                        default:
                            break;
                    }

                    #undef INCREMENT_IF_NECESSARY
                }
            }
        }
    }

    static bool custom_maps_on_retail = false;
    bool set_up_custom_edition_map_support() noexcept {
        custom_maps_on_retail = true;

        bitmaps_custom_rsc = std::fopen(BITMAPS_CUSTOM_MAP, "rb");
        sounds_custom_rsc = std::fopen(SOUNDS_CUSTOM_MAP, "rb");
        loc_custom_rsc = std::fopen(LOC_CUSTOM_MAP, "rb");
        if((custom_maps_on_retail = bitmaps_custom_rsc && sounds_custom_rsc && loc_custom_rsc)) {
            auto load_external_resources = [](std::FILE *file, std::vector<std::unique_ptr<std::byte []>> &into, std::vector<std::string> *index = nullptr) {
                if(!custom_maps_on_retail) {
                    return;
                }

                #define ASSERT_OR_BAIL_LOAD_EXT_RES(what) if(!(what)) { into.clear(); if(index) { index->clear(); } custom_maps_on_retail = false; return; };

                // Read the header
                std::byte header[0x10];
                ASSERT_OR_BAIL_LOAD_EXT_RES(std::fread(header, sizeof(header), 1, file));

                auto names = *reinterpret_cast<std::uint32_t *>(header + 0x4);
                auto indices = *reinterpret_cast<std::uint32_t *>(header + 0x8);
                auto count = *reinterpret_cast<std::uint32_t *>(header + 0xC);

                std::size_t name_size = indices - names;
                std::unique_ptr<char []> names_data = std::make_unique<char []>(name_size);
                std::fseek(file, names, SEEK_SET);
                ASSERT_OR_BAIL_LOAD_EXT_RES(std::fread(names_data.get(), name_size, 1, file));

                // Size things
                if(index) {
                    index->resize(count);
                }
                into.resize(count);

                // Read names
                std::uint32_t start = file != loc_custom_rsc ? 1 : 0;
                std::uint32_t increment = file != loc_custom_rsc ? 2 : 1;

                for(std::uint32_t i = start; i < count; i+=increment) {
                    std::byte item[0xC];
                    std::fseek(file, indices + sizeof(item) * i, SEEK_SET);
                    ASSERT_OR_BAIL_LOAD_EXT_RES(std::fread(item, sizeof(item), 1, file));

                    auto name_offset = *reinterpret_cast<std::uint32_t *>(item + 0x0);
                    auto size = *reinterpret_cast<std::uint32_t *>(item + 0x4);
                    auto data_offset = *reinterpret_cast<std::uint32_t *>(item + 0x8);

                    if(index) {
                        (*index)[i] = names_data.get() + name_offset;
                    }

                    into[i] = std::make_unique<std::byte []>(size);
                    std::fseek(file, data_offset, SEEK_SET);
                    ASSERT_OR_BAIL_LOAD_EXT_RES(std::fread(into[i].get(), size, 1, file));
                }

                #undef ASSERT_OR_BAIL_LOAD_EXT_RES
            };

            load_external_resources(bitmaps_custom_rsc, bitmaps_custom);
            load_external_resources(sounds_custom_rsc, sounds_custom, &sounds_custom_index);
            load_external_resources(loc_custom_rsc, loc_custom);

            if(custom_maps_on_retail) {
                overwrite(get_chimera().get_signature("retail_check_version_1_sig").data() + 7, static_cast<std::uint16_t>(0x9090));
                overwrite(get_chimera().get_signature("retail_check_version_2_sig").data() + 4, static_cast<std::uint8_t>(0xEB));

                static Hook set_flag, set_float;
                write_jmp_call(get_chimera().get_signature("ting_sound_call_sig").data(), set_flag, nullptr, reinterpret_cast<const void *>(override_ting_volume_set_asm), false);
                write_jmp_call(get_chimera().get_signature("game_event_volume_sig").data(), set_float, nullptr, reinterpret_cast<const void *>(override_ting_volume_write_asm), false);

                add_map_load_event(load_custom_edition_tags_into_retail_finally);
            }
        }

        if(!custom_maps_on_retail) {
            if(bitmaps_custom_rsc) std::fclose(bitmaps_custom_rsc);
            if(sounds_custom_rsc) std::fclose(sounds_custom_rsc);
            if(loc_custom_rsc) std::fclose(loc_custom_rsc);

            bitmaps_custom_rsc = nullptr;
            sounds_custom_rsc = nullptr;
            loc_custom_rsc = nullptr;
        }

        return custom_maps_on_retail;
    }

    void load_custom_edition_resource_data_in_retail(std::byte *output, std::size_t file_offset, std::size_t file_size, const char *map_name) {
        std::FILE *f;

        if(std::strcmp(map_name, "bitmaps") == 0) {
            f = bitmaps_custom_rsc;
        }
        else if(std::strcmp(map_name, "sounds") == 0) {
            f = sounds_custom_rsc;
        }
        else {
            f = loc_custom_rsc;
        }

        // Do it!
        std::fseek(f, static_cast<long>(file_offset), SEEK_SET);
        std::fread(output, file_size, 1, f);
    }

    bool custom_edition_maps_supported_on_retail() noexcept {
        return custom_maps_on_retail;
    }
}
