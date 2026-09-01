![C](https://img.shields.io/badge/C-99-blue)
![Graphics](https://img.shields.io/badge/Rendering-Raycasting-orange)
![Output](https://img.shields.io/badge/Output-PPM%20(P6)-purple)
 
**RayCast-in-C**
 
A raycasting engine written in plain C — the rendering technique *Wolfenstein 3D* used in 1992 to fake 3D graphics on hardware that couldn't render real 3D. For every column of pixels on screen, a ray is cast from the player's position through a 2D grid map; wherever it hits a wall, a vertical textured strip is drawn whose height and shading depend on distance. Repeat that a few hundred times per frame and a flat text-file maze becomes a first-person view. Built by **Muhammad Haroon Ali**.
 
Originally developed for the Compiler Design Lab (Programming 2, Saarland University). Unlike the course's other projects, no project skeleton was provided — the module layout, build system integration, and memory management are all original.
 
### Motive
 
Raycasting sits at an interesting intersection: it's graphics programming with none of the usual graphics-library scaffolding. There's no OpenGL, no framebuffer API, no vector math library — just a pinhole camera model derived from first principles, a DDA (digital differential analysis) grid-traversal algorithm to find wall intersections fast, and manual `.ppm` byte-writing to get pixels on disk. The constraint of writing it in C with manual memory management (parse a map, cast a thousand rays, tear it all down cleanly, leak nothing) makes the geometry and the systems-programming discipline equally load-bearing.
 
### Features
 
- Text-based maze parser (`-I`) reporting map dimensions, start position, and wall-cell count
- Top-down minimap export (`-M`) to a `.ppm` (P6) image — black walls, white floor, green start tile
- Full first-person rendering (`-R`) from an arbitrary position, rotation, and field of view, using a pinhole camera model with per-column DDA raycasting against the grid
- Textured walls (`-T`) sampled from a texture atlas, with optional directional shading to fake depth and distinguish x-facing from y-facing walls
- Textured floor and ceiling rendering, mapped per-pixel from the ray/floor-plane intersection
- Continuous streaming mode (`-C`) that reads player position/rotation from stdin and writes raw PPM frames to stdout, driven live by the included `extra/viewer.py` so you can actually walk around the maze
- Manual memory management throughout — every `malloc` paired with a matching free, constructors/destructors for every struct
### How It Works
 
**Pinhole camera model.** The camera sits at `(p_x, p_y, 0.5)`, halfway between floor and ceiling. Given a field of view and rotation, a virtual image plane is placed a focal distance `f = w / (2 tan(FOV/2))` in front of the camera; every output pixel maps to a point on that plane, and the ray direction is that point minus the camera position.
 
**DDA raycasting.** For each screen column, the ray is walked grid cell by grid cell — jumping from one grid line to the next in whichever axis is closer — until it hits a wall. This finds the exact intersection point without stepping pixel-by-pixel through the map, and cheaply tracks whether an x-side or y-side wall was hit (which determines wall color/texture orientation and shading).
 
**Texturing.** Each wall tile can select a texture by index (digits `0`–`9` in the map file map to atlas columns). The intersection point's fractional coordinate along the hit wall is used to look up a column in the corresponding texture, giving seamless per-pixel texture mapping across the wall strip.
 
### Tech Stack
 
| Layer | Technology |
|---|---|
| Language | C (C99), no external graphics libraries |
| Image I/O | Hand-rolled PPM (P6) reader/writer |
| Build | Makefile (auto-detects `.c`/`.h` files added to `src/`) |
| Dev environment | Dev Container, `.clang-format` |
| Live viewer | Python (`extra/viewer.py`), drives continuous mode over stdin/stdout |
| Testing | Public test suite under `test/` |
 
### Project Structure
 
```
RayCast-in-C/
├── .devcontainer/       # Reproducible dev environment
├── .vscode/             # Editor/debugger configuration
├── data/                # Sample maps and texture atlases (e.g. maze.txt, wolftextures.ppm)
├── extra/               # viewer.py — live keyboard-driven viewer for continuous mode
├── src/                 # All source files (flat directory, auto-picked up by the Makefile)
├── test/                # Public test suite (same tests used for grading)
├── .clang-format
├── Makefile
└── README.md
```
 
### Prerequisites
 
- A C compiler (GCC/Clang) and `make`
- Python 3 (only for the optional `extra/viewer.py` live viewer)
### How to Run
 
```bash
git clone https://github.com/Haroonali7816/RayCast-in-C.git
cd RayCast-in-C
make                       # builds bin/raycast
 
# Map diagnostics
./bin/raycast data/maze.txt -I
 
# Top-down minimap
./bin/raycast data/maze.txt -M output/maze.ppm
 
# First-person render: <output> <width> <height> <fov> <rotation> <pos_x> <pos_y>
./bin/raycast data/maze.txt -R output/render.ppm 800 600 60 0 0.5 1.5
 
# Textured render: <texture_file> <tex_cols> <tex_rows> <output> <width> <height> <fov> <rotation> <pos_x> <pos_y> [shade] [ceiling]
./bin/raycast data/maze.txt -T data/wolftextures.ppm 8 1 output/render.ppm 800 600 60 0 0.5 1.5 1 1
 
# Continuous mode, driven live via the Python viewer
python extra/viewer.py data/maze.txt
```
 
Output is raw `.ppm` (P6) — open it in any viewer that supports the format, or convert it (e.g. with ImageMagick) to `.png`.
 
### Map Format
 
```
11 11
###########
S          #
## 33 ### #
...
```
 
| Symbol | Meaning |
|---|---|
| `S` | Player start position |
| ` ` (space) | Walkable floor |
| digit (`0`–`9`) | Wall using texture atlas index *n* |
| any other non-whitespace character | Wall (default texture) |
 
### Author
 
**Muhammad Haroon Ali**
 
