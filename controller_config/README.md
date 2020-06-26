To use a controller config, paste the contents of the entire text file to the
`[controller]` section in chimera.ini. Set `enabled=1` or uncomment `enabled=1`
if using the included chimera.ini. For example:

```ini
[controller]

;===============================================================================
; Controller settings
;
; This is used to configure gamepads.
;===============================================================================

; Enable this to use this feature
enabled=1

; <PASTE HERE>
```

Note that the Windows and Linux drivers map the buttons/axes differently. For
example, Xbox One controllers have the left & right triggers mapped to one axis
on Windows, but they're mapped to separate axes on Linux. The Home button is
also mapped on Linux but not on Windows.
