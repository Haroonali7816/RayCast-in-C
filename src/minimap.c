#include "minimap.h"

#include <stdio.h>

void create_minimap(Map *map, const char *output_filename) {
    FILE *file = fopen(output_filename, "wb");
    if (!file) {
        fprintf(stderr, "Error : could not create output file %s\n", output_filename);
        return;
    }
    // PPM HEADER
    // P3
    // width then height and then 255.
    fprintf(file, "P6\n%d %d\n255\n", map->width, map->height);

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            char cell = map->grid[y][x];
            unsigned char r, g, b;

            // we check if cell is a wall.
            if (cell == ' ') {
                r = 255;
                g = 255;
                b = 255;
            } else if (cell == 'S') {
                r = 0;
                g = 255;
                b = 0;
            }  // it is black.

            else {
                // white pixel.
                r = 0;
                g = 0;
                b = 0;
            }
            fputc(r, file);
            fputc(g, file);
            fputc(b, file);
        }
        // we print a new line after each row of pixels.
    }
    fclose(file);
}
