# le3d  
**LightEngine 3D**  
**Version 1.75 - 13/08/2018**

A straightforward C++ 3D software engine for real-time graphics.  
The engine aims to be a minimal and clear implementation of a simplified fixed pipeline.  
Code has been designed for resource constrained platforms.  

**Build status - Linux, MacOS, Amiga (GCC / CLang)**  
[![Build Status](https://travis-ci.org/Marzac/le3d.svg?branch=master)](https://travis-ci.org/Marzac/le3d)  

**Build status - Windows (MSVC)**  
[![Build status](https://ci.appveyor.com/api/projects/status/wgilys64hn3wqqdu?svg=true)](https://ci.appveyor.com/project/Marzac/le3d)  

---

<p align="center">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render1.jpg" width="256">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render2.jpg" width="256">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render3.jpg" width="256">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render4.jpg" width="256">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render5.jpg" width="256">
<img src="https://github.com/Marzac/le3d/blob/master/docs/pictures/render6.jpg" width="256">
</p>

---

A project maintained by Fred's Lab  
http://fredslab.net  

This is open source software released under the MIT License, please refer to LICENCE file for more information.  

Currently supported platforms:
- Microsoft Windows XP, 7, 8 and 10
- Linux based operating systems
- MacOS X
- AmigaOS 3.x (m68k)

# Core engine
**Source code / examples / documentation**  
Frédéric Meslin  
**Twitter:** @marzacdev  
**Email:** fred@fredslab.net  

**CMake scripts / Amiga + MacOS support**  
Andreas Streichardt  
**Twitter:** @m0ppers  
**Email:** andreas@mop.koeln  

# Exceptions
__Mesh textures__  
In the example folder, one or more textures on the 3D models have been created with photographs from Textures.com.  
These photographs may not be redistributed by default;  
Please visit:  
http://www.textures.com  
for more information.  

__Skybox texture__  
In the example folder, the meadow skybox has been created by Emil "Humus" Persson and it is licenced under the Creative Commons  
Attribution 3.0 Unported License.  
Please visit:  
http://www.humus.name/index.php?page=Textures  
for more information.  

---

# Renderer (Portable C++ / ASM (SSE2))  
The renderer:  
- Renders meshes  
- Renders billboard sets  
- Does simple transformations (rotation, scaling, translation)  
- Performs 3D clipping  
- Performs 2D clipping  
- Performs projection  
- Does backculling  
- Performs Z-sorting  


The rasterizer:  
- Draws textured triangles  
- Handles mipmapping  
- Handles alpha blending  
- Applies solid color per triangle  
- Applies quadratic fog per fragment  
- Performs perspective correction  
- Does not perform texture filtering  
- Does not draw anti-aliased edges  
- Align vertex coordinates to nearest pixel coordinates  


The lighting system:
- Handles point lights
- Handles directional lights
- Handles ambient lights


# Backends (Native OS code)  
window / draw / gamepad / timing / system  

The backend - Windows version:  
- Handles windows creation / management
- Handles mouse events
- Handles keyboard events
- Handles graphic contexts with GDI
- Handles joysticks with rumble support
  (with Microsoft XInput V1.3 drivers)
- Supports Windows XP / 7 / 8 and 10 OS
- Supports fullscreen mode

The backend - Linux version:  
- Handles windows creation / management (with X11)
- Handles mouse events (with X11)
- Handles keyboard events (with X11)
- Handles graphic contexts (with X11)
- Handles joysticks with rumble support
  (with evdev interface)
- Supports virtually all Linux based OS

The backend - MacOS version:
- Similar to Linux version
- Needs X11 (XQuartz) for MacOS
- No native COCOA support
- No joystick support yet (soon)

The backend - Amiga version:
- Similar to Linux version
- Needs cybergraphics installed
- No joystick support yet
- AmigaOS 3.x
- Too slow on old m68k hardware (use vampire)
- PowerPC support unclear/untested
- AmigaOS 4.x support unclear/untested


# File formats  
Supported bitmap / texture formats:
- Uncompressed 24bit RGB windows bitmap
- Uncompressed 32bit RGBA windows bitmap
- Import & export functions

Supported 3D model formats:
- Wavefront OBJ
- Import & export functions

# Limitations  
- No Z-buffer  
- No antialiasing
- Single core support

# Comparison  
It renders graphics a bit better than a Sony Playstation one.  

# Building

Please refer to BUILDING.md
