.intel_syntax noprefix

.text

;# Here is the widescreen_element_reposition_letterbox function.
.globl _widescreen_element_reposition_letterbox
_widescreen_element_reposition_letterbox:

push eax
mov eax, [esp + 4]
pushfd
pushad
push eax
call _reposition_letterbox_element
add esp, 4
popad
popfd
pop eax

jmp dword ptr _widescreen_element_position_letterbox_fn
