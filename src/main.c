#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "minimap.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <map_file> <command_flag>\n", argv[0]);
        return 1;
    }
    const char *map_file = argv[1];
    const char *command = argv[2];

    // Delegate the parsing logic to map.c
    Map *map = read_map(map_file);
    if (!map) return 1;

    // Handle Task 1: The -I flag
    if (strcmp(command, "-I") == 0) {
        printf("Map dimensions: %d x %d\n", map->width, map->height);
        printf("Start point: (%d, %d)\n", map->x_start, map->y_start);
        printf("Total wall cells: %d\n", map->wall_count);
    } else if (strcmp(command, "-M") == 0) {
        // Handle Task 2: The -M flag
        const char *output_filename = argv[3];

        create_minimap(map, output_filename);
    } else {
        fprintf(stderr, "Error: Unknown command %s\n", command);
    }

    // Clean up memory
    free_map(map);
    return 0;
}