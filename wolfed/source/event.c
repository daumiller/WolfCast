#include "../../shared/map.h"
#include "wolfed.h"

//==================================================================================================================================
void EventMapLoad(const char *path) {
    
}

void EventMapNew() {

}

void EventMapSave(const char *path) {

}

//==================================================================================================================================

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
