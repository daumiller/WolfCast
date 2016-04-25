#include "wolfed.h"
#include <stdio.h>
#include <string.h>
extern State state;

static ImColor colorToolSelected = ImColor(0,200,240);
static ImColor colorPlaceErase = ImColor(146,0,0);
static ImColor colorClear = ImColor(0,0,0,0);
static ImColor colorWhite = ImColor(255,255,255);

#define CANVAS_WALL(x)  ImGui::ImageButton((void *)(long)state.tiles[x], ImVec2(32, 32), ImVec2(0,0), ImVec2(1,1), 1)
#define CANVAS_PLACE(x) ImGui::ImageButton((void *)(long)state.icons[x], ImVec2(32, 32), ImVec2(0,0), ImVec2(1,1), 1)
void DrawCanvas() {
    ImVec2 oldSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::GetStyle().ItemSpacing = ImVec2(0,0);

    int sameLineX = state.tileViewWidth - 1;
    int maxDataX = state.tileDataLeft + state.tileDataWidth - 1;
    int maxDataY = state.tileDataTop + state.tileDataHeight - 1;

    int dataPosX, dataPosY;
    for(int tileViewY=0; tileViewY<state.tileViewHeight; tileViewY++) {
        dataPosY = state.tileViewTop + tileViewY;
        for(int tileViewX=0; tileViewX<state.tileViewWidth; tileViewX++) {
            dataPosX = state.tileViewLeft + tileViewX;

            ImGui::PushID((tileViewY * state.tileViewWidth) + tileViewX);
            // inside/outside current map data?
            if((dataPosX >= state.tileDataLeft) && (dataPosX <= maxDataX) && (dataPosY >= state.tileDataTop) && (dataPosY <= maxDataY)) {
                if(((state.toolSelected == TOOL_PLACE) || (state.toolSelected == TOOL_INSPECT)) && (state.map->entityGrid[dataPosY][dataPosX] > 0)) {
                    int entityType = state.map->entity[state.map->entityGrid[dataPosY][dataPosX] - 1]->type;
                    if(CANVAS_PLACE(entityType)) { HandleCanvasData(dataPosX, dataPosY); }
                } else {
                    if(CANVAS_WALL(state.map->wallGrid[dataPosY][dataPosX])) { HandleCanvasData(dataPosX, dataPosY); }
                }
            } else {
                if(CANVAS_WALL(0)) { HandleCanvasView(dataPosX, dataPosY); }
            }
            ImGui::PopID();

            if(tileViewX < sameLineX) { ImGui::SameLine(); }
        }
    }

    ImGui::GetStyle().ItemSpacing = oldSpacing;
}

void DrawSidebarMenu() {
    if(ImGui::BeginMenuBar()) {
        if(ImGui::BeginMenu("File", true)) {
            if(ImGui::MenuItem("Open"   , "⌘O")) { HandleOpen();   }
            if(ImGui::MenuItem("Save"   , "⌘S")) { HandleSave();   }
            if(ImGui::MenuItem("Save As", "⌘A")) { HandleSaveAs(); }
            if(ImGui::MenuItem("New"    , "⌘N")) { HandleNew();    }
            if(ImGui::MenuItem("Exit"   , "⌘Q")) { HandleExit();   }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

#define BUTTON_TOOL(x) ImGui::ImageButton((void *)(long)state.icons[x], \
    ImVec2(32, 32), ImVec2(0,0), ImVec2(1,1), 4, \
    colorClear, (state.toolSelected == x) ? colorToolSelected : colorWhite)
void DrawSidebarTools() {
    if(BUTTON_TOOL(TOOL_EDIT    )) { HandleToolEdit();     } ImGui::SameLine();
    if(BUTTON_TOOL(TOOL_PLACE   )) { HandleToolPlace();    } ImGui::SameLine();
    if(BUTTON_TOOL(TOOL_INSPECT )) { HandleToolInspect();  } ImGui::SameLine();
    if(BUTTON_TOOL(TOOL_SETTINGS)) { HandleToolSettings(); }
}

#define BUTTON_EDIT(x) ImGui::ImageButton((void *)(long)state.tiles[x], ImVec2(42, 42))
void DrawSidebarEdit() {
    ImGui::BeginChild("siderbar-edit");
    for(int idx=0; idx<65; idx++) {
        if(BUTTON_EDIT(idx)) { HandleEditSelect(idx); }
        if((idx + 1) % 3) { ImGui::SameLine(); }
    }
    ImGui::EndChild();
}

#define BUTTON_PLACE(x) ImGui::ImageButton((void *)(long)state.icons[x], ImVec2(48, 48))
#define BUTTON_ERASE(x) ImGui::ImageButton((void *)(long)state.icons[x], ImVec2(48, 48), ImVec2(0,0), ImVec2(1,1), 4, colorClear, colorPlaceErase)
void DrawSidebarPlace() {
    if(BUTTON_PLACE(PLACE_DOOR))   { HandlePlaceSelect(PLACE_DOOR);   } ImGui::SameLine();
    if(BUTTON_PLACE(PLACE_SPAWN))  { HandlePlaceSelect(PLACE_SPAWN);  } ImGui::SameLine();
    if(BUTTON_PLACE(PLACE_KEY))    { HandlePlaceSelect(PLACE_KEY);    }
    if(BUTTON_PLACE(PLACE_WEAPON)) { HandlePlaceSelect(PLACE_WEAPON); } ImGui::SameLine();
    if(BUTTON_PLACE(PLACE_AMMO))   { HandlePlaceSelect(PLACE_AMMO);   } ImGui::SameLine();
    if(BUTTON_PLACE(PLACE_ENEMY))  { HandlePlaceSelect(PLACE_ENEMY);  }
    if(BUTTON_ERASE(PLACE_ERASE))  { HandlePlaceSelect(PLACE_ERASE);  }
}

void DrawSidebarInspect() {
    uint8 wall = state.map->wallGrid[state.inspectTileY][state.inspectTileX];
    int entityIndex = ((int)state.map->entityGrid[state.inspectTileY][state.inspectTileX]) - 1;
    const char *entityName = "";
    uint8 entityType = 0;
    if(entityIndex >= 0) {
        entityType = state.map->entity[entityIndex]->type;
        switch(entityType) {
            case ENTITY_TYPE_DOOR   : entityName = "Door";       break;
            case ENTITY_TYPE_SPAWN  : entityName = "Spawn";      break;
            case ENTITY_TYPE_KEY    : entityName = "Key";        break;
            case ENTITY_TYPE_WEAPON : entityName = "Weapon";     break;
            case ENTITY_TYPE_AMMO   : entityName = "Ammunition"; break;
            case ENTITY_TYPE_ENEMY  : entityName = "Enemy";      break;
        }
    }

    ImGui::Text("");
    ImGui::Text("Tile: (%d, %d)", state.inspectTileX, state.inspectTileY);
    ImGui::Text("");

    ImGui::Columns(2, "Details", false);
    ImGui::Text("Wall %02d", wall);
    ImGui::ImageButton((void *)(long)state.tiles[wall], ImVec2(64, 64), ImVec2(0,0), ImVec2(1,1), 0);
    ImGui::NextColumn();
    ImGui::Text("%s", entityName);
    if(entityType > 0) {
        ImGui::ImageButton((void *)(long)state.icons[entityType], ImVec2(64, 64), ImVec2(0,0), ImVec2(1,1), 0);

        ImGui::Columns(1);
        ImGui::Text("");
        if((entityType == ENTITY_TYPE_SPAWN) || (entityType == ENTITY_TYPE_ENEMY)) {
            ImGui::Text("Facing");
            int facing = (int)state.map->entity[entityIndex]->facing;
            if(ImGui::SliderInt("", &facing, 0, 359, "%.f°")) {
                state.map->entity[entityIndex]->facing = (double)facing;
            }
        } else if(entityType == ENTITY_TYPE_DOOR) {
            ImGui::Text("Security");
            int security = state.map->entity[entityIndex]->flags;
            if(ImGui::Combo("", &security, "None\0Red\0Yellow\0Blue\0\0", 4)) {
                state.map->entity[entityIndex]->flags = security;
            }
        }
    }
}

void DrawSidebarSettings() {
    char buffId[256], buffName[256];
    sprintf(buffId, "%s", state.map->id);
    sprintf(buffName, "%s", state.map->name);

    ImGui::Text("");
    ImGui::Text("Map ID:");
    ImGui::PushID(10);
    if(ImGui::InputText("", buffId, 255)) {
        free(state.map->id);
        state.map->id = strdup(buffId);
    }
    if(ImGui::IsItemActive()) { state.textEditing = true; }
    ImGui::PopID();

    ImGui::Text("");
    ImGui::Text("Map Name:");
    ImGui::PushID(11);
    if(ImGui::InputText("", buffName, 255)) {
        free(state.map->name);
        state.map->name = strdup(buffName);
    }
    if(ImGui::IsItemActive()) { state.textEditing = true; }
    ImGui::PopID();
}

void DrawDialogs() {
}

void DrawDialogOpen() {
    static char *buff = NULL;
    bool justOpened = (buff == NULL);
    if(justOpened) { buff = (char *)calloc(256, 1); }

    ImGui::OpenPopup("Open Map");
    if(!ImGui::BeginPopupModal("Open Map")) { return; }
    ImGui::Text("Map name (without path or extension):");
    ImGui::PushID(12);
    if(ImGui::GetWindowIsFocused() && !ImGui::IsAnyItemActive()) { ImGui::SetKeyboardFocusHere(); }
    if(ImGui::InputText("", buff, 255, ImGuiInputTextFlags_EnterReturnsTrue)) {
        ImGui::CloseCurrentPopup();
        state.showDialogOpen = false;
        HandleOpenFile(buff);
        free(buff); buff = NULL;
    }
    if(ImGui::IsItemActive()) { state.textEditing = true; }
    if(ImGui::Button("Open")) {
        ImGui::CloseCurrentPopup();
        state.showDialogOpen = false;
        HandleOpenFile(buff);
        free(buff); buff = NULL;
    }
    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
        state.showDialogOpen = false;
        free(buff); buff = NULL;
    }
    ImGui::PopID();
    ImGui::EndPopup();
}

void DrawDialogSaveAs() {
    static char *buff = NULL;
    bool justOpened = (buff == NULL);
    if(justOpened) { buff = (char *)calloc(256, 1); }

    ImGui::OpenPopup("Save Map");
    if(!ImGui::BeginPopupModal("Save Map")) { return; }
    ImGui::Text("Map name (without path or extension):");
    ImGui::PushID(13);
    if(ImGui::GetWindowIsFocused() && !ImGui::IsAnyItemActive()) { ImGui::SetKeyboardFocusHere(); }
    if(ImGui::InputText("", buff, 255, ImGuiInputTextFlags_EnterReturnsTrue)) {
        ImGui::CloseCurrentPopup();
        state.showDialogSaveAs = false;
        HandleSaveAsFile(buff);
        free(buff); buff = NULL;
    }
    if(ImGui::IsItemActive()) { state.textEditing = true; }
    if(ImGui::Button("Save")) {
        ImGui::CloseCurrentPopup();
        state.showDialogSaveAs = false;
        HandleSaveAsFile(buff);
        free(buff); buff = NULL;
    }
    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
        state.showDialogSaveAs = false;
        free(buff); buff = NULL;
    }
    ImGui::PopID();
    ImGui::EndPopup();
}

void DrawDialogError() {
    ImGui::OpenPopup("ERROR");
    if(!ImGui::BeginPopupModal("ERROR")) { return; }
    ImGui::Text("%s", state.errorMessage);
    if(ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
        state.showDialogError = false;
    }
    ImGui::EndPopup();
}
