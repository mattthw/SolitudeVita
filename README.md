# Solitude

## Introduction
- vitaQuake is a Quake engine source port for PSVITA.
- SolitudeVita is a port of Solitude for the vita. Solitude was originally made for the PSP, which was abandoned, then rebooted as Solitude Revamped, then abandoned, then ported to N3DS, then forgotten. This port is still a work in progress.
## Features
- Slayer game type
- Swat game type
- Firegiht mode
- 11 Slayer maps, 2 Firefight maps
 - One new map (Citadel, a remake of the map from the third official game DLC- remade by me) - this one is probably the best looking while maintaining close to 60fps on the vita
 - One remade map (Narrows, credit to the original author: Scifiknux)
- Overhauled menus and graphics
- New touchscreen aiming support

## Broken / WIP
- See [issues](https://github.com/mmccoy37/vitaQuake-Solitude/issues)

# Installation
- Place the ```/Solitude``` folder inside ```<ux0 or alternative:>/data/Quake/```. You will also need the original quake files or shareware files as a .PAK filetype inside ```<ux0 or alternative:>/data/Quake/id1/```. You should be able to use the data files from VITADB for vitaQuake.
- End result will look like
```
/data/Quake/
            ./id1/*.PAK  # Legally acquired PAK files from original quake game
            ./Solitude/*
```

# Development

## Compile SolitudeVita
- Install the VitaSDK and set up building with the instructions here; https://vitasdk.org/
- Install VitaGL (https://github.com/Rinnegatamante/vitaGl) with "make" then "make install"
- Then just run ``make`` in the SolitudeVita, which should produce a working vitaquake.vpk
## Add vitaQuake as remote (for source code updates, etc)
- ``git remote add vitaQuake https://github.com/Rinnegatamante/vitaQuake.git``

# Credits
- idSoftware for winQuake sourcecode
- MasterFeizz for ctrQuake sourcecode i studied to understand how winQuake works
- EasyRPG Team for the audio decoder used for CDAudio support
- Ch0wW for various improvements and code cleanup
- JPG for ProQuake and some various fixes.
- Cuevavirus for 1920x1080 rendering

