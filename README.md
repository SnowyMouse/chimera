# Chimera
Chimera is a mod for Halo: Combat Evolved for the PC.

## Installation
Here is the installation procedure for installing Chimera:

    1. Update your installation of Halo to version 1.10.
    2. Back up your game's Strings.dll. Renaming it to `strings-old.dll` works.
    3. Copy strings.dll, chimera.dll, and chimera.ini into your game folder.

    NOTE: Each file goes into the main game folder, NOT controls. If you put
    the files in controls, it will not work.

Uninstalling Chimera is simple:

    1. Delete Chimera's strings.dll, chimera.dll, and (optionally) chimera.ini.
    2. Rename your backed up copy of Halo's Strings.dll back to `strings.dll`.

## Mod support
Chimera does not support any binary mods like HAC2 or Open Sauce. The reason
for this is because they modify the game in similar ways, resulting in them
conflicting with one another. Supporting just one of these mods would mean
extra development time that I don't have. Sorry.

## Features
Chimera provides a number of features and enhancements to the base Halo game.
To ensure the base game is preserved as much as possible, most features are not
turned on by default, and the ones that are only exist to enhance or fix the
game.

### Passive features
These are features that are always on.

#### Interpolation
The game has an issue where object movement is tied to tick rate, thus objects
will never move faster than 30 frames per second. Chimera adds interpolation to
the game for objects. Unlike MCC, it also adds it for particles, too.

#### Limit increase
Chimera increases some of the limits of the game. Note that, unless you play a
map that exceeds these limits, your performance will not be impacted. If you
don't have this on, then maps that exceed the limits will look incorrect and
may not even be playable. Therefore, this feature is always on. Here are the
increases:
- BSP polygon limit boosted to 32767
- Visible object limit boosted to 1024
- Draw distance increased to 2250 world units (~6.86 km)

#### Reduced DRM
Normally, Halo does not allow you to join servers with someone that has the
same CD key. This was done to prevent people from sharing CD keys. However,
this protection has shown to not work and, as a result, is purely an hindrance
to people who have legitimate copies of the game, such as modders. By disabling
the CD key check, you can locally host a game that lets you join yourself.

Note that this feature does not "crack" the game. You still need a valid Halo
installation in order to run the installer for the game as well as update the
game to 1.10.

#### Descope fix
When you take damage while zoomed in, the game zooms back out automatically.
This functionality is broken when in a server, as the client only descopes when
the player takes damage on the client instance rather than if they actually
took damage on the server. Chimera changes this to making it so if you lose
health or shield, you are descoped.

#### Fast loading
Halo Custom Edition CRC32s every map on startup. On a small maps folder, this
does not significantly increase loading time. On a massive maps folder, this
drastically increases loading time, ranging from seconds to minutes.

Some mods "fix" this by storing the CRC32 in a cache, but this negates the
point of it in the first place, as now you can join servers with mismatched
CRC32s. Chimera truly fixes it by CRC32ing the map when you load the map, not
the game.

#### Fast server listing
The game takes forever to query the master server. Chimera speeds it up.

#### 128 MiB map leak fix
If you load a map that exceeds 128 MiB, the game opens the map multiple times
and only closes it once. This results in the map leaking, and eventually, the
game could run out of file descriptors. Chimera fixes this by removing the file
size check.

#### Model LOD fix
The game uses your vertical resolution to determine how detailed to draw a
model. However, this results in levels like The Pillar of Autumn showing the
highest LOD for some mods when it shouldn't, leading to infamous glitches like
the "4K headless chief" glitch. Chimera makes it scale by 480p, instead.

#### FOV fix
There is a bug in the game where, when you increase the vertical resolution of
the game, your FOV gets slightly more narrow, even if you are on the same
aspect ratio. Chimera fixes this so FOVs stay consistent between vertical
resolutions. This does NOT fix low FOV at widescreen aspect ratios, which is
what `chimera_fov` is for.

#### Sun fix
Lens flares are drawn at a set number of pixels regardless of vertical
resolution. Chimera makes it scale by 768p, instead.

#### Custom chat
The Keystone chat is crashy and broken. Chimera adds a replacement chat.

### Commands
Chimera provides a number of extra features that can be turned on. Most of
these features are off by default.

#### Aim assist
Halo's aim assist was broken in the PC release. Chimera fixes it. By default,
this feature is on. It only works for analog sticks, though.

**Usage:** `chimera_aim_assist [true/false]`

#### Anisotropic filtering
This enables the same thing that is done in config.txt but without having to
edit it. Note that this only applies to the level geometry, not individual
objects. You'd need AF enabled externally to do that.

**Usage:** `chimera_af [true/false]`

#### Auto centering
Vehicle auto centering is based on frame rate (0). Chimera allows you to either
fix it by making it based on tick rate (1) or turn it off completely (2).

**Usage:** `chimera_auto_center [0/1/2]`

#### Auto uncrouching
When on the ground, the game disabled crouching if the player was running at
full speed. You can re-enable this behavior if you want.

**Usage:** `chimera_auto_uncrouch [true/false]`

#### Block all bullshit
This feature runs the following commands:
- `chimera_block_buffering 1`
- `chimera_block_camera_shake 1`
- `chimera_block_gametype_indicator 1`
- `chimera_block_gametype_rules 1`
- `chimera_block_hold_f1 1`
- `chimera_block_loading_screen 1`
- `chimera_block_zoom_blur 1`
- `chimera_block_mouse_acceleration 1`

**Usage:** `chimera_block_all_bullshit`

#### Block buffering
This feature is known to reduce input lag, and it works like the feature in
config.txt. You can enable this behavior if you want.

**Usage:** `chimera_block_buffering [true/false]`

#### Block camera shaking
Camera shaking from taking damage or firing weapons is broken in Halo PC. You
can disable it if you want.

**Usage:** `chimera_block_camera_shake [true/false]`

#### Block gametype indicator
The original Xbox version of the game didn't have the gametype indicator, and
it looks out of place in most custom maps. You can disable it, if you want.

**Usage:** `chimera_block_gametype_indicator [true/false]`

#### Block gametype rules
In Halo Custom Edition, if you join a server, you see the gametype rules for a
few seconds (or part of a second depending on frame rate). If this is annoying,
you can disable it.

**Usage:** `chimera_block_gametype_rules [true/false]`

#### Block hold F1
When you join a server, you'll see a "Hold F1 for score" (or whatever button is
bound to score screens). You can disable this if you want.

**Usage:** `chimera_block_hold_f1 [true/false]`

#### Block letterbox
Cutscenes have a letterbox. You can disable it if you want.

**Usage:** `chimera_block_letterbox [true/false]`

#### Block loading screen
When you join a server, you are blinded for a few seconds by a loading screen,
and this can make you susceptible to being cheaply killed. This disables that
loading screen.

**Usage:** `chimera_block_loading_screen [true/false]`

#### Block mouse acceleration
Halo uses raw input, thus it bypasses the mouse acceleration setting of your
operating system (unless you run the game in Wine). However, Halo adds its own
mouse acceleration on top of direct input. You can disable this if you want. By
default, mouse acceleration is blocked.

**Usage:** `chimera_block_mouse_acceleration [true/false]`

#### Block server IP
This hides the server IP from view in the score screen. This is good for
streamers who don't want people to join what server they are in. You can enable
this feature if you want.

**Usage:** `chimera_block_server_ip [true/false]`

#### Block zoom blur
Halo's zoom blur looks like ass on high resolutions. This feature removes it
without requiring you to use DisableAlphaRenderTargets or safe mode.

**Usage:** `chimera_block_zoom_blur [true/false]`

#### Budget
Show the different budgets in Halo, including object limit and BSP polygon
limit.

**Usage:** `chimera_budget [true/false]`

#### Chat: Block IPs
Block IPs from being shown in the custom chat. This is good for streamers, so
their IPs don't get shown on stream. It requires custom chat to be enabled.

**Usage:** `chimera_chat_block_ips [true/false]`

#### Chat: Block server messages
Block all messages from the server except for if you're using `rcon` in the
console.

**Usage:** `chimera_chat_block_server_messages [true/false]`

#### Chat: Chat color help
Show fancy chat colors you can use.

**Usage:** `chimera_chat_color_help [true/false]`

#### Console prompt color
Set the console input color.

**Usage:** `chimera_console_prompt_color [<red> <green> <blue>]`

#### Deadzones
Set deadzones for analog input.

**Usage:** `chimera_deadzones [deadzones]`

#### Devmode
Enable devmode commands without blocking you from multiplayer or requiring any
command line arguments.

**Usage:** `chimera_devmode [true/false]`

#### Diagonals
Set the diagonals for moving with an analog stick. By default, this is set to
0.75, as Halo's default diagonals make it difficult to move with an analog
stick. This is because Halo converts it to digital input, as the netcode does
not support analog movement.

**Usage:** `chimera_diagonals [true/false]`

#### Enable console
Enable the console without command line arguments. This is on by default.

**Usage:** `chimera_enable_console [true/false]`

#### FOV
Set FOV. End with "v" to lock to a vertical FOV, or use "auto" to set to an
automatic FOV. Using a vertical FOV over a horizontal one helps prevent the
game from distorting your field of view when switching aspect ratios.

**Usage:** `chimera_fov [fov]`

#### HUD kill feed
Put the kill feed in the HUD. Halo puts it in the chat, normally. This feature
is on by default.

**Usage:** `chimera_hud_kill_feed [true/false]`

#### Language
Chimera is localized for both English and, thanks to help from the community,
Spanish. By default, it is English.

**Usage:** `chimera_language [en/es]`

#### Model detail
Change the model detail. Higher values increase LODs. This will not make models
extra detailed. Instead, it will change the threshold for when higher LODs
appear. Extremely high values will cause the aforementioned "4K headless chief"
glitch, except at any vertical resolution this time. By default, this is set to
1.0.

**Usage:** `chimera_model_detail [detail]`

#### Mouse sensitivity
Set the mouse sensitivity. This is more granular than using Halo's settings. It
is recommended to have mouse acceleration disabled if using low values.

Halo | Horizontal | Vertical
-----|------------|--------
10   | 4          | 2
9    | 4          | 2
8    | 3          | 1.5
7    | 2          | 1
6    | 1.5        | 0.75
5    | 1.25       | 0.625
4    | 1          | 0.5
3    | 0.75       | 0.375
2    | 0.5        | 0.25
1    | 0.25       | 0.125

**Usage:** `chimera_mouse_sensitivity [off | <horizontal> <vertical>]`

#### Player info
Show player info for the given player. This does not work outside of client
hosted games.

**Usage:** `chimera_player_info <player>`

#### Set name
Set your in-game name.

**Usage:** `chimera_set_name [name]`

#### Show coordinates
Show your in-game coordinates.

**Usage:** `chimera_show_coordinates [true/false]`

#### Show FPS
Show your in-game FPS.

**Usage:** `chimera_show_fps [true/false]`

#### Shrink empty weapons
Make empty weapons appear smaller so they're easier to spot.

**Usage:** `chimera_shrink_empty_weapons [true/false]`

#### Split screen HUD
Override the HUD with the split screen HUD.

**Usage:** `chimera_split_screen_hud [true/false]`

#### Throttle FPS
Throttle Halo's frame rate. Set to 0 to disable.

**Usage:** `chimera_throttle_fps [FPS]`

#### TPS
Set the game's tick rate. This will not work outside of client hosted games.
For reference, the game runs at 30 TPS by default.

**Usage:** `chimera_tps [TPS]`

#### Uncap cinematic
Uncap the cinematic frame rate from 30 FPS.

**Usage:** `chimera_uncap_cinematic [true/false]`

#### Widescreen fix
Fix the scaling of HUD, menu, and text elements for wider aspect ratios. This
may break some maps, and this will not be fixed as fixing one thing will break
another thing due to the nature of these fixes.

**Usage:** `chimera_widescreen_fix [true/false]`
