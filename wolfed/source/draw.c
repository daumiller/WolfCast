#include <stdlib.h>
#include "library/nuklear_glfw.h"
#include "wolfed.h"
#include "../../shared/stdint.h"

//==================================================================================================================================
#define NUK state.nuklear

#define WALL_TYPE(y,x)     state.map->wallGrid[y][x]
#define ENTITY_INDEX(y,x)  state.map->entityGrid[y][x]
#define ENTITY_OBJECT(y,x) state.map->entity[ENTITY_INDEX(y, x) - 1]
#define ENTITY_TYPE(y,x)   ENTITY_OBJECT(y,x)->type

static void DrawCanvas(void);
static void DrawSidebar(void);
static void DrawSidebar_Header(void);
static void DrawSidebar_Edit(void);
static void DrawSidebar_Place(void);
static void DrawSidebar_Inspect(void);
static void DrawSidebar_Settings(void);
static void DrawErrorDialog(void);
static void DrawPromptDialog(void);

//==================================================================================================================================
static int width, height, pixelWidth, pixelHeight;
static struct nk_color colorUnselected, colorSelected, colorErase;

//==================================================================================================================================
void DrawInit() {
    colorUnselected = nk_rgb(255,255,255);
    colorSelected   = nk_rgb(  0,200,240);
    colorErase      = nk_rgb(146,  0,  0);

    glfwGetWindowSize(state.window, &width, &height);
    EventResized(state.window, width, height);
}

//==================================================================================================================================
void DrawUI() {
    state.isTextEditing = false;
    nk_glfw_sizes(&width, &height, &pixelWidth, &pixelHeight);

    // shittiest modal method ever...
    // (nk_begin_popup only works from within a window, and only blocks input to that one, single, window)
    if(state.errorMessage) {
        DrawErrorDialog();
    } else if(state.prompt.showing) {
        DrawPromptDialog();
    } else {
        DrawCanvas();
        DrawSidebar();
    }
}

//==================================================================================================================================
#define BTN_CANVAS_WALL(x)  nk_button_image(NUK, nk_image_id(state.textures[x]), NK_BUTTON_DEFAULT)
#define BTN_CANVAS_PLACE(x) nk_button_image(NUK, nk_image_id(state.icons[x]), NK_BUTTON_DEFAULT)

static void DrawCanvas() {
    static struct nk_panel panel;
    static struct nk_rect bounds;
    static struct nk_vec2 windowSpacing, buttonPadding;

    bounds = nk_rect(0, 0, width-SIDEBAR_WIDTH, height);
    if(nk_begin(NUK, &panel, "Canvas", bounds, 0)) {
        nk_window_set_bounds(NUK, bounds);

        windowSpacing = NUK->style.window.spacing;
        buttonPadding = NUK->style.button.padding;
        NUK->style.window.spacing = nk_vec2(0.0f, 0.0f);
        NUK->style.button.padding = nk_vec2(1.0f, 1.0f);

        nk_layout_row_static(NUK, CANVAS_BLOCK_SIZE-2, CANVAS_BLOCK_SIZE-2, state.viewWidth);

        int maxDataX = state.dataLeft + state.dataWidth - 1;
        int maxDataY = state.dataTop + state.dataHeight - 1;

        int dataX, dataY, viewX, viewY; uint16 entityType;
        for(viewY=0; viewY < state.viewHeight; viewY++) {
            dataY = state.viewTop + viewY;
            for(viewX=0; viewX < state.viewWidth; viewX++) {
                dataX = state.viewLeft + viewX;

                if((state.dataTop <= dataY) && (dataY <= maxDataY) && (state.dataLeft <= dataX) && (dataX <= maxDataX)) {
                    if(((state.selectedTool == TOOL_PLACE) || (state.selectedTool == TOOL_INSPECT)) && (ENTITY_INDEX(dataY, dataX) > 0)) {
                        entityType = ENTITY_TYPE(dataY, dataX);
                        if(BTN_CANVAS_PLACE(entityType)) { EventCanvasData(dataX, dataY); }
                    } else {
                        if(BTN_CANVAS_WALL(WALL_TYPE(dataY,dataX))) { EventCanvasData(dataX, dataY); }
                    }
                } else {
                    if(BTN_CANVAS_WALL(0)) { EventCanvasView(dataX, dataY); }
                }
            }
        }

        NUK->style.window.spacing = windowSpacing;
        NUK->style.button.padding = buttonPadding;
    }
    nk_end(NUK);
}

//==================================================================================================================================
static void DrawSidebar() {
    static struct nk_panel panel;
    static struct nk_rect bounds;

    bounds = nk_rect(width-SIDEBAR_WIDTH, 0, SIDEBAR_WIDTH, height);
    if(nk_begin(NUK, &panel, "Sidebar", bounds, 0)) {
        nk_window_set_bounds(NUK, bounds);

        DrawSidebar_Header();

        switch(state.selectedTool) {
            case TOOL_EDIT:     DrawSidebar_Edit();     break;
            case TOOL_PLACE:    DrawSidebar_Place();    break;
            case TOOL_INSPECT:  DrawSidebar_Inspect();  break;
            case TOOL_SETTINGS: DrawSidebar_Settings(); break;
        }
    }
    nk_end(NUK);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define BTN_SIDE_HEAD(x) nk_button_image_colored(NUK, nk_image_id(state.icons[x]), NK_BUTTON_DEFAULT, (state.selectedTool == x) ? colorSelected : colorUnselected)

static void DrawSidebar_Header() {
    nk_layout_row_static(NUK, 40, 40, 4);
    if(BTN_SIDE_HEAD(TOOL_EDIT    )) { state.selectedTool = TOOL_EDIT;     }
    if(BTN_SIDE_HEAD(TOOL_PLACE   )) { state.selectedTool = TOOL_PLACE;    }
    if(BTN_SIDE_HEAD(TOOL_INSPECT )) { state.selectedTool = TOOL_INSPECT;  }
    if(BTN_SIDE_HEAD(TOOL_SETTINGS)) { state.selectedTool = TOOL_SETTINGS; }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define BTN_SIDE_EDIT(x) nk_button_image(NUK, nk_image_id(state.textures[x]), NK_BUTTON_DEFAULT)

static void DrawSidebar_Edit() {
    nk_layout_row_static(NUK, 52, 52, 3);
    for(int index=0; index<65; index++) {
        if(BTN_SIDE_EDIT(index)) { state.selectedEdit = index; }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define BTN_SIDE_PLACE(x) nk_button_image(NUK, nk_image_id(state.icons[x]), NK_BUTTON_DEFAULT)
#define BTN_SIDE_ERASE(x) nk_button_image_colored(NUK, nk_image_id(state.icons[x]), NK_BUTTON_DEFAULT, colorErase)

static void DrawSidebar_Place() {
    nk_layout_row_static(NUK, 52, 52, 3);
    if(BTN_SIDE_PLACE(PLACE_DOOR   )) { state.selectedPlace = PLACE_DOOR;   }
    if(BTN_SIDE_PLACE(PLACE_SPAWN  )) { state.selectedPlace = PLACE_SPAWN;  }
    if(BTN_SIDE_PLACE(PLACE_KEY    )) { state.selectedPlace = PLACE_KEY;    }
    if(BTN_SIDE_PLACE(PLACE_WEAPON )) { state.selectedPlace = PLACE_WEAPON; }
    if(BTN_SIDE_PLACE(PLACE_AMMO   )) { state.selectedPlace = PLACE_AMMO;   }
    if(BTN_SIDE_PLACE(PLACE_ENEMY  )) { state.selectedPlace = PLACE_ENEMY;  }
    if(BTN_SIDE_ERASE(PLACE_ERASE  )) { state.selectedPlace = PLACE_ERASE;  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static const char *EntityDoorSecurity[] = { "None", "Red", "Yellow", "Blue" };
static void DrawSidebar_Inspect() {
    uint8 wallType = WALL_TYPE(state.selectedInspectY, state.selectedInspectX);

    uint8 entityType = 0;
    const char *entityName = "";
    if(ENTITY_INDEX(state.selectedInspectY, state.selectedInspectX) > 0) {
        entityType = ENTITY_TYPE(state.selectedInspectY, state.selectedInspectX);
        switch(entityType) {
            case ENTITY_TYPE_DOOR   : entityName = "Door";       break;
            case ENTITY_TYPE_SPAWN  : entityName = "Spawn";      break;
            case ENTITY_TYPE_KEY    : entityName = "Key";        break;
            case ENTITY_TYPE_WEAPON : entityName = "Weapon";     break;
            case ENTITY_TYPE_AMMO   : entityName = "Ammunition"; break;
            case ENTITY_TYPE_ENEMY  : entityName = "Enemy";      break;
        }
    }

    nk_layout_row_dynamic(NUK, 16.0f, 1);
    nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);
    nk_labelf(NUK, NK_TEXT_ALIGN_CENTERED, "Block: (%d, %d)", state.selectedInspectX, state.selectedInspectY);
    nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);

    float colSize = (float)((SIDEBAR_WIDTH - 20) >> 1);
    nk_layout_row_static(NUK, 14.0f, colSize, 2);
    nk_labelf(NUK, NK_TEXT_ALIGN_CENTERED, "Wall %02d", wallType);
    nk_labelf(NUK, NK_TEXT_ALIGN_CENTERED, "%s", entityName);

    nk_layout_row_static(NUK, colSize, colSize, 2);
    nk_button_image(NUK, nk_image_id(state.textures[wallType]), NK_BUTTON_DEFAULT);
    if(ENTITY_INDEX(state.selectedInspectY, state.selectedInspectX) > 0) {
        nk_button_image(NUK, nk_image_id(state.icons[entityType]), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(NUK, 16.0f, 1);
        nk_label(NUK, "", NK_TEXT_ALIGN_LEFT);

        if((entityType == ENTITY_TYPE_SPAWN) || (entityType == ENTITY_TYPE_ENEMY)) {
            int facing = (int)ENTITY_OBJECT(state.selectedInspectY, state.selectedInspectX)->facing;

            nk_layout_row_dynamic(NUK, 12.0f, 1);
            nk_labelf(NUK, NK_TEXT_ALIGN_CENTERED, "Facing %d°", facing);
            
            nk_layout_row_dynamic(NUK, 24.0f, 1);
            facing = nk_propertyi(NUK, "", 0, facing, 359, 5, 5);
            ENTITY_OBJECT(state.selectedInspectY, state.selectedInspectX)->facing = (double)facing;

        } else if(entityType == ENTITY_TYPE_DOOR) {
            int security = (int)ENTITY_OBJECT(state.selectedInspectY, state.selectedInspectX)->flags;

            nk_layout_row_dynamic(NUK, 12.0f, 1);
            nk_labelf(NUK, NK_TEXT_ALIGN_CENTERED, "Security");

            nk_layout_row_dynamic(NUK, 24.0f, 1);
            security = nk_combo(NUK, EntityDoorSecurity, 4, security, 16);
            ENTITY_OBJECT(state.selectedInspectY, state.selectedInspectX)->flags = (uint32)security;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void DrawSidebar_Settings() {
    char idBuffer  [256]; strncpy(idBuffer,   state.map->id  , 256); int idLength   = strlen(idBuffer);
    char nameBuffer[256]; strncpy(nameBuffer, state.map->name, 256); int nameLength = strlen(nameBuffer);

    nk_layout_row_dynamic(NUK, 16.0f, 1);
    nk_label(NUK, "", NK_TEXT_ALIGN_LEFT);
    nk_label(NUK, "Map ID", NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(NUK, 24.0f, 1);
    if(nk_edit_string(NUK, NK_EDIT_SIMPLE | NK_EDIT_AUTO_SELECT, idBuffer, &idLength, 255, nk_filter_default) & NK_EDIT_ACTIVE) {
        state.isTextEditing = true;
    }
    idBuffer[idLength] = 0x00;

    nk_layout_row_dynamic(NUK, 16.0f, 1);
    nk_label(NUK, "", NK_TEXT_ALIGN_LEFT);
    nk_label(NUK, "Map Name", NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(NUK, 24.0f, 1);
    if(nk_edit_string(NUK, NK_EDIT_SIMPLE | NK_EDIT_AUTO_SELECT, nameBuffer, &nameLength, 255, nk_filter_default) & NK_EDIT_ACTIVE) {
        state.isTextEditing = true;
    }
    nameBuffer[nameLength] = 0x00;

    if(strcmp(idBuffer, state.map->id) != 0) {
        if(state.map->id) { free(state.map->id); }
        state.map->id = strdup(idBuffer);
    }

    if(strcmp(nameBuffer, state.map->name) != 0) {
        if(state.map->name) { free(state.map->name); }
        state.map->name = strdup(nameBuffer);
    }
}

//==================================================================================================================================
static void DrawErrorDialog() {
    static struct nk_panel panel;
    static struct nk_rect bounds;

    bounds = nk_rect((width>>1)-160, (height>>1)-64, 320,128);
    if(nk_begin(NUK, &panel, "ERROR", bounds, NK_WINDOW_TITLE | NK_WINDOW_BORDER_HEADER | NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE)) {
        nk_window_set_bounds(NUK, bounds);

        nk_layout_row_dynamic(NUK, 16.0f, 1);
        nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);
        nk_label(NUK, state.errorMessage, NK_TEXT_ALIGN_CENTERED);
        nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);
    } else {
        free(state.errorMessage);
        state.errorMessage = NULL;
    }
    nk_end(NUK);
}

//==================================================================================================================================
static void DrawPromptDialog() {
    static struct nk_panel panel;
    static struct nk_rect bounds;

    bounds = nk_rect((width>>1)-160, (height>>1)-90, 320,180);
    if(nk_begin(NUK, &panel, state.prompt.title, bounds, NK_WINDOW_TITLE | NK_WINDOW_BORDER_HEADER | NK_WINDOW_BORDER)) {
        nk_window_set_bounds(NUK, bounds);

        nk_layout_row_dynamic(NUK, 14.0f, 1);
        nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);
        nk_label(NUK, state.prompt.message, NK_TEXT_ALIGN_CENTERED);

        int promptLength = strlen(state.prompt.input);
        nk_layout_row_dynamic(NUK, 24.0f, 1);
        int status = nk_edit_string(NUK, NK_EDIT_SIMPLE | NK_EDIT_AUTO_SELECT | NK_EDIT_SIG_ENTER, state.prompt.input, &promptLength, 127, nk_filter_default);

        state.prompt.input[promptLength] = 0x00;
        if(status & NK_EDIT_ACTIVE) { state.isTextEditing = true; }
        if(status & NK_EDIT_COMMITED) { state.prompt.callback(true); }

        nk_layout_row_dynamic(NUK, 12.0f, 1);
        nk_label(NUK, "", NK_TEXT_ALIGN_CENTERED);

        nk_layout_row_dynamic(NUK, 24.0f, 2);
        if(nk_button_label(NUK, "Okay", NK_BUTTON_DEFAULT)) {
            state.prompt.callback(true);
        }
        if(nk_button_label(NUK, "Cancel", NK_BUTTON_DEFAULT)) {
            state.prompt.callback(false);
        }
    }
    nk_end(NUK);
}

//==================================================================================================================================
