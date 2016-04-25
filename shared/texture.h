#ifndef WOLFCAST_TEXTURE_H
#define WOLFCAST_TEXTURE_H

#include <stdbool.h>
#include "stdint.h"

bool TextureLoad(const char *path, uint32 *handle, int *width, int *height);
void TextureFree(uint32 handle);
bool TextureLoadSlices(const char *path, uint32 **handles, int *width, int *height);
void TextureFreeSlices(uint32 *handles, int width);

#endif //WOLFCAST_TEXTURE_H
