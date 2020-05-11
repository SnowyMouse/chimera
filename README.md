# Chimera
Chimera is a mod for Halo: Combat Evolved for the PC. It is licensed under
version 3.0 of the GNU General Public License. See LICENSE for more
information.

## Table of contents
- [Installation](#installation)
- [System requirements](#system-requirements)
- [Mod support](#mod-support)
- [Features](#features)
- [FAQ](#faq)

## Installation
Here is the installation procedure for installing Chimera:

1. Update your installation of Halo to version 1.10.
2. Back up your game's Strings.dll. Renaming it to `strings-old.dll` works.
3. Copy strings.dll and chimera.ini into your game folder.

NOTE: Each file goes into the main game folder, NOT controls. If you put the
files in controls, it will not work.

Uninstalling Chimera is simple:

1. Delete Chimera's strings.dll and (optionally) chimera.ini.
2. Rename your backed up copy of Halo's Strings.dll back to `strings.dll`.

## Mod support
Chimera does not support any binary mods like HAC2 or Open Sauce. The reason
for this is because they modify the game in similar ways, resulting in them
conflicting with one another. Supporting just one of these mods would mean
extra development time that I don't have. Sorry.

That said, you can create a mods folder and drop in binary mods in there.

## Features
Chimera provides a number of features and enhancements to the base Halo game.
To ensure the base game is preserved as much as possible, most features are not
turned on by default, and the ones that are only exist to enhance or fix the
game.
- [Passive features](#passive-features)
- [Ini features](#ini-features)
- [Commands](#commands)

### Passive features
These are features that are always on.
- [Interpolation](#interpolation)
- [Limit increase](#limit-increase)
- [Compressed maps](#compressed-maps)
- [Reduced DRM](#reduced-drm)
- [Descope fix](#descope-fix)
- [Fast loading](#fast-loading)
- [Fast server listing](#fast-server-listing)
- [128 MiB map leak fix](#128-mib-map-leak-fix)
- [Model LOD fix](#model-lod-fix)
- [FOV fix](#fov-fix)
- [Sun fix](#sun-fix)
- [Custom chat](#custom-chat)
- [NVIDIA camo fix](#nvidia-camo-fix)
- [Contrail fix](#contrail-fix)
- [Motion sensor fix](#motion-sensor-fix)
- [Map downloading](#map-downloading)

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

#### Compressed maps
Chimera supports [Invader](https://github.com/Kavawuvi/invader)-compressed
maps. This allows you to take advantage of the blazingly fast zstd compression
algorithm, drastically cutting down map size.

If [maps in RAM](#memory-settings) is enabled, compressed maps will be
decompressed directly into RAM. Otherwise, temp files will be used.

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

#### NVIDIA camo fix
Chimera fixes a bug where, if the GPU is an NVIDIA graphics card, it uses alpha
blended camo instead of the liquid camo shader.

#### Contrail fix
Chimera fixes a bug where contrails are bugged at high frame rates.

#### Motion sensor fix
Chimera fixes a bug where the motion sensor fade is incorrect at >30 FPS

#### Map downloading
Chimera will automatically attempt to download maps off of the
[HAC2 Map repo](http://maps.halonet.net/maplist.php) upon joining a server that
is playing a map you don't have. These maps are stored under `chimera/maps` in
your Halo profiles folder.

### Ini features
Chimera has a very customizable chimera.ini text file that you can use to
further tweak your game.

Note that .ini comments start with a `;` and some features are commented out
(disabled) like so: `;enabled=1`. This is so if you want to enable a feature,
you simply have to remove the semicolon.

Below are the features you can take advantage of.
- [Halo settings](#halo-settings)
- [Memory settings](#memory-settings)
- [Video mode](#video-mode)
- [Custom chat settings](#custom-chat-settings)
- [Hotkeys](#hotkeys)

#### Halo settings
These features exist to prevent the need to use arguments for the Halo
executable.
- `path` (changes the profile path location)
- `main_menu_music` (disables the music in the main menu)
- `server_port` (set the default server port)
- `client_port` (set the default client port)

#### Memory settings
This feature exists to allow you to take advantage larger amounts of RAM,
reducing hiccups and stutters, especially on slower storage. However, you will
need an LAA-patched executable to use this feature.
- `enable_map_memory_buffer` (enables loading maps directly into RAM)
- `benchmark` (shows a benchmark whenever a map is loaded)

#### Video mode
These settings allow you to change Halo's video settings without requiring
vidmode. You can even use refresh rates in excess of 120 Hz.
- `enabled` (enables the feature)
- `width` (default width in pixels)
- `height` (default height in pixels)
- `refresh_rate` (refresh rate in Hertz)
- `vsync` (enable double buffer vSync to reduce tearing)
- `windowed` (display Halo in a window)

#### Custom chat settings
This allows you to customize the different colors, positioning, and timings of
Chimera's chat. When we said it was a custom chat, we meant it. See the
included chimera.ini file for more information on these settings.

#### Hotkeys
This allows you to execute Chimera commands, Halo commands, or Halo scripts
when the key combination is invoked. See the included chimera.ini file for more
information on these settings.

### Commands
Chimera provides a number of extra features that can be turned on. Most of
these features are off by default. Commands are stored in preferences.txt in
the chimera folder created by Chimera.
- [Aim assist](#aim-assist)
- [Anisotropic filtering](#anisotropic-filtering)
- [Auto centering](#auto-centering)
- [Auto uncrouching](#auto-uncrouching)
- [Block all bullshit](#block-all-bullshit)
- [Block buffering](#block-buffering)
- [Block camera shaking](#block-camera-shaking)
- [Block extra weapon](#block-extra-weapon)
- [Block gametype indicator](#block-gametype-indicator)
- [Block gametype rules](#block-gametype-rules)
- [Block hold F1](#block-hold-f1)
- [Block letterbox](#block-letterbox)
- [Block loading screen](#block-loading-screen)
- [Block mouse acceleration](#block-mouse-acceleration)
- [Block server IP](#block-server-ip)
- [Block zoom blur](#block-zoom-blur)
- [Bookmark add](#bookmark-add)
- [Bookmark delete](#bookmark-delete)
- [Bookmark connect](#bookmark-connect)
- [Bookmark list](#bookmark-list)
- [History connect](#history-connect)
- [History list](#history-list)
- [Budget](#budget)
- [Chat: Block IPs](#chat-block-ips)
- [Chat: Block server messages](#chat-block-server-messages)
- [Chat: Chat color help](#chat-chat-color-help)
- [Console prompt color](#console-prompt-color)
- [Deadzones](#deadzones)
- [Devmode](#devmode)
- [Diagonals](#diagonals)
- [Enable console](#enable-console)
- [FOV](#fov)
- [HUD kill feed](#hud-kill-feed)
- [Language](#language)
- [Model detail](#model-detail)
- [Mouse sensitivity](#mouse-sensitivity)
- [Player info](#player-info)
- [Player list](#player-list)
- [Set name](#set-name)
- [Show coordinates](#show-coordinates)
- [Show FPS](#show-fps)
- [Shrink empty weapons](#shrink-empty-weapons)
- [Split screen HUD](#split-screen-hud)
- [Spectate](#spectate)
- [Spectate back](#spectate-back)
- [Spectate forward](#spectate-forward)
- [Spectate team only](#spectate-team-only)
- [Teleport](#teleport)
- [Throttle FPS](#throttle-fps)
- [TPS](#tps)
- [Uncap cinematic](#uncap-cinematic)
- [Unblock all extra weapons](#unblock-all-extra-weapons)
- [Widescreen fix](#widescreen-fix)

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
In stock Halo, vehicle auto centering is based on frame rate. Chimera allows
you to either fix it or turn it off completely, as this can be annoying.
- 0 is Halo's normal behavior
- 1 is fixed behavior (tick-based)
- 2 is no auto centering whatsoever

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

#### Block extra weapon
Block your currently held weapon if you have more than two weapons. Unblock
weapons with [Unblock all extra weapons](#unblock-all-extra-weapons). This will
last until the next game.

**Usage:** `chimera_block_extra_weapon`

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
loading screen. By default, Chimera disables the loading screen due to this,
but you can turn it back on if you want.

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

#### Bookmark add
Add a server to the server bookmarks list. If no server details are given, then
the last server joined is added. You can use a domain name in place of an IP.

**Usage:** `chimera_bookmark_add [ip:port [password]]`

#### Bookmark delete
Delete a server from the server bookmarks list. Either an index or an ip:port
can be used. If no server details are given, then the last server joined is
deleted.

**Usage:** `chimera_bookmark_delete [ip:port | index]`

#### Bookmark connect
Connect to a server in the bookmarks list.

**Usage:** `chimera_bookmark_connect <index>`

#### Bookmark list
List and query all bookmarked servers.

**Usage:** `chimera_bookmark_list`

#### History connect
Connect to a server in the recent server list.

**Usage:** `chimera_history_connect <index>`

#### History list
List and query all recent servers.

**Usage:** `chimera_history_list`

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
game from distorting your field of view when switching aspect ratios. On Halo
Trial, this feature will be set to "auto" by default.

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
Show player info for the given player.

**Usage:** `chimera_player_info <player>`

#### Player list
List player indices.

**Usage:** `chimera_player_list`

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

#### Spectate
Spectate a specific player. Use `0` to disable spectating.

**Usage:** `chimera_spectate <index>`

#### Spectate next
Spectate the next player. This is intended to be used with hotkeys.

**Usage:** `chimera_spectate_next`

#### Spectate previous
Spectate the previous player. This is intended to be used with hotkeys.

**Usage:** `chimera_spectate_back`

#### Spectate team only
Set whether or not chimera_spectate_next or chimera_spectate_previous selects
players not on the same team. This has no effect on non-team games.

**Usage:** `chimera_spectate_team_only <enabled>`

#### Teleport
Teleport you or, if you're hosting a server, someone else to the given X/Y/Z
coordinates or player.

**Usage:** `chimera_teleport [#] <<#> | <x> <y> <z>>`

#### Throttle FPS
Throttle Halo's frame rate. Set to 0 to disable. By default, this is set to 300
on Halo Trial.

**Usage:** `chimera_throttle_fps [FPS]`

#### TPS
Set the game's tick rate. This will not work outside of client hosted games.
For reference, the game runs at 30 TPS by default.

**Usage:** `chimera_tps [TPS]`

#### Unblock all extra weapons
Unblock all weapons blocked with [Block extra weapon](#block-extra-weapon)

**Usage:** `chimera_unblock_all_extra_weapons`

#### Uncap cinematic
Uncap the cinematic frame rate from 30 FPS. This will be turned on by default
on Halo Trial.

**Usage:** `chimera_uncap_cinematic [true/false]`

#### Widescreen fix
Fix the scaling of HUD, menu, and text elements for wider aspect ratios. This
may break some maps, and this will not be fixed as fixing one thing will break
another thing due to the nature of these fixes. On Halo Trial, this will be on
by default.

**Usage:** `chimera_widescreen_fix [true/false]`

## FAQ

### Will Chimera run on my system?
Chimera isn't too taxing, especially on modern systems, but it can make Halo
harder to run if you're on very ancient hardware.

There are the hard requirements:

- OS (if on Windows): Windows 7 or newer
- Wine (if on Linux): Wine 4.0 or newer\*

\*We test on the latest wine-staging 5.0 version, but 4.0 should work.

If your PC does not at least meet the above requirements, Chimera may fail to
run properly due to the underlying API being older.

There are the recommendations for running Chimera optimally:

- CPU (Intel): Intel Core i3-2100 (3.1 GHz) or better
- CPU (AMD): AMD FX 6100 (3.3 GHz) or better
- RAM: 8 GiB or more\*
- GPU (Nvidia): Nvidia GeForce 8500 GT
- GPU (AMD): ATI Radeon HD 5670 or better
- GPU (Intel): *see Nvidia and AMD requirements*

\* Halo, even with Chimera, is 32-bit, so it won't technically use more than
4 GiB, but 8 GiB is recommended so Chimera has enough RAM.

The newest of the above requirements is the AMD CPU (FX 6100) which was
released in the year 2011 - over seven years ago at the time of writing this
sentence.

### Can I use Chimera under a license besides GNU GPL version 3?
Short answer: Probably not.

Long answer: It's not impossible. You would first need to ask *all* of the
developers of upstream Chimera for permission. Chimera, however, also depends
on [Invader](https://github.com/Kavawuvi/invader) for map compression, and this
is licensed under the GPL as well. So, you'd have to either remove the Invader
dependency (thus no map compression - required for map downloading!) or ask
*all* of the developers of upstream Invader for permission, too.

### What is the password to the archive?
Read PASSWORD.txt.
