#ifndef _TEXT_H
#define _TEXT_H

#include "utils/vector.h"
#include "video/texture.h"

enum {
    FONT_UNDEFINED,
    FONT_BIG,
    FONT_SMALL
};

typedef struct font_t font;

struct font_t {
    int size;
    int w,h;
    vector textures;
};

void font_create(font *font);
void font_free(font *font);
int font_load(font *font, const char* file, unsigned int size);
void font_render(font *font, const char *text, int x, int y, unsigned char r, unsigned char g, unsigned char b);

#endif // _TEXT_H