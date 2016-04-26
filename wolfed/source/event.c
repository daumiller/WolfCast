#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "library/nuklear_glfw.h"
#include "../../shared/map.h"
#include "wolfed.h"

//==================================================================================================================================
static char *MapNamePath(const char *name);
static void AfterMapOpened(void);
static void AfterMapSaved(void);

static void EventMapOpen_Request(void);
static void EventMapSave_Request(void);
static void EventMapSaveAs_Request(void);

//==================================================================================================================================
void EventResized(GLFWwindow *window, int width, int height) {
    state.viewWidth  = floor((double)(width - SIDEBAR_WIDTH) / ((double)CANVAS_BLOCK_SIZE - 1.5));
    state.viewHeight = floor((double)(height) / ((double)CANVAS_BLOCK_SIZE - 0.5));
}

void EventKey(GLFWwindow *window, int key, int code, int action, int mods) {
    bool filtered = false;

    if((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE)) {
        EventExit();
        return;
    }
    if(state.isTextEditing || state.errorMessage || state.prompt.showing) { 
        nk_glfw_callback_key(window, key, code, action, mods);
        return;
    }

    if(action == GLFW_PRESS) {
        filtered = true;
        if(mods & GLFW_MOD_SUPER) {
                 if(key == GLFW_KEY_O) { EventMapOpen_Request();   }
            else if(key == GLFW_KEY_S) { EventMapSave_Request();   }
            else if(key == GLFW_KEY_A) { EventMapSaveAs_Request(); }
            else if(key == GLFW_KEY_N) { EventMapNew();            }
            else { filtered = false; }
        } else {
                 if(key == GLFW_KEY_1) { state.selectedTool = TOOL_EDIT;     }
            else if(key == GLFW_KEY_2) { state.selectedTool = TOOL_PLACE;    }
            else if(key == GLFW_KEY_3) { state.selectedTool = TOOL_INSPECT;  }
            else if(key == GLFW_KEY_4) { state.selectedTool = TOOL_SETTINGS; }
            else { filtered = false; }
        }
    }

    if((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        filtered = true;
             if(key == GLFW_KEY_LEFT)   { EventCanvasTranslate(1, 0, 0, 0); }
        else if(key == GLFW_KEY_RIGHT)  { EventCanvasTranslate(0, 0, 1, 0); }
        else if(key == GLFW_KEY_UP)     { EventCanvasTranslate(0, 1, 0, 0); }
        else if(key == GLFW_KEY_DOWN)   { EventCanvasTranslate(0, 0, 0, 1); }
        else { filtered = false; }
    }

    if(!filtered) { nk_glfw_callback_key(window, key, code, action, mods); }
}

void EventCanvasTranslate(int left, int up, int right, int down) {
    state.viewLeft -= left;
    state.viewLeft += right;
    state.viewTop  -= up;
    state.viewTop  += down;

    int minViewLeft = state.dataLeft - (state.viewWidth  - 1);
    int minViewTop  = state.dataTop  - (state.viewHeight - 1);
    int maxViewLeft = state.dataLeft + state.dataWidth  - 1;
    int maxViewTop  = state.dataTop  + state.dataHeight - 1;

    if(state.viewLeft < minViewLeft) { state.viewLeft = minViewLeft; }
    if(state.viewLeft > maxViewLeft) { state.viewLeft = maxViewLeft; }
    if(state.viewTop  < minViewTop ) { state.viewTop  = minViewTop ; }
    if(state.viewTop  > maxViewTop ) { state.viewTop  = maxViewTop ; }
}

void EventExit() {
    if(state.errorMessage) {
        free(state.errorMessage);
        state.errorMessage = NULL;
        return;
    }
    if(state.prompt.showing) {
        state.prompt.callback(false);
        return;
    }
    glfwSetWindowShouldClose(state.window, GL_TRUE);
}

//==================================================================================================================================
void EventMapLoad(const char *name) {
    char *path = MapNamePath(name);
    Map *map = MapLoad(path);
    if(!map) {
        EventError("Unable to read map \"%s\".", name);
        free(path);
        return;
    }

    if(state.mapName) { free(state.mapName); }
    if(state.map) { MapFree(state.map); }
    state.mapName = strdup(name);
    state.map = map;
    free(path);

    AfterMapOpened();
}

void EventMapNew() {
    if(state.mapName) { free(state.mapName); }
    if(state.map) { MapFree(state.map); }
    state.mapName = NULL;
    state.map = MapNew(state.viewWidth, state.viewHeight);
    AfterMapOpened();
}

void EventMapSave(const char *name) {
    char *path = MapNamePath(name);
    if(!MapSave(state.map, path)) {
        EventError("Error saving map \"%s\".", name);
        free(path);
        return;
    }
    if(name != state.mapName) {
        if(state.mapName) { free(state.mapName); }
        state.mapName = strdup(name);
    }
    AfterMapSaved();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void PromptDialogSetup(char *title, char *message, char *suggestion, void (*callback)(bool success)) {
    state.prompt.title = title;
    state.prompt.message = message;
    if(suggestion) {
        strncpy(state.prompt.input, suggestion, 128);
    } else {
        state.prompt.input[0] = 0x00;
    }
    state.prompt.callback = callback;
    state.prompt.showing = true;
}

static void PromptDialogTeardown() {
    state.prompt.showing = false;
    free(state.prompt.title); state.prompt.title = NULL;
    free(state.prompt.message); state.prompt.message = NULL;
    state.prompt.input[0] = 0x00;
    state.prompt.callback = NULL;
}

static void EventMapOpen_Callback(bool proceed) {
    if(proceed) { EventMapLoad(state.prompt.input); }
    PromptDialogTeardown();
}

static void EventMapSaveAs_Callback(bool proceed) {
    if(proceed) { EventMapSave(state.prompt.input); }
    PromptDialogTeardown();
}

static void EventMapOpen_Request() {
    PromptDialogSetup(strdup("Open Map"), strdup("Map Name:"), NULL, EventMapOpen_Callback);
}

static void EventMapSaveAs_Request() {
    PromptDialogSetup(strdup("Save Map"), strdup("Map Name:"), NULL, EventMapSaveAs_Callback);
}

static void EventMapSave_Request() {
    state.mapName ? EventMapSave(state.mapName) : EventMapSaveAs_Request();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static char *MapNamePath(const char *name) {
    static const char *format = "maps/%s.map";
    int bufferSize = strlen(name) + strlen(format);
    char *buffer = malloc(bufferSize);
    snprintf(buffer, bufferSize, format, name);
    return buffer;
}

static void AfterMapOpened() {
    state.viewLeft   = state.viewTop = 0;
    state.dataLeft   = state.dataTop = 0;
    state.dataWidth  = state.map->blockWidth;
    state.dataHeight = state.map->blockHeight;

    // if data is smaller than view, center view
    if(state.dataWidth < state.viewWidth) {
        state.viewLeft = -((state.viewWidth - state.dataWidth) >> 1);
    }
    if(state.dataHeight < state.viewHeight) {
        state.viewTop = -((state.viewHeight - state.dataHeight) >> 1);
    }
}

static void AfterMapSaved(void) {
    // map may contract while saving
    int oldWidth = state.dataWidth, oldHeight = state.dataHeight;
    state.dataWidth  = state.map->blockWidth;
    state.dataHeight = state.map->blockHeight;
    if(state.dataWidth  < oldWidth ) { state.viewLeft -= (oldWidth  - state.dataWidth);  }
    if(state.dataHeight < oldHeight) { state.viewTop  -= (oldHeight - state.dataHeight); }
}

//==================================================================================================================================
void EventCanvasView(int x, int y) {
    // clicked on a canvas tile that is in the view, but not in the data (part of the map)
    if(state.selectedTool == TOOL_INSPECT) {
        // select blank/erase
        state.selectedEdit = 0;
        state.selectedPlace = PLACE_ERASE;
    } else if((state.selectedTool == TOOL_EDIT) && (state.selectedEdit > 0)) {
        // grow map
        int left=0, right=0, top=0, bottom=0;
        if(x < state.dataLeft) { left = (state.dataLeft - x); }
        if(y < state.dataTop ) { top  = (state.dataTop  - y); }
        if(x >= (state.dataLeft + state.dataWidth )) { right  = x - (state.dataLeft + state.dataWidth  - 1); }
        if(y >= (state.dataTop  + state.dataHeight)) { bottom = y - (state.dataTop  + state.dataHeight - 1); }
        MapExpand(state.map, left, top, right, bottom);
        // adjust view relative to data
        if(left) { state.viewLeft += left; }
        if(top ) { state.viewTop  += top;  }
        // update data bounds
        state.dataWidth  = state.map->blockWidth;
        state.dataHeight = state.map->blockHeight;
        // set wallGrid
        state.map->wallGrid[y+top][x+left] = state.selectedEdit;
    }
}

void EventCanvasData(int x, int y) {
    switch(state.selectedTool) {
        case TOOL_EDIT:
            state.map->wallGrid[y][x] = state.selectedEdit;
            break;

        case TOOL_PLACE:
            if(state.map->entityGrid[y][x] > 0) {
                MapEntityDel(state.map, state.map->entityGrid[y][x] - 1);
            }
            if(state.selectedPlace > PLACE_ERASE) {
                // place entity centered on block
                MapEntityAdd(state.map,
                    (x << UNIT_BLOCK_SHIFT) + (UNITS_PER_BLOCK >> 1),
                    (y << UNIT_BLOCK_SHIFT) + (UNITS_PER_BLOCK >> 1),
                    state.selectedPlace);
            }
            break;

        case TOOL_INSPECT:
            state.selectedInspectX = x;
            state.selectedInspectY = y;
            state.selectedEdit = state.map->wallGrid[y][x];
            if(state.map->entityGrid[y][x] > 0) {
                state.selectedPlace = state.map->entity[state.map->entityGrid[y][x] - 1]->type;
            } else {
                state.selectedPlace = PLACE_ERASE;
            }
            break;

        case TOOL_SETTINGS: break;
    }
}

//==================================================================================================================================
void EventError(const char *format, ...) {
    char *buffer = malloc(256);

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    if(state.errorMessage) { 
        // whoops
        fprintf(stderr, "ERROR: Error message overflow.\n");
        free(state.errorMessage);
    }
    state.errorMessage = buffer;
}

//==================================================================================================================================
