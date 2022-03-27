<<<<<<< HEAD
# Introduction
vitaQuake is a Quake engine source port for PSVITA.

An official channel to discuss the development of this source port can be found on [Vita Nuova discord server](https://discord.gg/PyCaBx9).

# Features
- Hardware accelerated GPU rendering
- Native 960x544 resolution
- Rendering resolution up to 1920x1080 on the PSTV with [Sharpscale](https://git.shotatoshounenwachigau.moe/vita/sharpscale)
- MSAA 2x and MSAA 4x support
- Dual analogs support
- Native IME for inputing commands/text
- Sounds and Musics (CDTracks) support in OGG, MP3, WAV formats
- Gyroscope and touchscreen support for camera movement
- Custom arguments support and mods support
- Support for both official missionpacks
- Support for transparent surfaces (.alpha and .renderamt)
- Increased Quake Engine limits (max vertices, max entities, max static entities, etc...)
- LAN Multiplayer support (locale and online)
- AdHoc Multiplayer support
- ProQuake net protocol support
- Savegames fully working
- Support for colored lights with .lit files support
- Support for Half-Life BSP
- Supprt for BSP2 and 2BSP formats
- Smooth animations thanks to interpolation techniques
- Crosshair and custom crosshairs support
- Mirrors support
- Specular mode support
- Fog support
- Cel Shading support
- Bilinear filtering support
- Dynamic shadows support
- Several different improvements in the renderer quality
- Several different miscellaneous features (eg: transparent statusbar, benchmark feature, working gamma, etc...)
- Map downloader support if you try to join an online server and you don't own the running map

# Supported DarkPlaces extensions
- DP_CON_SET
- DP_CON_SETA
- DP_EF_BLUE
- DP_EF_NODRAW
- DP_EF_RED
- DP_ENT_ALPHA
- DP_GFX_EXTERNALTEXTURES
- DP_GFX_EXTERNALTEXTURES_PERMAPTEXTURES
- DP_HALFLIFE_MAP
- DP_LITSUPPORT
- DP_QC_ASINACOSATANATAN2TAN
- DP_QC_COPYENTITY
- DP_QC_CVAR_STRING
- DP_QC_EDICT_NUM
- DP_QC_ETOS
- DP_QC_FINDCHAIN
- DP_QC_FINDCHAINFLOAT
- DP_QC_MINMAXBOUND
- DP_QC_NUM_FOR_EDICT
- DP_QC_RANDOMVEC
- DP_QC_SINCOSSQRTPOW
- DP_QC_TRACEBOX
- DP_SND_FAKETRACKS
- DP_SV_MODELFLAGS_AS_EFFECTS
- DP_SV_NODRAWTOCLIENT
- DP_SV_DRAWONLYTOCLIENT
- EXT_BITSHIFT
- FRIK_FILE

# CDAudio Support

vitaQuake supports all soundtrack packs for Quake and its two official mission packs, "Scourge of Armagon" and "Dissolution of Eternity." In order for the soundtrack to work, files must be placed in a folder named /cdtracks/ in each campaign folder (main game for example will be ux0:data/Quake/id1/cdtracks). 
=======
# Solitude
>>>>>>> 04d051a (updated readme)

## Introduction
vitaQuake is a Quake engine source port for PSVITA.

vitaQuake Solitude is a port of Solitude for the vita. Solitude was originally made for the PSP, which was abandoned, then rebooted as Solitude Revamped, then abandoned, then ported to N3DS, then forgotten. This port is still a work in progress.

## Credit
- Credit to Id Software for Quake
- Credit to Rinne for [vitaQuake](https://github.com/Rinnegatamante/vitaQuake)
- Credit to FlamingIce team for the original Solitude and Solitude Revamped
- Ghost_Fang for menu & scoreboard inspiration
- Credit to TCPixel for the [N3DS port](https://github.com/CollinScripter/Revamped3DS), which was the only place I was able to obtain the source code
- Also credit to all the unnamed and unknown contributors to Solitudes several maps and many assets

An official channel to discuss the development of this source port can be found on [Vita Nuova discord server](https://discord.gg/PyCaBx9).

<<<<<<< HEAD
# Credits
- idSoftware for winQuake sourcecode
- MasterFeizz for ctrQuake sourcecode i studied to understand how winQuake works
- EasyRPG Team for the audio decoder used for CDAudio support
- Ch0wW for various improvements and code cleanup
- JPG for ProQuake and some various fixes.
- Cuevavirus for 1920x1080 rendering
=======
## Features
- Slayer game type
- Swat game type
- Firegiht mode
- Many maps
 - One new map (Citadel, a remake of the map from the third official game DLC- remade by me) - this one is probably the best looking while maintaining close to 60fps on the vita
 - One remade map (Narrows, credit to the original author: Scifiknux)
- Overhauled menus and graphics
- New touchscreen aiming support
- A ton of small stuff ranging from bug fixes to making the game feel more like Solitude

## Broken / WIP
- See [issues](https://github.com/mmccoy37/vitaQuake-Solitude/issues)

# Installation

<<<<<<< HEAD
Place the Solitude folder inside ```<ux0 or alternative:>/data/Quake/```. You will also need the original quake files or shareware files as a .PAK filetype inside ```<ux0 or alternative:>/data/Quake/id1/```. You should be able to use the data files from VITADB for vitaQuake.
>>>>>>> 04d051a (updated readme)
=======
Place the ```/Solitude``` folder inside ```<ux0 or alternative:>/data/Quake/```. You will also need the original quake files or shareware files as a .PAK filetype inside ```<ux0 or alternative:>/data/Quake/id1/```. You should be able to use the data files from VITADB for vitaQuake.
>>>>>>> 27badf1 (Update README.md)
