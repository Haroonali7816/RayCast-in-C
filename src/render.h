#ifndef RENDER_H
#define RENDER_H

#include "map.h"
#include "texture.h"
void render_map(Map *map, const char *output_filename, int width, int height, double fov, double rotation, double pos_x,
                double pos_y);

//Textured Rendering with Shading and Ceiling/Floor.
void render_texture_map(Map *map, const char *output_file, int width, int height, double fov, double rotation,
                        double pos_x, double pos_y, TextureAtlas *atlas, int shade, int ceiling);

#endif  // RENDER_H