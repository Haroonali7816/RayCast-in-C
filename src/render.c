#include "render.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846

int is_wall(Map *map, int x, int y) {
    //  out of bounds is NOT a wall. The ray should just escape.
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 0;
    }
    char cell = map->grid[y][x];
    if (cell == ' ' || cell == 'S') {
        return 0;
    }
    return 1;
}

void write_pixel(FILE *file, unsigned char r, unsigned char g, unsigned char b) {
    fputc(r, file);
    fputc(g, file);
    fputc(b, file);
}

void render_map(Map *map, const char *output_file, int width, int height, double fov, double rotation, double pos_x,
                double pos_y) {
    FILE *file = fopen(output_file, "wb");

    if (!file) {
        fprintf(stderr, "Error: Could not create %s\n", output_file);
        return;
    }

    fprintf(file, "P6\n%d %d\n255\n", width, height);

    double fov_rad = fov * PI / 180.0;
    double rotation_rad = rotation * PI / 180.0;

    double dx = cos(rotation_rad);
    double dy = sin(rotation_rad);
    double pz = 0.5;

    double focal = width / (2.0 * tan(fov_rad / 2.0));
    double cx = pos_x - (dx * focal);
    double cy = pos_y - (dy * focal);

    double *column_t = malloc(width * sizeof(double));
    int *column_side = malloc(width * sizeof(int));
    int *column_hit = malloc(width * sizeof(int));  // Track if the ray actually hit a wall

    for (int x = 0; x < width; x++) {
        double ix = cx + (x - (width / 2.0) + 0.5) * dy;
        double iy = cy - (x - (width / 2.0) + 0.5) * dx;

        double rx = pos_x - ix;
        double ry = pos_y - iy;

        int map_x = (int)floor(pos_x);
        int map_y = (int)floor(pos_y);

        double length = sqrt(rx * rx + ry * ry);
        double ux = rx / length;
        double uy = ry / length;

        double sw = (ux == 0.0) ? INFINITY : fabs(1.0 / ux);
        double sh = (uy == 0.0) ? INFINITY : fabs(1.0 / uy);

        double sx, sy;
        int step_x, step_y;

        if (ux < 0) {
            step_x = -1;
            sx = (pos_x - map_x) * sw;
        } else {
            step_x = 1;
            sx = (map_x + 1.0 - pos_x) * sw;
        }

        if (uy < 0) {
            step_y = -1;
            sy = (pos_y - map_y) * sh;
        } else {
            step_y = 1;
            sy = (map_y + 1.0 - pos_y) * sh;
        }

        int hit = 0;
        int hit_x_side = 0;

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

            // If ray leaves the map, it hits nothing. Break and leave hit = 0.
            if (map_x < 0 || map_x >= map->width || map_y < 0 || map_y >= map->height) {
                break;
            }

            if (is_wall(map, map_x, map_y)) {
                hit = 1;
            }
        }

        column_hit[x] = hit;

        if (hit) {
            double wx, wy, t;
            // Calculate the exact hit point and distance t along the ray.
            if (hit_x_side) {
                wx = (step_x > 0) ? map_x : (map_x + 1.0);
                t = (wx - pos_x) / rx;
            } else {
                wy = (step_y > 0) ? map_y : (map_y + 1.0);
                t = (wy - pos_y) / ry;
            }
            column_t[x] = t;
            column_side[x] = hit_x_side;
        }
    }

    for (int y = 0; y < height; y++) {
        double iz = y - (height / 2.0) + 1.0;
        double rz = pz - iz;

        for (int x = 0; x < width; x++) {
            // If the ray didn't hit anything, color black.
            if (!column_hit[x]) {
                write_pixel(file, 0, 0, 0);
                continue;
            }

            double wz = pz + column_t[x] * rz;

            if (wz > -0.0001 && wz < 0.0) {
                wz = 0.0;
            }
            if (wz > 1.0 && wz < 1.0001) {
                wz = 1.0;
            }

            if (wz >= 0.0 && wz <= 1.0) {
                if (column_side[x]) {
                    write_pixel(file, 255, 0, 0);  // Red for y-axis sides
                } else {
                    write_pixel(file, 0, 255, 0);  // Green for x-axis sides
                }
            } else {
                write_pixel(file, 0, 0, 0);  // Black for floor/ceiling
            }
        }
    }

    free(column_t);
    free(column_side);
    free(column_hit);

    fclose(file);
}