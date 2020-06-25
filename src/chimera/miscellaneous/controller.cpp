// SPDX-License-Identifier: GPL-3.0-only

#include "controller.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../output/output.hpp"
#include "../config/ini.hpp"

namespace Chimera {
    extern "C" {
        const void *original_button_text_fn;
        const void *original_axis_text_fn;
        const void *original_pov_text_fn;

        void on_button_text_asm() noexcept;
        void on_axis_text_asm() noexcept;
        void on_pov_text_asm() noexcept;
    }

    void set_up_controller() {
        auto &chimera = get_chimera();
        if(!chimera.get_ini()->get_value_bool("controller.enabled").value_or(false)) {
            return;
        }

        auto *button_text_data = get_chimera().get_signature("button_text_sig").data();
        auto *axis_text_data = get_chimera().get_signature("axis_text_sig").data();
        auto *pov_text_data = get_chimera().get_signature("pov_text_sig").data();

        static Hook button_text_hook, axis_text_hook, pov_text_hook;
        write_function_override(button_text_data, button_text_hook, reinterpret_cast<const void *>(on_button_text_asm), &original_button_text_fn);
        write_function_override(axis_text_data, axis_text_hook, reinterpret_cast<const void *>(on_axis_text_asm), &original_axis_text_fn);
        write_function_override(pov_text_data, pov_text_hook, reinterpret_cast<const void *>(on_pov_text_asm), &original_pov_text_fn);
    }

    #define COPY_AND_RETURN_TEST { \
        const auto *new_value = get_chimera().get_ini()->get_value(test); \
        if(new_value) { \
            for(const char *v = new_value; *v; v++, output++) { \
                *output = *v; \
            } \
            *output = 0; \
            return true; \
        } \
        else { \
            return false; \
        } \
    }

    extern "C" bool on_button_text(char16_t *output, std::uint32_t index) {
        char test[32];
        std::snprintf(test, sizeof(test), "controller.button_%u", index + 1);
        COPY_AND_RETURN_TEST
    }

    extern "C" bool on_axis_text(char16_t *output, std::uint32_t index, std::uint32_t positive) {
        char test[32];
        std::snprintf(test, sizeof(test), "controller.axis_%u_%s", index + 1, positive ? "p" : "n");
        COPY_AND_RETURN_TEST
    }

    extern "C" bool on_pov_text(char16_t *output, std::uint32_t index, std::uint32_t axis) {
        char test[32];
        const char *axis_text;
        switch(axis) {
            case 0: axis_text = "n"; break;
            case 1: axis_text = "ne"; break;
            case 2: axis_text = "e"; break;
            case 3: axis_text = "se"; break;
            case 4: axis_text = "s"; break;
            case 5: axis_text = "sw"; break;
            case 6: axis_text = "w"; break;
            case 7: axis_text = "nw"; break;
            default: axis_text = "n"; break;
        }
        std::snprintf(test, sizeof(test), "controller.pov_%u_%s", index + 1, axis_text);
        COPY_AND_RETURN_TEST
    }
}
