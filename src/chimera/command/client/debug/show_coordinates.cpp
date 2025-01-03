// SPDX-License-Identifier: GPL-3.0-only

#include "../../../event/frame.hpp"
#include "../../../output/draw_text.hpp"
#include "../../../output/output.hpp"
#include "../../../command/command.hpp"
#include "../../../halo_data/player.hpp"
#include "../../../halo_data/object.hpp"
#include "../../../signature/signature.hpp"
#include "../../../chimera.hpp"

namespace Chimera {
    static void show_coordinates() noexcept;
    bool show_coordinates_command(int argc, const char **argv) noexcept {
        static bool enabled = false;

        // Check if the user supplied an argument.
        if(argc) {
            // Check the value of the argument and see if it differs from the current setting.
            bool new_enabled = STR_TO_BOOL(argv[0]);
            if(new_enabled != enabled) {
                if(new_enabled) {
                    add_preframe_event(show_coordinates, EventPriority::EVENT_PRIORITY_FINAL);
                }
                else {
                    remove_preframe_event(show_coordinates);
                }
                enabled = new_enabled;
            }
        }

        console_output(BOOL_TO_STR(enabled));
        return true;
    }

    static ColorARGB blue = ColorARGB { 0.7, 0.45, 0.72, 1.0 };

    static void show_coordinates() noexcept {
        auto *player = PlayerTable::get_player_table().get_client_player();
        auto *camx = *reinterpret_cast<float **>(get_chimera().get_signature("camera_rotation_sig").data() + 0xF);
        auto *camy = *reinterpret_cast<float **>(get_chimera().get_signature("camera_rotation_sig").data() + 0x14);

        char rotation[256] = "";
        char center[256] = "Dead";
        char relative[256] = "";

        // Check if the player exists. If so, show the coordinates of them (and their object's parent if they have one)
        if(player) {
            auto &object_table = ObjectTable::get_object_table();
            auto *object = object_table.get_dynamic_object(player->object_id);
            if(object) {
                std::snprintf(rotation, sizeof(center), "Rotation: %.10f, %.10f", *camx , *camy);
                std::snprintf(center, sizeof(center), "Center: %.05f, %.05f, %.05f", object->center_position.x, object->center_position.y, object->center_position.z);

                auto *vehicle_object = object_table.get_dynamic_object(object->parent);
                if(vehicle_object) {
                    std::snprintf(relative, sizeof(relative), "Absolute (Parent): %.05f, %.05f, %.05f", vehicle_object->position.x, vehicle_object->position.y, vehicle_object->position.z);
                }
                else {
                    std::snprintf(relative, sizeof(relative), "Absolute: %.05f, %.05f, %.05f", object->position.x, object->position.y, object->position.z);
                }
            }
        }

        auto font = GenericFont::FONT_CONSOLE;
        int font_size = font_pixel_height(font);
        apply_text(rotation, -240, 220 - font_size * 3, 480, 200, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
        apply_text(center, -240, 220 - font_size * 2, 480, 200, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
        apply_text(relative, -240, 220 - font_size, 480, 200, blue, font, FontAlignment::ALIGN_CENTER, TextAnchor::ANCHOR_CENTER);
    }
}
