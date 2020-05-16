// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_KEYBOARD_HPP
#define CHIMERA_KEYBOARD_HPP

namespace Chimera {
    struct KeyboardKeys {
    char

    // 0x0
    escape, f1, f2, f3,
    f4, f5, f6, f7,
    f8, f9, f10, f11,
    f12, print_screen, scroll_lock, pause_break,

    // 0x10
    tilde, top_1, top_2, top_3,
    top_4, top_5, top_6, top_7,
    top_8, top_9, top_0, top_minus,
    top_equals, backspace, tab, q,

    // 0x20
    w, e, r, t,
    y, u, i, o,
    p, left_bracket, right_bracket, back_slash,
    caps_lock, a, s, d,

    // 0x30
    f, g, h, j,
    k, l, semicolon, apostrophe,
    enter, left_shift, z, x,
    c, v, b, n,

    // 0x40
    m, comma, period, forward_slash,
    right_shift, left_control, windows, left_alt,
    space, right_alt, unknown, menu,
    right_control, up_arrow, down_arrow, left_arrow,

    // 0x50
    right_arrow, ins, home, page_up,
    del, end, page_down, num_lock,
    num_star, num_forward_slash, num_0, num_1,
    num_2, num_3, num_4, num_5,

    // 0x60
    num_6, num_7, num_8, num_9,
    num_minus, num_plus, num_enter, num_decimal;
    };

    KeyboardKeys &get_keyboard_keys() noexcept;
}

#endif
