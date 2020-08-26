// SPDX-License-Identifier: GPL-3.0-only

#include "damage.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/damage.hpp"

namespace Chimera {
    static std::vector<Event<DamageEventFunction>> damage_events;

    static void enable_damage_hook();

    // Functions for determining if we bypass or not
    static bool should_bypass = false;
    void set_bypass_damage_events(bool bypass) noexcept {
        should_bypass = bypass;
    }
    bool get_bypass_damage_events() noexcept {
        return should_bypass;
    }

    extern "C" {
        void on_damage_asm();
        const void *do_continue_damage_effect;
    }

    void add_damage_event(const DamageEventFunction function, EventPriority priority) {
        // Remove if exists
        remove_damage_event(function);

        // Enable hook if necessary
        enable_damage_hook();

        // Add the event
        damage_events.emplace_back(Event<DamageEventFunction> { function, priority });
    }

    void remove_damage_event(const DamageEventFunction function) {
        for(std::size_t i = 0; i < damage_events.size(); i++) {
            if(damage_events[i].function == function) {
                damage_events.erase(damage_events.begin() + i);
                return;
            }
        }
    }

    extern "C" bool do_damage_event(ObjectID *object, DamageObjectStructThing *damage_thing) {
        if(should_bypass) {
            return true;
        }
        bool allow = true;
        call_in_order_allow(damage_events, allow, *object, damage_thing->damage_tag_id, damage_thing->multiplier, damage_thing->causer_player, damage_thing->causer_object);
        return allow;
    }

    static void enable_damage_hook() {
        // Enable if not already enabled.
        static bool enabled = false;
        if(enabled) {
            return;
        }
        enabled = true;

        // Add the hook
        static Hook hook;
        write_function_override(get_chimera().get_signature("apply_damage_sig").data(), hook, reinterpret_cast<const void *>(on_damage_asm), &do_continue_damage_effect);
    }
}
