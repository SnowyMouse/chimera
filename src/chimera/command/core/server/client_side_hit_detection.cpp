// SPDX-License-Identifier: GPL-3.0-only

#include "../../../localization/localization.hpp"
#include "../../../halo_data/multiplayer.hpp"
#include "../../../signature/hook.hpp"
#include "../../../signature/signature.hpp"
#include "../../../event/damage.hpp"
#include "../../../event/vcr.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/chat.hpp"
#include "../../../halo_data/damage.hpp"
#include "../../../halo_data/tag.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/encode.hpp"

namespace Chimera {
    #define VCR_PREFIX "[VCR-CSHD-DMG] "

    static void on_vcr(const wchar_t *message, std::uint8_t sender) {
        if(server_type() != ServerType::SERVER_LOCAL) {
            return;
        }

        // Convert to UTF-8
        auto converted = u16_to_u8(message);
        char *converted_str = converted.data();
        console_output("Received %s", converted_str);
        if(std::strncmp(VCR_PREFIX, converted_str, std::strlen(VCR_PREFIX)) != 0) {
            console_output("Prefix mismatch");
            return;
        }
        converted_str += std::strlen(VCR_PREFIX);

        // Let's do this
        auto &table = PlayerTable::get_player_table();
        auto *player = table.get_player_by_rcon_id(sender);
        if(!player) {
            console_output("Sender invalid");
            return;
        }

        std::size_t player_getting_screwed = 0;
        float multiplier = 0;
        std::uint32_t tag_index = 0;
        std::sscanf(converted_str, "%zu %08X %f", &player_getting_screwed, &tag_index, &multiplier);

        // Bullshit
        auto *tag = get_tag(TagID { tag_index });
        if(!tag || tag->primary_class != TagClassInt::TAG_CLASS_DAMAGE_EFFECT) {
            console_output("Tag (%08X) invalid", tag_index);
            return;
        }

        // Get the player
        auto *memed_player = table.get_player_by_rcon_id(player_getting_screwed);
        if(!memed_player) {
            console_output("Target player (%zu) invalid", player_getting_screwed);
            return;
        }

        auto object = memed_player->object_id;
        auto *memed_object = ObjectTable::get_object_table().get_dynamic_object(object);
        if(!memed_object) {
            console_output("Target player (%zu) dead", player_getting_screwed);
            return;
        }

        // Apply damage
        auto old_bypass = get_bypass_damage_events();
        set_bypass_damage_events(true);
        apply_damage(object, tag->id, multiplier, player->get_full_id());
        set_bypass_damage_events(old_bypass);
    }

    static bool on_damage(ObjectID &damaged_id, TagID &damage_id, float &multiplier, PlayerID &damaging_player, ObjectID &) {
        auto st = server_type();

        // Ignore singleplayer
        if(st == ServerType::SERVER_NONE) {
            return true;
        }
        bool host = st == ServerType::SERVER_LOCAL;
        auto player_id = get_client_player_id();
        auto &object_table = ObjectTable::get_object_table();
        auto *damaged_object = object_table.get_dynamic_object(damaged_id);
        auto &player_table = PlayerTable::get_player_table();

        if(host) {
            // If we're the host, passthrough anything done by us or anything that isn't a player
            if(damaging_player.is_null() || damaging_player == player_id) {
                return true;
            }

            // Also, we should passthrough if it's a weapon or equipment that doesn't have a parent
            if(damaged_object->type == ObjectType::OBJECT_TYPE_WEAPON || damaged_object->type == ObjectType::OBJECT_TYPE_EQUIPMENT) {
                return damaged_object->parent.is_null();
            }

            // If it's a vehicle, check if it has anything that isn't a biped that's parented to it
            if(damaged_object->type == ObjectType::OBJECT_TYPE_VEHICLE) {
                for(std::size_t i = 0; i < object_table.max_elements; i++) {
                    auto *dynamic_object = object_table.get_dynamic_object(i);
                    if(dynamic_object && dynamic_object->parent == damaged_id && dynamic_object->type == ObjectType::OBJECT_TYPE_BIPED) {
                        // If so, then check if this is a player
                        for(std::size_t i = 0; i < 16; i++) {
                            auto *player = player_table.get_player_by_rcon_id(i);
                            if(player && player->object_id == dynamic_object->full_object_id()) {
                                return false;
                            }
                        }
                    }
                }
                return true;
            }

            return false;
        }

        // If we're NOT the host, we can't do damage. But block anything we aren't responsible for
        if(player_id != damaging_player) {
            return false;
        }

        // Resolve the object ID to the player
        for(std::size_t i = 0; i < 16; i++) {
            auto *player = player_table.get_player_by_rcon_id(i);
            if(player && player->object_id == damaged_id) {
                char memes[256];
                std::snprintf(memes, sizeof(memes), VCR_PREFIX "%zu %08X %f", player->machine_index, damage_id.whole_id, multiplier);
                console_output("Sent %s", memes);
                chat_out(0, memes);
                return false;
            }
        }

        return false;
    }

    bool client_side_hit_detection_command(int argc, const char **argv) {
        static bool active = false;

        if(argc) {
            bool new_active = STR_TO_BOOL(*argv);
            if(new_active != active) {
                active = new_active;
                if(active) {
                    add_vcr_event(on_vcr);
                    add_damage_event(on_damage);
                }
                else {
                    remove_vcr_event(on_vcr);
                    remove_damage_event(on_damage);
                }
            }
        }

        console_output(BOOL_TO_STR(active));
        return true;
    }
}
