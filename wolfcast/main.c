#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "../shared/texture.h"
#include "../shared/map.h"
#include "font.h"
#include "render.h"
#include "handle.h"
#include "wolfcast.h"

//==================================================================================================================================
#define STARTUP_SCALE_WIDTH  800
#define STARTUP_SCALE_HEIGHT 600

WolfState state;

//==================================================================================================================================
static bool InitGL();
static bool InitResources();
static void MainLoop();
static void CleanupGL();
static void CleanupResources();

//==================================================================================================================================
int main(int argc, char **argv) {
    state.scaleWidth  = STARTUP_SCALE_WIDTH;
    state.scaleHeight = STARTUP_SCALE_HEIGHT;
    state.cameraX = state.cameraY = 16.0;
    state.cameraAngle = 1.57;

    // TODO:
    if(argc > 1) {
        state.map = MapLoad(argv[1]);
    } else {
        state.map = MapLoad("maps/E0M0.map");
    }
    if(!state.map) {
        fprintf(stderr, "ERROR: MapLoad() failed.\n");
        return -1;
    }

    if(!InitGL())        { return -1; }
    if(!InitHandle())    { CleanupGL(); return -1; }
    if(!InitRender())    { CleanupGL(); CleanupHandle(); return -1; }
    if(!InitResources()) { CleanupGL(); CleanupHandle(); CleanupRender(); return -1; }

    MainLoop();

    CleanupGL();
    CleanupHandle();
    CleanupRender();
    CleanupResources();
    return 0;
}

//==================================================================================================================================
static void MainLoop() {
    double tickPrevious = glfwGetTime(), tickCurrent = glfwGetTime();
    double fpsInstant, fpsAverage = 60.0, fpsAverageSum = 0.0; int fpsAverageCount = 0;

    while(!glfwWindowShouldClose(state.window)) {
        // Get Input
        glfwPollEvents();

        // Limit FPS spin
        fpsInstant = 120.0;
        while(fpsInstant > 60.4) {
            //TODO: usleep() or nanosleep()
            tickCurrent = glfwGetTime();
            fpsInstant = 1.0 / (tickCurrent - tickPrevious);
        }
        tickPrevious = tickCurrent;

        // Average FPS smooth
        fpsAverageSum += fpsInstant; fpsAverageCount++;
        if(fpsAverageCount == 10) {
            fpsAverage = fpsAverageSum / 10.0;
            fpsAverageSum = 0.0;
            fpsAverageCount = 0;
        }

        // GL Setup
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render
        RenderView(state.cameraX, state.cameraY, state.cameraAngle);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glBegin(GL_TRIANGLES);
        // glColor4ub(255,0,0,255); glVertex3f(   0.0f,   0.0f, 0.5f);
        // glColor4ub(0,255,0,255); glVertex3f( 256.0f,   0.0f, 0.5f);
        // glColor4ub(0,0,255,255); glVertex3f(   0.0f, 144.0f, 0.5f);
        // glEnd();

        // FPS Display
        FontWriteStringF(0.0, 0.0, 10.0, 0xFFFFFFFF, "%dfps", (int)fpsAverage);
        FontWriteStringF(0.0, 8.0, 10.0, 0xFFFFFFFF, "%s", state.map->id);

        // GL Flush
        glfwSwapBuffers(state.window);

    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static bool InitGL() {
    if(!glfwInit()) {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        return false;
    }

    // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    state.window = glfwCreateWindow(state.scaleWidth, state.scaleHeight, "WolfCast", NULL, NULL);
    if(!state.window) {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        return false;
    }

    glfwMakeContextCurrent(state.window);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glfwGetFramebufferSize(state.window, &state.pixelWidth, &state.pixelHeight);
    glViewport(0, 0, state.pixelWidth, state.pixelHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, DRAW_WIDTH, 0.0, DRAW_HEIGHT, 0.0, -1.0);
    glScalef(1.0f, -1.0f, 1.0f);
    glTranslatef(0.0, -144.0, 0.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return true;
}


static void CleanupGL() {
    glfwTerminate();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static bool InitResources() {
    if(!FontLoad()) { return false; }

    state.wallTextures = malloc(sizeof(GLuint *) * 64);
    {
        char buffer[32]; int wallTextureWidth, wallTextureHeight;
        for(int index=0; index<64; index++) {
            snprintf(buffer, 32, "textures/%02d.png", index);
            if(!TextureLoadSlices(buffer, &state.wallTextures[index], &wallTextureWidth, &wallTextureHeight)) {
                return false;
            }
            if((wallTextureWidth != 64) || (wallTextureHeight != 64)) {
                fprintf(stderr, "ERROR: Texture \"%s\" wrong size (%dx%d).\n", buffer, wallTextureWidth, wallTextureHeight);
                return false;
            }
        }
    }

    return true;
}

static void CleanupResources() {
    FontFree();
    for(int index=0; index<64; index++) {
        TextureFreeSlices(state.wallTextures[index], 64);
    }
}
