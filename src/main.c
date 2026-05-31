#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "minimap.h"
#include "render.h"

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

    // This handles task 1: The -I flag
    if (strcmp(command, "-I") == 0) {
        printf("Map dimensions: %d x %d\n", map->width, map->height);
        printf("Start point: (%d, %d)\n", map->x_start, map->y_start);
        printf("Total wall cells: %d\n", map->wall_count);
    } else if (strcmp(command, "-M") == 0) {
        // This  handles task 2: The -M flag
        const char *output_filename = argv[3];
        create_minimap(map, output_filename);
    } else if (strcmp(command, "-R") == 0) {
        //  This handles task 3 : The -R flag
        if (argc < 10) {
            fprintf(
                stderr,
                "Usage: %s <map_file> -R <output_image> <width> <height> <fov> <rotation> <position x> <position y>\n",
                argv[0]);

            free_map(map);

            return 1;
        }

        const char *output_file = argv[3];
        int width = atoi(argv[4]);
        int height = atoi(argv[5]);
        double fov = atof(argv[6]);
        double rotation = atof(argv[7]);
        double pos_x = atof(argv[8]);
        double pos_y = atof(argv[9]);

        render_map(map, output_file, width, height, fov, rotation, pos_x, pos_y);
    } else {
        fprintf(stderr, "Error: Unknown command %s\n", command);
    }

    // Clean up memory
    free_map(map);
    return 0;
}
