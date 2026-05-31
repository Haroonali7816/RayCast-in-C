#include "map.h"

#include <stdio.h>
#include <stdlib.h>

void free_map(Map *map) {
    if (!map) return;

    if (map->grid) {
        // Free each row first
        for (int y = 0; y < map->height; y++) {
            free(map->grid[y]);
        }
        // Then free the array of row pointers
        free(map->grid);
    }
    // Finally, free the struct itself
    free(map);
}

Map *read_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open a file %s\n", filename);
        return NULL;
    }

    Map *map = malloc(sizeof(Map));
    if (!map) {
        fclose(file);
        return NULL;
    }

    // Now we read width and Height from the file.
    if (fscanf(file, "%d %d", &map->width, &map->height) != 2) {
        free(map);
        fclose(file);
        return NULL;
    }

    // After reading the dimensions, we need to consume the rest of the line to prepare for reading the grid.
    int flc;
    while ((flc = fgetc(file)) != '\n' && flc != EOF);

    map->wall_count = 0;
    map->x_start = -1;
    map->y_start = -1;

    map->grid = malloc(map->height * sizeof(char *));

    for (int y = 0; y < map->height; y++) {
        map->grid[y] = malloc(map->width * sizeof(char));

        for (int x = 0; x < map->width; x++) {
            int c = fgetc(file);

            // If the row ends early, pad with spaces
            if (c == '\n' || c == '\r' || c == EOF) {
                map->grid[y][x] = ' ';

                if (c == '\r') fgetc(file);
            } else {
                map->grid[y][x] = (char)c;

                if (c == 'S') {
                    map->x_start = x;
                    map->y_start = y;
                } else if (c != ' ') {
                    map->wall_count++;
                }
            }
        }
        // Consume the rest of the line if it is longer than the width.
        int trl;
        while ((trl = fgetc(file)) != '\n' && trl != EOF);
    }
    fclose(file);
    return map;
}
