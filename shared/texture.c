#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include "stdint.h"
#include "library/stb_image.h"
#include "texture.h"

static bool PngLoad(const char *path, uint8 **data, int *width, int *height, int *bytes) {
    *data = stbi_load(path, width, height, bytes, 0);

    if((*bytes != 3) && (*bytes != 4)) {
        fprintf(stderr, "ERROR: Unable to load texture \"%s\".\n", path);
        fprintf(stderr, "       Not RGB or RGBA (may be paletted).\n");
        stbi_image_free(*data);
        *data = NULL;
        return false;
    }
    return true;
}

bool TextureLoad(const char *path, GLuint *handle, int *width, int *height) {
    uint8 *data; int pngWidth, pngHeight, pngBytes;
    if(!PngLoad(path, &data, &pngWidth, &pngHeight, &pngBytes)) { return false; }

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, handle);
    glBindTexture(GL_TEXTURE_2D, *handle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if(pngBytes == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pngWidth, pngHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pngWidth, pngHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D);
    
    stbi_image_free(data);
    if(width)  { *width  = pngWidth;  }
    if(height) { *height = pngHeight; }
    return true;
}

void TextureFree(uint32 handle) {
    glDeleteTextures(1, handle);
}

bool TextureLoadSlices(const char *path, GLuint **handles, int *width, int *height) {
    uint8 *data; int pngWidth, pngHeight, pngBytes;
    if(!PngLoad(path, &data, &pngWidth, &pngHeight, &pngBytes)) { return false; }

    GLuint *slices = malloc(sizeof(GLuint) * pngWidth);
    uint8 *buffer = malloc(pngHeight * pngBytes);
    int dataRowOffset, dataRowDelta = pngWidth * pngBytes, dataColOffset = 0;

    glEnable(GL_TEXTURE_1D);
    glGenTextures(pngWidth, slices);
    for(int col=0; col<pngWidth; col++) {
        dataColOffset += pngBytes;
        dataRowOffset = 0;
        for(int row=0; row<pngHeight; row++) {
            memcpy(buffer+(row*pngBytes), data+dataRowOffset+dataColOffset, pngBytes);
            dataRowOffset += dataRowDelta;
        };

        glBindTexture(GL_TEXTURE_1D, slices[col]);
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        if(pngBytes == 3) {
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8,  pngHeight, 0, GL_RGB,  GL_UNSIGNED_BYTE, buffer);
        } else {
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, pngHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        }
    }
    glDisable(GL_TEXTURE_1D);

    stbi_image_free(data);
    if(width)   { *width  = pngWidth;  }
    if(height)  { *height = pngHeight; }
    if(handles) { *handles = slices;   }
    return true;
}

void TextureFreeSlices(GLuint *handles, int width) {
    glDeleteTextures(width, handles);
    free(handles);
}
