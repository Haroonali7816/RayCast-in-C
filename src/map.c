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

    // We now initialize the default/starting values.
    map->wall_count = 0;
    map->x_start = -1;
    map->y_start = -1;

    // Now we allocate the 2D grid.

    map->grid = malloc(map->height * sizeof(char *));
    // Parse the file row by row using fgetc
    for (int y = 0; y < map->height; y++) {
        map->grid[y] = malloc(map->width * sizeof(char));

        // Skip any leading newlines/carriage returns between rows
        int c = fgetc(file);
        while (c == '\n' || c == '\r') {
            c = fgetc(file);
        }

        for (int x = 0; x < map->width; x++) {
            // If we hit EOF or a newline mid-row, fill the rest with spaces
            if (c == EOF || c == '\n' || c == '\r') {
                map->grid[y][x] = ' ';
                continue;
            }

            map->grid[y][x] = (char)c;

            // Track stats requested by Task 1
            if (c == 'S') {
                map->x_start = x;
                map->y_start = y;
            } else if (c != ' ') {
                map->wall_count++;
            }

            // Read next character only if we still have columns left
            if (x + 1 < map->width) {
                c = fgetc(file);
            }
        }
    }
    fclose(file);
    return map;
}
