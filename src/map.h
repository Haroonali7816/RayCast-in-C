#ifndef MAP_H
#define MAP_H

// we define the data structure that we will use
typedef struct{
    int width;
    int height;
    char **grid;
    int x_start;
    int y_start;
    int wall_count;
} Map;

//Now we declare the function prototype.
Map* read_map(const char *filename);
void free_map (Map *map);

#endif //MAP_H