#ifndef WOLFED_H
#define WOLFED_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "library/nuklear.h"
#include "../../shared/map.h"

//==================================================================================================================================
#define STARTUP_WIDTH  960
#define STARTUP_HEIGHT 720
#define PROGRAM_NAME   "WolfEd"

//==================================================================================================================================
typedef enum {
    TOOL_EDIT     = 0,
    TOOL_PLACE    = 1,
    TOOL_INSPECT  = 2,
    TOOL_SETTINGS = 3
} ToolType;

typedef enum {
    PLACE_ERASE  =  4,
    PLACE_DOOR   =  5,
    PLACE_SPAWN  =  6,
    PLACE_KEY    =  7,
    PLACE_WEAPON =  8,
    PLACE_AMMO   =  9,
    PLACE_ENEMY  = 10
} PlaceType;

//==================================================================================================================================
typedef struct {
    GLFWwindow *window;
    struct nk_context *nuklear;

    GLuint *icons, *textures;
    const char *mapPath;
    Map *map;

    ToolType      selectedTool;
    unsigned char selectedEdit;
    PlaceType     selectedPlace;
    int selectedInspectX, selectedInspectY;

    int viewLeft, viewTop, viewWidth, viewHeight;
    int dataLeft, dataTop, dataWidth, dataHeight;

    bool isTextEditing;
    int shownDialog;
} WolfEdState;

//==================================================================================================================================
// main.c
extern WolfEdState state;

//==================================================================================================================================
// draw.c
void DrawInit(void);
void DrawUI(void);

//==================================================================================================================================
// event.c
void EventMapLoad(const char *path);
void EventMapNew(void);
void EventMapSave(const char *path);


#endif //WOLFED_H
