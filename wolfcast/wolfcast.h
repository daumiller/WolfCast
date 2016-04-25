#ifndef WOLFCAST_H
#define WOLFCAST_H

#include <GLFW/glfw3.h>
#include "../shared/map.h"

//==================================================================================================================================
#define DRAW_WIDTH  256.0
#define DRAW_HEIGHT 144.0

//==================================================================================================================================
typedef struct {
    GLFWwindow *window;
    int pixelWidth, pixelHeight;
    int scaleWidth, scaleHeight;
    GLuint **wallTextures;
    Map *map;
    double cameraX, cameraY, cameraAngle;
} WolfState;

extern WolfState state;

#endif //WOLFCAST_H
