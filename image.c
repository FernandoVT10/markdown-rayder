#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "raylib.h"
#include "raymath.h"
#include "image.h"

pthread_mutex_t mutex_lock;

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *chunk)
{
    size_t real_size = size * nmemb;
    ImageChunk *image_chunk = (ImageChunk *)chunk;

    char *ptr = realloc(image_chunk->data, image_chunk->size + real_size + 1);
    if(!ptr) {
        TraceLog(LOG_ERROR, "Error trying to reallocate memory for the image chunk");
        return 0;
    }

    image_chunk->data = ptr;
    memcpy(&(image_chunk->data[image_chunk->size]), contents, real_size);
    image_chunk->size += real_size;
    image_chunk->data[image_chunk->size] = 0;

    return real_size;
}

// dest should allocate enough data for 4 chars and one for null char
// NOTE: if the extension is not found or the extension is bigger than 4 chars
// we do nothing to the dest string
void get_image_ext(char *dest, const char *path)
{
    int dot_pos = -1;

    for(size_t i = strlen(path) - 1; i > 0; i--) {
        if(path[i] != '.') continue;
        dot_pos = i;
        break;
    }

    // image extensions usually are not longer than 4 chars
    if(dot_pos < 0 || strlen(path + dot_pos) > 4) return;

    strcpy(dest, path + dot_pos);
}

void *load_image_from_url(void *arg)
{
    ImageNode *image_node = (ImageNode *)arg;

    CURL *curl_handle;
    CURLcode res;

    ImageChunk chunk = {
        .data = malloc(1),
        .size = 0,
    };

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, image_node->url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        TraceLog(LOG_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return NULL;
    }

    char image_ext[5] = ".jpg";
    get_image_ext(image_ext, image_node->url);

    pthread_mutex_lock(&mutex_lock);
    image_node->image = LoadImageFromMemory(image_ext, (unsigned char *)chunk.data, chunk.size);
    image_node->loading_image = false;
    pthread_mutex_unlock(&mutex_lock);

    if(!IsImageValid(image_node->image)) {
        TraceLog(LOG_ERROR, "The given url %s is not a valid image", image_node->url);
        return NULL;
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.data);
    curl_global_cleanup();

    return NULL;
}

void image_loader_init(ImageNode *node)
{
    pthread_mutex_init(&mutex_lock, NULL);
}

Vector2 draw_image_node(Vector2 pos, int screen_width, ImageNode *node)
{
    if(node->loading_image) {
        return Vector2Zero();
    }

    if(!node->texture_loaded) {
        node->texture_loaded = true;
        node->texture = LoadTextureFromImage(node->image);
    }

    Vector2 image_size = {0};

    if(node->texture.width > screen_width) {
        float scale = screen_width / (float)node->texture.width;
        DrawTextureEx(node->texture, pos, 0, scale, WHITE);
        image_size.x = screen_width;
        image_size.y = node->texture.height * scale;
    } else {
        DrawTexture(node->texture, pos.x, pos.y, WHITE);
        image_size.x = node->texture.width;
        image_size.y = node->texture.height;
    }

    return image_size;
}

void image_loader_async_load(ImageNode *node)
{
    pthread_t tid;
    pthread_create(&tid, NULL, &load_image_from_url, node);
}

void free_image_node(ImageNode *node)
{
    free(node->alt);
    free(node->url);
    UnloadTexture(node->texture);
    UnloadImage(node->image);
}

void image_loader_destroy()
{
    pthread_mutex_destroy(&mutex_lock);
}
