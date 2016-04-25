#include <stdio.h>
#include <GLFW/glfw3.h>
#include "../shared/texture.h"
#include "font.h"

#define FONT_TEXTURE_DELTA 0.0625  /* 1/16 */
#define FONT_CHAR_HEIGHT   1.00
#define FONT_CHAR_WIDTH    0.5
#define FONT_CHAR_TRIM_X   0.014

static GLuint fontTexture;

bool FontLoad() {
    int width, height;
    bool result = TextureLoad("font/font.png", &fontTexture, &width, &height);
    if(result) {
        if(width != height) {
            fprintf(stderr, "ERROR: Font file not square dimensions.\n");
            return false;
        }
    }
    return result;
}

static double FontWriteChar(double left, double top, double scale, uint32 color, char ch) {
    int textureCol = ((unsigned char)ch) & 0x0F;
    int textureRow = (((unsigned char)ch) & 0xF0) >> 4;
    double textureLeft = FONT_TEXTURE_DELTA * (double)textureCol;
    double textureTop  = FONT_TEXTURE_DELTA * (double)textureRow;
    double textureRight  = textureLeft + FONT_TEXTURE_DELTA;
    double textureBottom = textureTop  + FONT_TEXTURE_DELTA;
    textureLeft += FONT_CHAR_TRIM_X;
    textureRight -= FONT_CHAR_TRIM_X;

    double right  = left + (FONT_CHAR_WIDTH  * scale);
    double bottom = top  + (FONT_CHAR_HEIGHT * scale);

    glColor4ubv((const GLubyte *)&color);
    glTexCoord2d(textureLeft,  textureTop   ); glVertex3d(left,  top,    0.1);
    glTexCoord2d(textureRight, textureTop   ); glVertex3d(right, top,    0.1);
    glTexCoord2d(textureRight, textureBottom); glVertex3d(right, bottom, 0.1);
    glTexCoord2d(textureLeft,  textureBottom); glVertex3d(left,  bottom, 0.1);

    return right - left;
}

int FontWriteString(double x, double y, double scale, uint32 color, const char *string) {
    int wrote = 0;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    while(*string) {
        x += FontWriteChar(x, y, scale, color, *string);
        string++;
        wrote++;
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    return wrote;
}

int FontWriteStringF(double x, double y, double scale, uint32 color, const char *format, ...) {
    static char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    return FontWriteString(x, y, scale, color, buffer);
}

void FontFree() {
}
