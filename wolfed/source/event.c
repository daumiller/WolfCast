#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../../shared/map.h"
#include "wolfed.h"

//==================================================================================================================================
static char *MapNamePath(const char *name);
static void AfterMapOpened(void);
static void AfterMapSaved(void);

//==================================================================================================================================
void EventResized(GLFWwindow *window, int width, int height) {
    state.viewWidth  = floor((double)(width - SIDEBAR_WIDTH) / ((double)CANVAS_BLOCK_SIZE - 1.5));
    state.viewHeight = floor((double)(height) / ((double)CANVAS_BLOCK_SIZE - 0.5));
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

    // TODO: GUI errors
    fprintf(stderr, "ERROR: %s", buffer);
    free(buffer);
}

//==================================================================================================================================
