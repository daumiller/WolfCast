#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "library/nuklear_glfw.h"
#include "../../shared/texture.h"
#include "wolfed.h"

//==================================================================================================================================
WolfEdState state;

//==================================================================================================================================
static bool InitNuklear(void);   static void CleanupNuklear(void);
static bool InitResources(void); static void CleanupResources(void);
static void MainLoop(void);

//==================================================================================================================================
int main(int argc, char ** argv) {
    memset(&state, 0, sizeof(WolfEdState));

    if(!InitNuklear())   { return -1; }
    if(!InitResources()) { return -1; }
    
    DrawInit();
    (argc > 1) ? EventMapLoad(argv[1]) : EventMapNew();
    state.nuklear->style.window.padding = nk_vec2(0.0f, 0.0f);

    MainLoop();

    CleanupResources();
    CleanupNuklear();
    if(state.mapName) { free(state.mapName); }
    if(state.map) { MapFree(state.map); }
    return 0;
}

//==================================================================================================================================
static void MainLoop() {
    while(!glfwWindowShouldClose(state.window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glfwPollEvents();
        nk_glfw_input();
        DrawUI();
        nk_glfw_render();
        glfwSwapBuffers(state.window);
    }
}

//==================================================================================================================================
static bool InitNuklear() {
    if(!glfwInit()) { fprintf(stderr, "ERROR: glfwInit() failed.\n"); return false; }
    state.window = glfwCreateWindow(STARTUP_WIDTH, STARTUP_HEIGHT, PROGRAM_NAME, NULL, NULL);
    glfwMakeContextCurrent(state.window);

    state.nuklear = nk_glfw_init(state.window, true, false);
    if(!state.nuklear) { fprintf(stderr, "ERROR: nk_glfw_init() failed.\n"); return false; }
    glfwSetKeyCallback            (state.window, EventKey);
    glfwSetCharCallback           (state.window, nk_glfw_callback_char);
    glfwSetScrollCallback         (state.window, nk_glfw_callback_scroll);
    glfwSetFramebufferSizeCallback(state.window, nk_glfw_callback_resize);
    glfwSetWindowSizeCallback     (state.window, EventResized);

    nk_glfw_font_begin(NULL);
    nk_glfw_font_end();

    return true;
}

static void CleanupNuklear() {
    nk_glfw_shutdown();
    glfwTerminate();
}

//==================================================================================================================================
static GLuint *InitResrouces_Icons() {
    GLuint *handles = malloc(sizeof(GLuint) * 10);
    if(!TextureLoad("resource/icon/edit.png",     &(handles[TOOL_EDIT    ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/place.png",    &(handles[TOOL_PLACE   ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/inspect.png",  &(handles[TOOL_INSPECT ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/settings.png", &(handles[TOOL_SETTINGS]), NULL, NULL)) { free(handles); return NULL; }

    if(!TextureLoad("resource/icon/erase.png",  &(handles[PLACE_ERASE ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/door.png",   &(handles[PLACE_DOOR  ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/spawn.png",  &(handles[PLACE_SPAWN ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/key.png",    &(handles[PLACE_KEY   ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/weapon.png", &(handles[PLACE_WEAPON]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/ammo.png",   &(handles[PLACE_AMMO  ]), NULL, NULL)) { free(handles); return NULL; }
    if(!TextureLoad("resource/icon/enemy.png",  &(handles[PLACE_ENEMY ]), NULL, NULL)) { free(handles); return NULL; }
    return handles;
}

static GLuint *InitResources_Textures() {
    char buff[128];
    int width, height;
    GLuint *handles = malloc(sizeof(GLuint) * 65);

    for(int index=0; index<65; index++) {
        snprintf(buff, 128, "textures/%02d.png", index);
        if(!TextureLoad(buff, &handles[index], &width, &height)) {
            fprintf(stderr, "ERROR: Unable to read texture file \"%s\".\n", buff);
            free(handles);
            return NULL;
        }
        if((width != 64) || (height != 64)) {
            fprintf(stderr, "ERROR: Texture file \"%s\" is wrong dimensions (%dx%d != 64x64).\n", buff, width, height);
            free(handles);
            return NULL;
        }
    }

    return handles;
}

static bool InitResources() {
    state.icons = InitResrouces_Icons();
    if(!state.icons) { fprintf(stderr, "ERROR: Missing icon file(s).\n"); return false; }

    state.textures = InitResources_Textures();
    if(!state.textures) { fprintf(stderr, "ERROR: Missing texture file(s).\n"); return false; }

    return true;
}

static void CleanupResources() {
    glDeleteTextures(10, state.icons);    free(state.icons);
    glDeleteTextures(65, state.textures); free(state.textures);
}

//==================================================================================================================================
