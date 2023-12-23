# Chimera
Chimera is the update to Halo: Combat Evolved for the PC that we should have had
but never got.

It is licensed under version 3.0 of the GNU General Public License. See LICENSE
for more information.

The official repository is hosted at https://github.com/SnowyMouse/chimera




## Table of contents
- [Installation](#installation)
- [Mod support](#mod-support)
- [Features](#features)
- [FAQ](#faq)





## Installation
Here is the installation procedure for installing Chimera:

1. Update Halo to version 1.10 (unless you're playing the trial).
2. Back up your game's Strings.dll. Renaming it to `strings-old.dll` works.
3. Copy strings.dll, chimera.ini, and fonts into your game folder.
4. Optionally add a mods folder to load custom dll mods.

> **NOTE**: Each file goes into the main game folder, NOT controls. If you put
> the files in controls, it will not work.

Uninstalling Chimera is simple:

1. Delete Chimera's strings.dll and optionally chimera.ini, fonts, and/or mods.
2. Rename your backed up copy of Halo's Strings.dll back to `strings.dll`.





## Mod support
You can install binary mods (i.e. dlls) by creating a folder called "mods" and
copying the dlls into the directory.

> **NOTE**: Chimera does *not* support dll mods including HAC2 or Open Sauce.
> This is because they modify the game in similar ways, resulting in them
> conflicting with one another. Supporting just one of these mods would mean
> extra development time that we don't have. Sorry.





## Features
Chimera provides a number of features and enhancements to the base Halo game.
To ensure the base game is preserved as much as possible, most features are not
turned on by default, and the ones that are only exist to enhance or fix the
game.
- [Passive features](#passive-features)
- [Custom Edition map support on retail](#custom-edition-map-support-on-retail)
- [Ini features](#ini-features)
- [Commands](#commands)


### Passive features
These are features that are always on.
- [Interpolation](#interpolation)
- [Limit increase](#limit-increase)
- [Compressed maps](#compressed-maps)
- [Camera shake fix](#camera-shake-fix)
- [Reduced DRM](#reduced-drm)
- [Descope fix](#descope-fix)
- [Extended description fix](#extended-description-fix)
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
- [Auto center fix](#auto-center-fix)
- [Fog fix](#fog-fix)
- [Model detail fix](#model-detail-fix)
- [HUD numbers fix](#hud-numbers-fix)
- [Weapon swap ticks fix](#weapon-swap-ticks-fix)
- [Name fade fix](#name-fade-fix)
- [Scoreboard fade fix](#scoreboard-fade-fix)
- [Timer offset fix](#timer-offset-fix)
- [Sane default Halo settings](#sane-default-halo-settings)
- [Removed update check](#removed-update-check)
- [Removed Watson](#removed-watson)
- [Multi-team vehicle fix](#multi-team-vehicle-fix)
- [Map downloading](#map-downloading)
- [Lua scripting](#lua-scripting)

#### Interpolation
The game has an issue where object movement is tied to tick rate, thus objects
will never move faster than 30 frames per second. Chimera adds interpolation to
the game for objects. Unlike MCC, it also adds it for cutscenes and plasma
projectiles, too.

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
Chimera supports [Invader](https://github.com/SnowyMouse/invader)-compressed
maps. This allows you to take advantage of the blazingly fast zstd compression
algorithm, drastically cutting down map size.

If [maps in RAM](#memory-settings) is enabled, compressed maps will be
decompressed directly into RAM. Otherwise, temp files will be used (placed in
Chimera's folder).

#### Camera shake fix
Chimera fixes a bug where camera shaking does not work at high frame rates.

#### Reduced DRM
Normally, Halo does not allow you to join servers with someone that has the
same CD key. This was done to prevent people from sharing CD keys. However,
this protection has shown to not work and, as a result, is purely an hindrance
to people who have legitimate copies of the game, such as modders. By disabling
the CD key check, you can locally host a game that lets you join yourself. Also,
because a large number of players do not own legitimate copies of the game,
having the CD key check in place will likely prevent you from getting any
players in the first place.

Note that this feature does not "crack" the game. You still need a valid Halo
installation in order to run the installer for the game as well as update the
game to 1.10.

#### Descope fix
When you take damage while zoomed in, the game zooms back out automatically.
This functionality is broken when in a server, as the client only descopes when
the player takes damage on the client instance rather than if they actually
took damage on the server. Chimera changes this to making it so if you lose
health or shield, you are descoped.

#### Extended description fix
Chimera fixes a bug where the "About" button of the profile settings menu
doesn't display the correct bitmap in the extended description.

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

#### Fog fix
Fog in maps such as Assault on the Control Room's sky fog now works as intended.

#### Model detail fix
The "detail after reflection" flag in model shaders now works as intended.

#### HUD numbers fix
HUD numbers are no longer drawn oversized if modified to be a higher resolution.

#### Weapon swap ticks fix
Weapons are now picked up with the correct timing when holding the action key.

#### Auto center fix
Chimera fixes a bug where the game does auto look centering based on frame rate.
This would result in vehicles being difficult to handle at high frame rates.

#### Name fade fix
Names now fade in/out correctly at high frame rates when staring at a player.

#### Scoreboard fade fix
Scoreboard now fades in/out correctly at high frame rates.

#### Timer offset fix
Halo PC has a bug where equipment spawns `floor(8.1n)` ticks (where n = the
netgame equipment index). This bug is fixed.

#### Sane default Halo settings
Halo's default video (not including resolution) and audio settings are set to
more appropriate settings for modern PCs:

Setting               | New default | Note
----------------------|-------------|-------------------------------------------
FRAMERATE             | NO VSYNC    |
SPECULAR              | YES         |
SHADOWS               | YES         |
DECALS                | YES         |
PARTICLES             | HIGH        |
TEXTURE QUALITY       | HIGH        |
HARDWARE ACCELERATION | YES         | Requires [dsoal]
SOUND QUALITY         | HIGH        |
ENVIRONMENTAL AUDIO   | EAX         | Requires [dsoal]
SOUND VARIETY         | HIGH        |

[dsoal]: https://github.com/kcat/dsoal

#### Removed update check
The check for updates that occurs when clicking on "INTERNET" in joining or
creating games is disabled.

#### Removed Watson
When Halo PC crashes, it launches a program called "Watson" to attempt to report
errors to a server that has not been online in years. This is a waste of time,
and it presents a minor annoyance for people who are unaware that you can simply
remove the Watson folder. Therefore, Chimera removes the functionality
altogether.

#### Multi-team vehicles fix
Servers with Chimera's [chimera_allow_all_passengers](#allow-all-passengers),
multi-team vehicles, or some equivalent feature enabled will not desync a client
with this fix.

#### Map downloading
Chimera will automatically attempt to download maps off of the
[HAC2 Map repo](http://maps.halonet.net/maplist.php) upon joining a server that
is playing a map you don't have. A different map source can be configured using
the `download_template` setting. These maps are stored under `chimera/maps` in
your Halo profiles folder by default (see the `download_map_path` setting).

#### Lua scripting
Lua scripting ported from Chimera -572. Scripts in the global folder are loaded
on startup. They remain permanently loaded unless the user uses the scripts
reload command. Scripts in the map folder is loaded when a map is loaded and
unloaded when a map is unloaded. Scripts may also be contained inside of map
files.

### Ini features
Chimera has a very customizable chimera.ini text file that you can use to
further tweak your game.

Note that .ini comments start with a `;` and some features are commented out
(disabled) like so: `;enabled=1`. This is so if you want to enable a feature,
you simply have to remove the semicolon.

Below are the features you can take advantage of.
- [Halo settings](#halo-settings)
- [Scoreboard settings](#scoreboard-settings)
- [Name settings](#name-settings)
- [Memory settings](#memory-settings)
- [Font override settings](#font-override-settings)
- [Video mode settings](#video-mode-settings)
- [Controller settings](#controller-settings)
- [Custom chat settings](#custom-chat-settings)
- [Hotkeys](#hotkeys)

#### Halo settings
These features exist to modify how Halo is initialized.
- `path` (changes the profile path location)
- `map_path` (changes the path maps will be loaded from)
- `download_map_path` (changes the path custom maps are downloaded to)
- `exec` (changes the init text file loaded)
- `server_port` (set the default server port)
- `client_port` (set the default client port)
- `console` (enable console)
- `main_menu_music` (disables the music in the main menu)
- `background_playback` (allow Halo to play in the background)
- `multiple_instances` (allow Halo to spawn in multiple instances)
- `hash` (set a custom hash or `%` for a random one)

#### Scoreboard settings
This exists to manipulate the scoreboard
- `font` (change the font)
- `fade_time` (change the fade in and fade out time)

#### Name settings
This exists to manipulate the names shown when looking at players.
- `font` (change the font)

#### Memory settings
This feature exists to allow you to take advantage larger amounts of RAM,
reducing hiccups and stutters, especially on slower storage. However, you will
need an LAA-patched executable to use this feature.
- `enabled` (enables loading maps directly into RAM)
- `map_size` (size of buffer in MiB for loading maps)
- `benchmark` (shows a benchmark whenever a map is loaded)
- `download_font` (change the font used for downloading)
- `download_template` (URL template used to downloading maps)
- `download_retail_maps` (allow downloading of retail Halo PC maps - UNSAFE)
- `load_embedded_lua` (enables loading Lua scripts embedded into map files)

#### Font override settings
This allows you to override fonts Chimera uses with system fonts, allowing you
to choose the size, family, drop shadow, and weight (boldness) of each font. See
the included chimera.ini for more information on these. You can also optionally
add a fonts folder to your game install location to use loose ttf, otf, and ttc
files.

#### Video mode settings
These settings allow you to change Halo's video settings without requiring
vidmode. You can even use refresh rates in excess of 120 Hz.
- `enabled` (enables the feature)
- `width` (default width in pixels)
- `height` (default height in pixels)
- `refresh_rate` (refresh rate in Hertz)
- `vsync` (enable double buffer vSync to reduce tearing)
- `windowed` (display Halo in a window)
- `borderless` (display Halo in borderless fullscreen - required windowed mode)

#### Controller settings
These settings allow you to configure gamepads. See the included chimera.ini
file for more information on these settings. You can also get [premade configs]
in the Chimera repository if you want to use a premade configuration file.

[example configs]: https://github.com/SnowyMouse/chimera/tree/master/controller_config

#### Custom chat settings
This allows you to customize the different colors, positioning, and timings of
Chimera's chat. When we said it was a custom chat, we meant it. See the
included chimera.ini file for more information on these settings.

#### Hotkeys
This allows you to execute Chimera commands, Halo commands, or Halo scripts
when the key combination is invoked. See the included chimera.ini file for more
information on these settings.


### Custom Edition map support on retail
Chimera can enable Halo Custom Edition map support when playing the base Halo PC
game.

To enable this, put the following files to your maps folder:
- `custom_bitmaps.map` (Halo Custom Edition bitmaps.map file)
- `custom_loc.map`     (Halo Custom Edition loc.map file)
- `custom_sounds.map`  (Halo Custom Edition sounds.map file)

You can obtain these files from the Halo Custom Edition installer. Then, simply
rename them to have the `custom_` prefixes and copy them into your maps folder.

> **NOTE:** If you do not wish to install Halo Custom Edition, you can extract
> these resource map files from the installer using archiver programs such as
> [7-Zip](https://www.7-zip.org/download.html) (Windows) or
> [p7zip](https://wiki.archlinux.org/index.php/P7zip) (POSIX).

> **CAUTION:** Do NOT overwrite your original bitmaps.map or sounds.map files.
> You need these to load Halo PC maps. Also, Chimera will *not* enable this
> feature if *any* of the above files are missing.


### Commands
Chimera provides a number of extra features that can be turned on. Most of
these features are off by default. Commands are stored in preferences.txt in
the chimera folder created by Chimera.
- [Aim assist](#aim-assist)
- [Allow all passengers](#allow-all-passengers)
- [Anisotropic filtering](#anisotropic-filtering)
- [Auto uncrouching](#auto-uncrouching)
- [Block all bullshit](#block-all-bullshit)
- [Block auto center](#block-auto-center)
- [Block buffering](#block-buffering)
- [Block button quotes](#block-button-quotes)
- [Block equipment rotation](#block-equipment-rotation)
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
- [FoV](#fov)
- [FP reverb](#fp-reverb)
- [Language](#language)
- [Model detail](#model-detail)
- [Mouse sensitivity](#mouse-sensitivity)
- [Player info](#player-info)
- [Player list](#player-list)
- [Send chat message](#send-chat-message)
- [Set color](#set-color)
- [Set name](#set-name)
- [Show coordinates](#show-coordinates)
- [Show FPS](#show-fps)
- [Shrink empty weapons](#shrink-empty-weapons)
- [Spam to join](#spam-to-join)
- [Split screen HUD](#split-screen-hud)
- [Spectate](#spectate)
- [Spectate next](#spectate-next)
- [Spectate previous](#spectate-previous)
- [Spectate team only](#spectate-team-only)
- [Teleport](#teleport)
- [Throttle FPS](#throttle-fps)
- [TPS](#tps)
- [Uncap cinematic](#uncap-cinematic)
- [Unblock all extra weapons](#unblock-all-extra-weapons)
- [Widescreen fix](#widescreen-fix)

#### Aim assist
Halo's aim assist was broken in the PC release. Chimera fixes it. It only works
for analog sticks, though.

**Usage:** `chimera_aim_assist [true/false]`

#### Allow all passengers
Disable teammate restrictions for vehicles, allowing anyone to enter anyone's
vehicle. This will only apply if you're hosting a game, and clients will need to
have a version of Chimera that fixes desyncing with multi-team vehicles (or an
equivalent mod), or else desyncing will occur.

**Usage:** `chimera_allow_all_passengers [true/false]`

#### Anisotropic filtering
This enables the same thing that is done in config.txt but without having to
edit it. Note that this only applies to the level geometry, not individual
objects. You'd need AF enabled externally to do that.

**Usage:** `chimera_af [true/false]`

#### Auto uncrouching
When on the ground, the game disabled crouching if the player was running at
full speed. You can re-enable this behavior if you want.

**Usage:** `chimera_auto_uncrouch [true/false]`

#### Block all bullshit
This feature runs the following commands:
- `chimera_block_buffering 1`
- `chimera_block_gametype_indicator 1`
- `chimera_block_gametype_rules 1`
- `chimera_block_hold_f1 1`
- `chimera_block_loading_screen 1`
- `chimera_block_zoom_blur 1`
- `chimera_block_mouse_acceleration 1`

**Usage:** `chimera_block_all_bullshit`

#### Block auto center
Set whether or not auto centering in vehicles is disabled.

**Usage:** `chimera_block_auto_center [true/false]`

#### Block buffering
This feature is known to reduce input lag, and it works like the feature in
config.txt. Disabling buffering is known to destroy performance on DXVK, but
generally works better everywhere else. You can enable this behavior if you want.

**Usage:** `chimera_block_buffering [true/false]`

#### Block button quotes
Remove the quotation marks in button prompts (e.g. Hold "E")

**Usage:** `chimera_block_button_quotes [true/false]`

#### Block equipment rotation
Netgame equipment rotation was added in Halo PC. You can disable it if you want
spawns to look more like their Xbox counterparts, though most maps will probably
look worse.

**Usage:** `chimera_block_equipment_rotation [true/false]`

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
loading screen.

**Usage:** `chimera_block_loading_screen [true/false]`

#### Block mouse acceleration
Halo uses raw input, thus it bypasses the mouse acceleration setting of your
operating system (unless you run the game in Wine). However, Halo adds its own
mouse acceleration on top of direct input.

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

**Usage:** `chimera_deadzones [0-1]`

#### Delete empty weapons
Automatically delete empty weapons when hosting, similar to Xbox behavior.

**Usage:** `chimera_delete_empty_weapons [true/false]`

#### Devmode
Enable devmode commands without blocking you from multiplayer or requiring any
command line arguments.

**Usage:** `chimera_devmode [true/false]`

#### Diagonals
Set the diagonals for moving with an analog stick. This is because Halo converts
it to digital input, as the netcode does not support analog movement. A good
value is 0.75.

**Usage:** `chimera_diagonals [0-1]`

#### FoV
Set FoV. End with "v" to lock to a vertical FoV, or use "auto" to set to an
automatic FoV. Using a vertical FoV over a horizontal one helps prevent the
game from distorting your field of view when switching aspect ratios.

Setting `chimera_fov_cinematic` and `chimera_fov_vehicle` will apply their FoV
when those apply, falling back to `chimera_fov` if they aren't set or not doing
anything at all if `chimera_fov` is not set.

**Usage:** `chimera_fov [degrees]`
**Usage:** `chimera_fov_cinematic [degrees]`
**Usage:** `chimera_fov_vehicle [degrees]`

#### FP reverb
Set whether or not first person sounds should have reverberations when EAX is
enabled.

**Usage:** `chimera_fp_reverb [true/false]`

#### Language
Chimera is localized for both English and, thanks to help from the community,
Spanish.

**Usage:** `chimera_language [en/es]`

#### Model detail
Change the model detail. Higher values increase LODs. This will not make models
extra detailed. Instead, it will change the threshold for when higher LODs
appear. Extremely high values will cause the aforementioned "4K headless chief"
glitch, except at any vertical resolution this time.

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

#### Send chat message
Send a chat message. Channel 0 is "all", channel 1 is "team", and channel 2 is
"vehicle". Other channels may be used by mods.

**Usage:** `chimera_send_chat_message <channel> <message>`

#### Set color
Set your in-game color for free-for-all gametypes.

**Usage:** `chimera_set_color [color]`

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

#### Spam to join
Set whether or not to automatically reattempt connecting if the server is full.

**Usage:** `chimera_spam_to_join [true/false]`

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
Throttle Halo's frame rate. Set to 0 to disable.

**Usage:** `chimera_throttle_fps [FPS]`

#### TPS
Set the game's tick rate. This will not work outside of client hosted games.

**Usage:** `chimera_tps [TPS]`

#### Unblock all extra weapons
Unblock all weapons blocked with [Block extra weapon](#block-extra-weapon)

**Usage:** `chimera_unblock_all_extra_weapons`

#### Uncap cinematic
Uncap the cinematic frame rate from 30 FPS.

**Usage:** `chimera_uncap_cinematic [true/false]`

#### Widescreen fix
Fix the scaling of HUD, menu, and text elements for wider aspect ratios. This
may break some maps, and this will not be fixed as fixing one thing will break
another thing due to the nature of these fixes. Setting 1 has the HUD fill the
screen, while setting 2 restricts the HUD to the center 4:3 area of the screen.

**Usage:** `chimera_widescreen_fix [0/1/2]`





## FAQ
Below is a list of frequently asked questions. These questions are either based
on questions I have received or questions I *might* receive that I feel are
worth answering in a readme.

- [Will Chimera run on my system?]
- [Can I use Chimera under a license besides GNU GPL version 3?]
- [Why are my custom fonts not working?]
- [Why does Halo's gamma setting not work when Chimera is installed?]
- [Why is there no auto updater built into Chimera?]
- [Why do I get an error when joining Custom Edition servers with modded maps?]



### Will Chimera run on my system?
Short answer: If your PC is semi-recent and uses Windows 10, it'll work. Note
that, while Chimera isn't too taxing, especially on modern systems, it can make
Halo harder to run if you're on extremely ancient hardware. We do not test
Chimera on Windows XP, Windows Vista, Windows 7, or Windows 8(.1), but it will
*probably* run on them, too.

Long answer: There are the requirements:

- OS (if on Windows): Windows 10 or newer (Windows XP may work, but it is untested)
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

The newest of the above recommendations is the AMD CPU (FX 6100) which was
released in the year 2011 - over seven years ago at the time of writing this
sentence.

### Can I use Chimera under a license besides GNU GPL version 3?
Probably not. You would first need to ask *all* of the developers of Chimera for
permission.

### Why are my custom fonts not working?
This can occur when a required DirectX 9 library isn't installed. The fix is to
download and run the official DirectX installer from
https://www.microsoft.com/en-us/download/details.aspx?id=35 .

### Why does Halo's gamma setting not work when Chimera is installed?
Short answer: Halo's gamma setting was disabled as it does a lot more harm than
good. Basically, it's screwed up and there's no good way to fix it. This may
seem like an inconvenience, but you really do *not* want to use this. We
recommend using [dgVoodoo2] for this, instead.

Long answer: How Halo's gamma setting works is, rather than changing Halo's own
brightness level, it changes your operating system's brightness. It then stores
the gamma setting by editing the registry. Halo is then programmed to restore
the operating system's older gamma setting when it closes. However, this can
only occur if Halo closes correctly. There is the chance that it won't close
properly (exception errors, you closed it improperly, etc.), and if that
happens, you may end up with Halo's gamma setting lingering.

We decided to disable this setting since it presented unnecessary modifications
to the registry, it potentially ends up actually screwing up your operating
system's video settings, and there is quite simply a better way to do it,
anyway.

We are *not* going to provide an option to re-enable it. Again, use [dgVoodoo2],
instead.

[dgVoodoo2]: http://dege.freeweb.hu/dgVoodoo2/

### Why is there no auto updater built into Chimera?
Short answer: Auto updating is *not* actually a feature you want. For the best
experience, the responsibility for keeping Chimera up-to-date falls upon you,
the user.

Long answer: The gains of adding an auto updater are vastly outweighed by the
cons of having one.

The only reason to add an auto updater is for convenience. Chimera is very easy
to install and update. So, it is not necessary to add one even for novice users.
Updates are generally announced on both the Discord server as well as on Open
Carnage at https://opencarnage.net/index.php?/topic/6916-chimera-10-beta/.

In fact, there are many reasons *against* having an auto updater built into the
mod.

First, you need write access to the mod DLL. Chimera is written to be a single
strings.dll. When the game is running, this file cannot be written to due to it
being in use. This can be worked around by having Chimera, itself, be a separate
DLL, but this would require rewriting how Chimera is loaded. We decided to use
one DLL because a number of users ran into issues with it due to user error.

Also, most people do not have write access to the Halo: Combat Evolved game
files as they installed it into the default "Program Files" folder. To bypass
this, you would have to manually give yourself permission (too complicated for
some users to figure out), or you run the .exe as an administrator (very foolish
to do on an Internet-connected game from 2003 that more-or-less assumes that
cache files are "safe").

Having an auto updater or even an update notifier would also require contacting
a server whenever the game starts. Doing that increases startup time, and it is
error prone.

For example, an auto updater presents a security risk. You're assuming that the
server used to host the update is never going to be hijacked. Signing the update
*can* be used to mitigate this, preventing unauthorized builds from being
installed due to validation failing on the client side, but that's only
effective if the signing mechanism is not compromised. Also, there's the chance
that a developer could decide to go rogue and deliberately push a bad update out
of spite. We would NEVER do that, but should you still take the chance?

Also, existing auto updater implementations in Halo mods have all proven to
cause issues, some of which are actively killing the game.

For example, SAPP, a popular server mod, has auto updating. However, some
versions would crash Halo with an exception error when run on Wine. The reason
for the crash is due to how the mod developer obfuscates the DLL. Wine has
issues loading these DLLs, requiring them to be unpacked. However, because the
DLL has been tampered with, doing this is not possible without reverting the DLL
manually.

As another example, HAC2, a popular client mod, also has auto updating.
Periodically, the HAC2 server may go down, and for whatever reason, HAC2 was
written to not allow the user to start the game with HAC2 if auto updating
failed, resulting in the player base permanently being decimated. This behavior
is not unlike "always online" DRM although the intentions are not malicious - it
is merely an oversight (albeit an expensive one). In fact, HAC2's loader.dll
does not even come with HAC2 and requires an initial connection to the Internet
to work.

Newer versions of the HAC2 loader do not exhibit this issue, but many people use
the older version of the loader as it's what's most distributed. Either way,
the damage is done.

Also, at some point, the HAC2 project changed developers, and updates became
closed source (the [public HAC2 repository](https://github.com/Chaosvex/HAC2) is
not maintained and is years out of date, and it isn't even complete enough to
build). This meant that people updated from a mostly free and open source build
to a nonfree and closed source build.

No mod developer should be able to do any of the following:
- Update to a build that no longer works when it was working before
- Restrict you from playing the game unless it can communicate with a server
- **Change the terms of your agreement to said software without your consent**

Also, no user should have the option to consent to any of the above, because
absolutely nothing good comes from having it enabled.

Since it's easy to keep your version of Chimera up-to-date, anyway, it was
decided against implementing auto updating functionality in Chimera.

### Why do I get an error when joining Custom Edition servers with modded maps?
Short answer: This is probably due to a CRC32 mismatch. Basically, this is to
prevent Halo from crashing due to using a map that differs from what is hosted
on the server. This is done for your protection, and we recommend either forging
the CRC32 or getting the original map.

Long answer: Halo Custom Edition (and Chimera if joining Custom Edition servers)
calculates a checksum (a number) from your map based on the contents of the map
file. It then sends this number to the server upon joining, and the server
compares it against the map it has. If the checksum isn't forged, then it can
confirm, with ~99.999999976% certainty (or a 1 in 4,294,967,296 chance for a
collision), that the map is identical.

The reason it does all of this is due to the fact that using a modified map or
a different map entirely can result in a game crash for the client. Obviously,
there are some modifications that can be done to a map that will not result in a
crash, but there is no way to discern such a chance using a checksum, alone.
This is done for your protection, and it is not recommended to attempt to bypass
this check.

If you *want* to join a server using a modified Halo Custom Edition map on a
Halo Custom Edition server, then you should forge the CRC32.



[Will Chimera run on my system?]: #will-chimera-run-on-my-system
[Can I use Chimera under a license besides GNU GPL version 3?]: #can-i-use-chimera-under-a-license-besides-gnu-gpl-version-3
[Why are my custom fonts not working?]: #why-are-my-custom-fonts-not-working
[Why does Halo's gamma setting not work when Chimera is installed?]: #why-does-halos-gamma-setting-not-work-when-chimera-is-installed
[Why is there no auto updater built into Chimera?]: #why-is-there-no-auto-updater-built-into-chimera
[Why do I get an error when joining Custom Edition servers with modded maps?]: #why-do-i-get-an-error-when-joining-custom-edition-servers-with-modded-maps
