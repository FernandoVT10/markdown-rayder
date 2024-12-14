#ifndef IMAGE_H_
#define IMAGE_H_

typedef struct ImageNode {
    Texture2D texture;
    Image image;
    char *alt;
    char *url;
    bool loading_image;
    bool texture_loaded;
} ImageNode;

typedef struct ImageChunk {
    char *data;
    size_t size;
} ImageChunk;

void image_loader_init();
void image_loader_destroy();
void free_image_node(ImageNode *node);
void image_loader_load(ImageNode *node);

Vector2 draw_image_node(Vector2 pos, int screen_width, ImageNode *node);

#endif
