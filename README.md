# Ldoomc

Ldoomc is a first person shooter prototype project. The name is derived from the intial
prototypes, which were coded in Lua with a plan to move to C, and envisioned to be like id's Doom.

## Features
* Cross Platform (Linux, Mac, (Windows soon))
* 3D rendering
* Physics

## Building
```bash
git clone git@bitbucket.org:bakpakin/ldoomc.git
cd Ldoomc
mkdir build && cd build
cmake ..
make
# Run the project on OSX
open Ldoom.app
# Linux
./Ldoom
```
Ldoomc is built with Cmake. Should build straight out of the box on OSX, and with
some dependencies on linux. To install dependencies on ubuntu and debian derivatives, run:
```bash
sudo apt-get install libgl1-mesa-dev libxrandr-dev libxcursor-dev libopenal-dev
```

Requires OpenGL 3.3, so make sure you have good drivers.
