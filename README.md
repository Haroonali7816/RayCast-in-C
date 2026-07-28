# 🎮 raycaster

A raycasting engine written in C — the technique *Wolfenstein 3D* used back in 1992 to fake 3D graphics on hardware that couldn't actually do 3D. Instead of rendering polygons, a ray is shot straight out from the player for every column of pixels on screen. Wherever it hits a wall, a vertical strip gets drawn whose height depends on how far away that wall was. Do that a few hundred times and a flat 2D grid turns into a full first-person view.

Written for a systems programming course — plain C, manual memory management, no external graphics libraries. Output goes straight to `.ppm` image files.

---

## ✨ Features

- 🗺️ Parse a maze from a simple text map format
- 🖼️ Export a top-down minimap as an image
- 📷 Render a full first-person view from any position, angle, and FOV using a pinhole camera model
- 🧱 Apply wall textures from a texture atlas, with basic shading to fake depth
- 🌌 Render a textured floor and ceiling
- 🎥 Stream frames continuously over stdin/stdout to walk around the maze live

---

## 🛠️ Building

```bash
make
```

Builds `bin/raycast`. No dependencies beyond a C compiler and `make`.

---

## 🚀 Usage

Every command follows the same pattern — map file first, then a flag for what to do.

**Map info**
```bash
./bin/raycast data/maze.txt -I
```

**Top-down minimap**
```bash
./bin/raycast data/maze.txt -M output/maze.ppm
```

**First-person render**
```bash
# output  width  height  fov  rotation  pos_x  pos_y
./bin/raycast data/maze.txt -R output/render.ppm 800 600 60 0 0.5 1.5
```

**Textured render**
```bash
# atlas  atlas_cols  atlas_rows  output  width  height  fov  rotation  pos_x  pos_y  [shade]  [ceiling]
./bin/raycast data/maze.txt -T data/wolftextures.ppm 8 1 output/render.ppm 800 600 60 0 0.5 1.5 1 1
```

**Continuous mode** (streams frames over stdin/stdout)
```bash
./bin/raycast data/maze.txt -C data/wolftextures.ppm 8 1 800 600 60
```

Output is plain `.ppm` (P6) — any image viewer that supports it will open it directly, or convert it with ImageMagick for a `.png`.

A small Python viewer drives continuous mode so you can actually walk around instead of rendering static frames:

```bash
python extra/viewer.py data/maze.txt
```

---

## 🗺️ Map format
11 11
###########
S #
33 ###

| Symbol | Meaning |
|--------|---------|
| `S`    | Start tile |
| ` `    | Walkable space |
| digit  | Wall using texture index *n* |
| other  | Wall, default texture |
