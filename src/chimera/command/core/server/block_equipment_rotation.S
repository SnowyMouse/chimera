;# SPDX-License-Identifier: GPL-3.0-only

.intel_syntax noprefix

.text

;# Load 0
.globl _null_equipment_spawn_rotation
_null_equipment_spawn_rotation:
    sub esp, 4
    mov dword ptr [esp], 0
    fld dword ptr [esp]
    add esp, 4
    ret
