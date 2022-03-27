# Solitude

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

# Development

## Compile SolitudeVita
- Install the VitaSDK and set up building with the instructions here; https://vitasdk.org/
- Install VitaGL (https://github.com/Rinnegatamante/vitaGl) with "make" then "make install"
- Then just run ``make`` in the SolitudeVita, which should produce a working vitaquake.vpk

## Add vitaQuake as remote (for source code updates, etc)
- ``git remote add vitaQuake https://github.com/Rinnegatamante/vitaQuake.git``