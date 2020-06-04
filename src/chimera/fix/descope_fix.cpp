// SPDX-License-Identifier: GPL-3.0-only

#include "descope_fix.hpp"

#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"
#include "../halo_data/object.hpp"
#include "../halo_data/player.hpp"
#include "../chimera.hpp"

namespace Chimera {
    typedef void (*ds)(ObjectID object_id);
    static ds do_descope = nullptr;
    static bool descope_fix_enabled = false;

    static void set_halo_descoping(bool do_it) {
        // Don't do anything if we don't have to
        if(!do_it == descope_fix_enabled) {
            return;
        }

        // Set whether or not to do it
        descope_fix_enabled = !do_it;
        auto &descope_fn_sig = get_chimera().get_signature("descope_fix_sig");
        if(do_it) {
            descope_fn_sig.rollback();
        }
        else {
            const short mod[] = {0x90,0x90,0x90,0x90,0x90};
            write_code_s(descope_fn_sig.data(), mod);
        }
    }

    static void fix_descoping() noexcept {
        // More like disable descoping and do it when the player loses health or shield
        static float health = 0;
        static float shield = 0;
        auto *player = PlayerTable::get_player_table().get_client_player();
        if(!player) {
            return;
        }
        auto *object = ObjectTable::get_object_table().get_dynamic_object(player->object_id);
        if(object) {
            set_halo_descoping(object->shield > shield && object->shield > 1.0); // enable regular descoping if overshield is charging

            if(object->health < health || (object->shield < shield && (shield - object->shield) > 0.01)) {
                if(descope_fix_enabled) {
                    do_descope(player->object_id);
                }
            }

            health = object->health;
            shield = object->shield;
        }
    }

    void set_up_descope_fix() noexcept {
        auto &descope_fn_sig = get_chimera().get_signature("descope_fix_sig");
        do_descope = reinterpret_cast<ds>(*reinterpret_cast<char **>(descope_fn_sig.data() + 1) + reinterpret_cast<int>(descope_fn_sig.data() + 5));
        set_halo_descoping(false);
        add_tick_event(fix_descoping);
    }
}
