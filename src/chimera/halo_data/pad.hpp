// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_PAD_HPP
#define CHIMERA_PAD_HPP

#define MAKE_PAD2(size, name) char zzz_padding_ ## size ## _l ## name [ size ]
#define MAKE_PAD(size, name) MAKE_PAD2(size, name)
#define PAD(size) MAKE_PAD(size, __LINE__)

#define MAKE_PAD_BIT2(type, size, name) type zzz_padding_ ## size ## _l ## name : size
#define MAKE_PAD_BIT(type, size, name) MAKE_PAD_BIT2(type, size, name)
#define PAD_BIT(type, size) MAKE_PAD_BIT(type, size, __LINE__)

#endif
