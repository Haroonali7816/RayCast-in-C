#ifndef TEXTURE_H
#define TEXTURE_H

// TextureAtlas structure to hold texture data.
typedef struct {
    int width;
    int height;
    int count_x;
    int count_y;
    int tex_w;
    int tex_h;
    unsigned char *data;
} TextureAtlas;

// FUunction prototype for loading texture file from PPM format.
TextureAtlas *load_texture_atlas(const char *filename, int count_x, int count_y);

// freeing the memory allocated for texture atlas.
void free_texture_atlas(TextureAtlas *atlas);

#endif  // TEXTURE_H