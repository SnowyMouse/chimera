// SPDX-License-Identifier: GPL-3.0-only

#include "../../../halo_data/tag.hpp"
#include "../../../output/output.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/damage.hpp"

#include <optional>
#include <string>

namespace Chimera {
    static std::optional<std::uint32_t> read_hex(const char *f) {
        try {
            return std::stoll(f, nullptr, 16);
        }
        catch(std::exception &) {
            return std::nullopt;
        }
    }

    bool apply_damage_command(int argc, const char **argv) {
        // Get the damage effect
        auto *tag = get_tag(argv[0], TagClassInt::TAG_CLASS_DAMAGE_EFFECT);
        if(!tag) {
            console_error("Tag path does not correspond to a valid damage_effect tag");
            return false;
        }

        TagID damage_effect = tag->id;
        PlayerID causer_player_id = PlayerID::null_id();
        ObjectID damaged_object_id;
        ObjectID causer_object_id = ObjectID::null_id();
        auto &object_table = ObjectTable::get_object_table();
        float multiplier = 1.0F;

        // Make sure the object is valid
        try {
            damaged_object_id.whole_id = read_hex(argv[1]).value();
        }
        catch (std::exception &) {
            console_error("Damaged Object ID given is invalid (must be hexadecimal)");
            return false;
        }
        auto *damaged_object = object_table.get_dynamic_object(damaged_object_id);
        if(!damaged_object) {
            console_error("Damaged Object ID does not correspond to a valid object");
            return false;
        }

        // Get multiplier
        if(argc >= 3) {
            try {
                multiplier = std::strtof(argv[2], nullptr);
            }
            catch (std::exception &) {
                console_error("Multiplier must be a valid number");
                return false;
            }
        }

        // Get causer player/object
        if(argc >= 4) {
            try {
                auto player_index = std::stoll(argv[3]);
                if(player_index != -1) {
                    auto &player_table = PlayerTable::get_player_table();
                    auto *player = player_table.get_player_by_rcon_id(player_index - 1);
                    if(!player) {
                        throw std::exception();
                    }
                    causer_player_id = player->get_full_id();
                }
            }
            catch(std::exception &) {
                console_error("Damager Player Index given is invalid (must be a valid rcon index or -1)");
            }

            if(argc == 5) {
                try {
                    causer_object_id.whole_id = read_hex(argv[4]).value();
                }
                catch (std::exception &) {
                    console_error("Damager Object ID given is invalid (must be hexadecimal)");
                    return false;
                }
                auto *causer_object = object_table.get_dynamic_object(causer_object_id);
                if(!causer_object) {
                    console_error("Damager Object ID does not correspond to a valid object");
                    return false;
                }
            }
        }

        apply_damage(damaged_object_id, damage_effect, multiplier, causer_player_id, causer_object_id);

        return true;
    }
}
