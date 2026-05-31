#include "render.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// first of all we define PI.
#define PI 3.14159265358979323846

// First of all we check if a map cell is a wall or not.
int is_wall(Map *map, int x, int y) {
    // anything outside the map is  a wall.
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 1;
    }
    char cell = map->grid[y][x];
    // empty spaces and S is not wall.
    if (cell == ' ' || cell == 'S') {
        return 0;
    }
    return 1;
}
// helper function to write colors of a pixel to the file.
void write_pixel(FILE *file, unsigned char r, unsigned char g, unsigned char b) {
    fputc(r, file);
    fputc(g, file);
    fputc(b, file);
}

// main rendering function.
void render_map(Map *map, const char *output_file, int width, int height, double fov, double rotation, double pos_x,
                double pos_y) {
    FILE *file = fopen(output_file, "wb");

    if (!file) {
        fprintf(stderr, "Error: Could not create %s\n", output_file);
        return;
    }
    // Write PPM P6 header

    fprintf(file, "P6\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n");

    // we convert the degrees to radians.
    double fov_rad = fov * PI / 180.0;
    double rotation_rad = rotation * PI / 180.0;

    // now we make the direction vector.
    double dx = cos(rotation_rad);
    double dy = sin(rotation_rad);

    // we know that the camera height is 0.5
    double pz = 0.5;

    // we calculate the focal distance.
    double focal = width / (2.0 * tan(fov_rad / 2.0));

    // we calculate the principal point.
    double cx = pos_x - (dx * focal);
    double cy = pos_y - (dy * focal);

    /* store one result per column */
    double *column_t = malloc(width * sizeof(double));
    int *column_side = malloc(width * sizeof(int));

    // the main loop that handles the raycasting for every column.

    for (int x = 0; x < width; x++) {
        // start point of ray.(ix,iy)
        double ix = cx + (x - (width / 2.0) + 0.5) * dy;
        double iy = cy - (x - (width / 2.0) + 0.5) * dx;

        double rx = pos_x - ix;
        double ry = pos_y - iy;

        double length = sqrt(rx * rx + ry * ry);
        //(ux,uy) is the unit vector for the ray.
        double ux = rx / length;
        double uy = ry / length;

        int map_x = (int)pos_x;
        int map_y = (int)pos_y;

        double sw = fabs(1.0 / ux);
        double sh = fabs(1.0 / uy);

        double sx;
        double sy;

        int step_x;
        int step_y;

        if (ux < 0) {
            sx = (pos_x - map_x) * sw;
            step_x = -1;
        } else {
            sx = (map_x + 1.0 - pos_x) * sw;
            step_x = 1;
        }

        if (uy < 0) {
            sy = (pos_y - map_y) * sh;
            step_y = -1;
        } else {
            sy = (map_y + 1.0 - pos_y) * sh;
            step_y = 1;
        }

        int hit = 0;
        int hit_x_side = 0;  // to track if we have hit a vertical wall (1) or not(0)

        while (!hit) {
            if (sx < sy) {
                sx += sw;
                map_x += step_x;
                hit_x_side = 1;
            } else {
                sy += sh;
                map_y += step_y;
                hit_x_side = 0;
            }

            if (is_wall(map, map_x, map_y)) {
                hit = 1;
            }
        }

        double distance;

        if (hit_x_side) {
            distance = sx - sw;
        } else {
            distance = sy - sh;
        }
        // wx and wy are exact coordinates of the point we hit the wall.
        double wx = pos_x + distance * ux;
        double wy = pos_y + distance * uy;

        double t;

        if (fabs(rx) > 0.000001) {
            t = (wx - pos_x) / rx;
        } else {
            t = (wy - pos_y) / ry;
        }

        column_t[x] = t;
        column_side[x] = hit_x_side;
    }

    // The main rendering phase.

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;

            double iz = y - height / 2.0 + 1.0;

            double rz = pz - iz;

            double wz = pz + column_t[x] * rz;

            if (wz > -0.0001 && wz < 0.0) {
                wz = 0.0;
            }

            if (wz > 1.0 && wz < 1.0001) {
                wz = 1.0;
            }

            if (wz >= 0.0 && wz <= 1.0) {  // if 0<=wz<=1.0, then we know ray hits at this exact vertical point;
                if (column_side[x]) {
                    r = 255;
                    g = 0;
                    b = 0;

                } else {
                    r = 0;
                    g = 255;
                    b = 0;
                }
            }

            write_pixel(file, r, g, b);
        }
    }

    free(column_t);
    free(column_side);

    fclose(file);
}