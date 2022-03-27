# Solitude

> ⚠️ **WARNING:** I am looking for the QC quake code source files to [Halo Revamped](https://www.youtube.com/watch?v=B_GB9LLBATQ) (r17?) for N3DS! Without this, the changes I am able to make are extremely limited. If you happen to have this laying around on your computer or know someone who has this, please let me know.
## Introduction
vitaQuake is a Quake engine source port for Vita by Rinnegatamante

SolitudeVita is a port of Solitude for the vita, built ontop of vitaQuake. Solitude was originally made for the PSP, which was abandoned, then rebooted as Solitude Revamped, then abandoned, then ported to N3DS, then forgotten. This port is still a work in progress.

### Features
- Slayer game type
- Swat game type
- Firegiht mode
- 11 Slayer maps, 2 Firefight maps
 - One new map (Citadel, a remake of the map from the third official game DLC- remade by me) - this one is probably the best looking while maintaining close to 60fps on the vita
 - One remade map (Narrows, credit to the original author: Scifiknux)
- Overhauled menus and graphics
- New touchscreen aiming support

### Screenshots
![](https://github.com/mmccoy37/SolitudeVita/blob/master/files/menu.png)
![](https://github.com/mmccoy37/SolitudeVita/blob/master/files/gameplay.png)
### Gameplay
[Watch on youtube](https://www.youtube.com/watch?v=949wQT5fhPk)

# Issues
![](https://img.shields.io/github/issues-raw/mmccoy37/SolitudeVita) ![](https://img.shields.io/github/issues-closed-raw/mmccoy37/SolitudeVita)

Search [issues](https://github.com/mmccoy37/vitaQuake-Solitude/issues) for existing bugs and feature requests before submitting a new one.

# Installation on Playstation Vita
1. Download the latest release: [releases](https://github.com/mmccoy37/SolitudeVita/releases/)
1. Extract the archive
1. Install ``Solitude.vpk`` on the Playstation Vita
1. Copy the ```/Solitude``` folder to ```<ux0 or alternative:>/data/Quake/```. 
    - You will also need the original quake files or shareware files as a .PAK filetype inside ```<ux0 or alternative:>/data/Quake/id1/```. You should be able to use the data files from VITADB for vitaQuake.


The end result will look something like this:
```
/data/Quake/
            ./id1/*.PAK  # Legally acquired PAK files from original quake game
            ./Solitude/*
```

# Development

### Compile SolitudeVita
- Install the VitaSDK and set up building with the instructions here; https://vitasdk.org/
- Install VitaGL (https://github.com/Rinnegatamante/vitaGl) with "make" then "make install"
- Then just run ``make`` in the SolitudeVita, which should produce a working vitaquake.vpk

### Add vitaQuake as remote (for source code updates, etc)
- ``git remote add vitaQuake https://github.com/Rinnegatamante/vitaQuake.git``

# Credits
- idSoftware for winQuake sourcecode
- MasterFeizz for ctrQuake sourcecode i studied to understand how winQuake works
- EasyRPG Team for the audio decoder used for CDAudio support
- Ch0wW for various improvements and code cleanup
- JPG for ProQuake and some various fixes.
- Cuevavirus for 1920x1080 rendering

## SolitudeVita Additional Credit
- Credit to Id Software for Quake
- Credit to Rinne for [vitaQuake](https://github.com/Rinnegatamante/vitaQuake)
- Credit to FlamingIce team for the original Solitude and Solitude Revamped
- Ghost_Fang for menu & scoreboard inspiration
- Credit to TCPixel for the [N3DS port](https://github.com/CollinScripter/Revamped3DS), which was the only place I was able to obtain the source code
- Also credit to all the unnamed and unknown contributors to Solitudes several maps and many assets