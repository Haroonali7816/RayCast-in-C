#ifndef RENDER_H
#define RENDER_H

#include "map.h"

void render_map(Map *map,
                const char *output_filename,
                int width,
                int height,
                double fov,
                double rotation,
                double pos_x,
                double pos_y);

#endif // RENDER_H