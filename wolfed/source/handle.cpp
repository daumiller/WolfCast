#include <math.h>
#include <stdio.h>
#include "wolfed.h"
extern State state;

static void afterMapOpened();
static void afterMapSaved();

//==================================================================================================================================
void HandleGlfwError(int code, const char *desc) {
    fprintf(stderr, "ERROR: GLFW %d, \"%s\".\n", code, desc);
}

void HandleResize(GLFWwindow *window, int fbWidth, int fbHeight) {
    glfwGetWindowSize(window, &(state.width), &(state.height));
    state.fbWidth = fbWidth;
    state.fbHeight = fbHeight;
    state.tileViewWidth = floor((double)(state.width - 216) / 34.0);
    state.tileViewHeight = floor((double)(state.height - 16) / 34.0);
    glViewport(0, 0, fbWidth, fbHeight);
}

void HandleKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
    bool filtered = false;

    if(action == GLFW_PRESS) {
        if(mods & GLFW_MOD_SUPER) {
                 if(key == GLFW_KEY_O) { HandleOpen();   }
            else if(key == GLFW_KEY_S) { HandleSave();   }
            else if(key == GLFW_KEY_A) { HandleSaveAs(); }
            else if(key == GLFW_KEY_N) { HandleNew();    }
            else if(key == GLFW_KEY_Q) { HandleExit();   } // ⌘Q never actually gets received
        } else {
                 if(key == GLFW_KEY_ESCAPE) { HandleExit(); }
            else if(key == GLFW_KEY_1) { state.ifNotTextEditing = HandleToolEdit;     }
            else if(key == GLFW_KEY_2) { state.ifNotTextEditing = HandleToolPlace;    }
            else if(key == GLFW_KEY_3) { state.ifNotTextEditing = HandleToolInspect;  }
            else if(key == GLFW_KEY_4) { state.ifNotTextEditing = HandleToolSettings; }
        }
    }

    if((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
             if(key == GLFW_KEY_LEFT)   { state.ifNotTextEditing = HandleViewOffsetRight; }
        else if(key == GLFW_KEY_RIGHT)  { state.ifNotTextEditing = HandleViewOffsetLeft;  }
        else if(key == GLFW_KEY_UP)     { state.ifNotTextEditing = HandleViewOffsetDown;  }
        else if(key == GLFW_KEY_DOWN)   { state.ifNotTextEditing = HandleViewOffsetUp;    }
    }

    if(!filtered) { ImGui_ImplGlFw_KeyCallback(window, key, scancode, action, mods); }
}

void HandleOpen() {
    state.showDialogOpen = true;
}
void HandleOpenFile(const char *path) {
    char *buff = (char *)malloc(strlen(path) + 9);
    sprintf(buff, "map/%s.map", path);
    Map *opened = MapLoad(buff);
    if(!opened) {
        state.errorMessage = "File not found.";
        state.showDialogError = true;
        free(buff);
        return;
    }

    if(state.mapPath) { free(state.mapPath); }
    if(state.map) { MapFree(state.map); }
    state.mapPath = strdup(buff);
    state.map = opened;
    free(buff);
    afterMapOpened();
}

void HandleNew() {
    // TODO: Save Before New?
    if(state.map) { MapFree(state.map); }
    state.map = MapNew(state.tileViewWidth, state.tileViewHeight);
    afterMapOpened();
}

void HandleSave() {
    if(!state.mapPath) { HandleSaveAs(); return; }
    if(!MapSave(state.map, state.mapPath)) {
        state.errorMessage = "Error saving file.";
        state.showDialogError = true;
        return;
    }
    afterMapSaved();
}
void HandleSaveAs() {
    state.showDialogSaveAs = true;
}
void HandleSaveAsFile(const char *path) {
    char *buff = (char *)malloc(strlen(path) + 9);
    sprintf(buff, "map/%s.map", path);
    if(!MapSave(state.map, buff)) {
        state.errorMessage = "Error saving file.";
        state.showDialogError = true;
        free(buff);
        return;
    }
    if(state.mapPath) { free(state.mapPath); }
    state.mapPath = strdup(buff);
    free(buff);
    afterMapSaved();
}

void HandleExit() {
    glfwSetWindowShouldClose(state.window, GL_TRUE);
}

void HandleToolEdit()     { state.toolSelected = TOOL_EDIT;     }
void HandleToolPlace()    { state.toolSelected = TOOL_PLACE;    }
void HandleToolInspect()  { state.toolSelected = TOOL_INSPECT;  }
void HandleToolSettings() { state.toolSelected = TOOL_SETTINGS; }

void HandleEditSelect(uint8 tile) {
    state.editTileSelected = tile;
}

void HandlePlaceSelect(uint8 place) {
    state.placeSelected = place;
}

void HandleCanvasData(int tileX, int tileY) {
    int entityIndex = state.map->entityGrid[tileY][tileX] - 1;

    if(state.toolSelected == TOOL_EDIT) {
        state.map->wallGrid[tileY][tileX] = state.editTileSelected;
    } else if(state.toolSelected == TOOL_PLACE) {
        if(entityIndex > -1) { MapEntityDel(state.map, entityIndex); }
        if(state.placeSelected > PLACE_ERASE) {
            MapEntityAdd(state.map,
                (tileX << UNIT_BLOCK_SHIFT) + (UNITS_PER_BLOCK >> 1),
                (tileY << UNIT_BLOCK_SHIFT) + (UNITS_PER_BLOCK >> 1),
                state.placeSelected);
        }
    } else if(state.toolSelected == TOOL_INSPECT) {
        state.inspectTileX = tileX;
        state.inspectTileY = tileY;
        state.editTileSelected = state.map->wallGrid[tileY][tileX];
        if(entityIndex > -1) {
            state.placeSelected = state.map->entity[entityIndex]->type;
        } else {
            state.placeSelected = PLACE_ERASE;
        }
    }
}

void HandleCanvasView(int tileX, int tileY) {
    if((state.toolSelected == TOOL_EDIT) && (state.editTileSelected > 0)) {
        // grow map
        int left=0, right=0, top=0, bottom=0;
        if(tileX < state.tileDataLeft) { left = (state.tileDataLeft - tileX); }
        if(tileX >= (state.tileDataLeft + state.tileDataWidth)) { right = tileX - (state.tileDataLeft + state.tileDataWidth - 1); }
        if(tileY < state.tileDataTop) { top = (state.tileDataTop - tileY); }
        if(tileY >= (state.tileDataTop + state.tileDataHeight)) { bottom = tileY - (state.tileDataTop + state.tileDataHeight - 1); }
        MapExpand(state.map, left, top, right, bottom);
        if(left) { state.tileViewLeft += left; }
        if(top ) { state.tileViewTop  += top;  }
        state.tileDataWidth  = state.map->blockWidth;
        state.tileDataHeight = state.map->blockHeight;
        // set tile
        state.map->wallGrid[tileY+top][tileX+left] = state.editTileSelected;
    } else if(state.toolSelected == TOOL_INSPECT) {
        state.editTileSelected = 0;
        state.placeSelected = PLACE_ERASE;
    }
}

void HandleViewOffset(int left, int right, int up, int down) {
    state.tileViewLeft -= left;
    state.tileViewLeft += right;
    state.tileViewTop  -= up;
    state.tileViewTop  += down;

    int minViewLeft = state.tileDataLeft - (state.tileViewWidth  - 1);
    int minViewTop  = state.tileDataTop  - (state.tileViewHeight - 1);
    int maxViewLeft = state.tileDataLeft + state.tileDataWidth  - 1;
    int maxViewTop  = state.tileDataTop  + state.tileDataHeight - 1;

    if(state.tileViewLeft < minViewLeft) { state.tileViewLeft = minViewLeft; }
    if(state.tileViewLeft > maxViewLeft) { state.tileViewLeft = maxViewLeft; }
    if(state.tileViewTop  < minViewTop ) { state.tileViewTop  = minViewTop;  }
    if(state.tileViewTop  > maxViewTop ) { state.tileViewTop  = maxViewTop;  }
}
void HandleViewOffsetLeft()  { HandleViewOffset(1,0,0,0); }
void HandleViewOffsetRight() { HandleViewOffset(0,1,0,0); }
void HandleViewOffsetUp()    { HandleViewOffset(0,0,1,0); }
void HandleViewOffsetDown()  { HandleViewOffset(0,0,0,1); }

//==================================================================================================================================
static void afterMapOpened() {
    state.tileViewLeft = state.tileViewTop = 0;
    state.tileDataLeft = state.tileDataTop = 0;
    state.tileDataWidth  = state.map->blockWidth;
    state.tileDataHeight = state.map->blockHeight;

    // if data is smaller than view, center view
    if(state.tileDataWidth < state.tileViewWidth) {
        state.tileViewLeft = -((state.tileViewWidth - state.tileDataWidth) >> 1);
    }
    if(state.tileDataHeight < state.tileViewHeight) {
        state.tileViewTop = -((state.tileViewHeight - state.tileDataHeight) >> 1);
    }
}

static void afterMapSaved() {
    // map may contract while saving
    int oldWidth = state.tileDataWidth, oldHeight = state.tileDataHeight;
    state.tileDataWidth  = state.map->blockWidth;
    state.tileDataHeight = state.map->blockHeight;
    if(state.tileDataWidth  < oldWidth ) { state.tileViewLeft -= (oldWidth  - state.tileDataWidth);  }
    if(state.tileDataHeight < oldHeight) { state.tileViewTop  -= (oldHeight - state.tileDataHeight); }
}
