#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A helper function  to reliably skip whitespace and '#' comments in PPM headers.
static int get_next_int(FILE *f) {
    int ch;
    while (1) {
        ch = fgetc(f);
        if (ch == EOF) return -1;
        if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') continue;
        if (ch == '#') {
            while ((ch = fgetc(f)) != '\n' && ch != EOF);
            continue;
        }
        ungetc(ch, f);
        int val;
        if (fscanf(f, "%d", &val) != 1) return -1;
        return val;
    }
}

TextureAtlas *load_texture_atlas(const char *filename, int count_x, int count_y) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    // We read the PPM header manually to handle comments and whitespaces.
    char header[3];
    if (fscanf(f, "%2s", header) != 1 || strcmp(header, "P6") != 0) {
        fclose(f);
        return NULL;
    }

    int w = get_next_int(f);
    int h = get_next_int(f);
    int maxval = get_next_int(f);

    // consume the exact single whitespace character after maxval
    fgetc(f);

    TextureAtlas *atlas = malloc(sizeof(TextureAtlas));
    if (!atlas) {
        fclose(f);
        return NULL;
    }
    // we assign the texture atlas data based on the loaded PPM image.
    atlas->width = w;
    atlas->height = h;
    atlas->count_x = count_x;
    atlas->count_y = count_y;
    atlas->tex_w = w / count_x;
    atlas->tex_h = h / count_y;

    atlas->data = malloc(w * h * 3);  // since we have 3 bytes per pixel (RGB) that is why we multiply by 3.
    if (atlas->data) {
        fread(atlas->data, 1, w * h * 3, f);
    }

    fclose(f);
    return atlas;
}

// we free the memory allocated for texture atlas.
void free_texture_atlas(TextureAtlas *atlas) {
    if (!atlas) return;
    if (atlas->data) free(atlas->data);
    free(atlas);
}