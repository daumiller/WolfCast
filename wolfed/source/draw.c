#include "library/nuklear_glfw.h"
#include "wolfed.h"

//==================================================================================================================================
#define NUK state.nuklear
#define SIDEBAR_WIDTH 200

static void DrawCanvas(void);
static void DrawSidebar(void);
static void DrawSidebar_Header(void);
static void DrawSidebar_Edit(void);
static void DrawSidebar_Place(void);
static void DrawSidebar_Inspect(void);
static void DrawSidebar_Settings(void);

//==================================================================================================================================
static int width, height, pixelWidth, pixelHeight;
static struct nk_color colorUnselected, colorSelected, colorErase;

//==================================================================================================================================
void DrawInit() {
    colorUnselected = nk_rgb(255,255,255);
    colorSelected   = nk_rgb(  0,200,240);
    colorErase      = nk_rgb(146,  0,  0);
}

//==================================================================================================================================
void DrawUI() {
    nk_glfw_sizes(&width, &height, &pixelWidth, &pixelHeight);

    DrawCanvas();
    DrawSidebar();
}

//==================================================================================================================================
static void DrawCanvas() {
    static struct nk_panel panel;
    static struct nk_rect bounds;
    static struct nk_vec2 spacing;

    bounds = nk_rect(0, 0, width-SIDEBAR_WIDTH, height);
    if(nk_begin(NUK, &panel, "Canvas", bounds, 0)) {
        nk_window_set_bounds(NUK, bounds);
        spacing = NUK->style.window.spacing;
        NUK->style.window.spacing = nk_vec2(0.0f, 0.0f);

        nk_layout_row_dynamic(NUK, 30, 1);
        nk_labelf(NUK, NK_TEXT_ALIGN_LEFT, "Canvas!");

        NUK->style.window.spacing = spacing;
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
static void DrawSidebar_Inspect() {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void DrawSidebar_Settings() {

}


/*
    struct nk_panel layout, colors;
    static int angle, direction = 0;
    static char lineBuff[128]; static int lineLen;

    if(nk_begin(nuklear, &layout, "WolfEd", nk_rect(50, 50, 235, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_static(nuklear, 30, 80, 1);
        if(nk_button_label(nuklear, "Okay", NK_BUTTON_DEFAULT)) { printf("Okay Then...\n"); }

        nk_layout_row_dynamic(nuklear, 30, 2);
        if(nk_option_label(nuklear, "Left",  direction == 0)) { direction = 0; }
        if(nk_option_label(nuklear, "Right", direction == 1)) { direction = 1; }

        nk_layout_row_dynamic(nuklear, 30, 1);
        nk_labelf(nuklear, NK_TEXT_LEFT, "Okay then, %08X", 0xDEADBEEF);

        nk_layout_row_dynamic(nuklear, 30, 1);
        nk_edit_string(nuklear, NK_EDIT_SIMPLE, lineBuff, &lineLen, 127, nk_filter_default);

        nk_layout_row_dynamic(nuklear, 25, 1);
        nk_property_int(nuklear, "Angle: ", 0, &angle, 360, 10, 1);

        nk_layout_row_dynamic(nuklear, 20, 1);
        nk_label(nuklear, "bgcolor: ", NK_TEXT_LEFT);
        nk_layout_row_dynamic(nuklear, 25, 1);
        if(nk_combo_begin_color(nuklear, &colors, bgcolor, 400)) {
            nk_layout_row_dynamic(nuklear, 120, 1);
            bgcolor = nk_color_picker(nuklear, bgcolor, NK_RGBA);
            nk_layout_row_dynamic(nuklear, 25, 1);
            bgcolor.r = (nk_byte)nk_propertyi(nuklear, "R", 0, bgcolor.r, 255, 1, 1);
            bgcolor.g = (nk_byte)nk_propertyi(nuklear, "G", 0, bgcolor.g, 255, 1, 1);
            bgcolor.b = (nk_byte)nk_propertyi(nuklear, "B", 0, bgcolor.b, 255, 1, 1);
            bgcolor.a = (nk_byte)nk_propertyi(nuklear, "A", 0, bgcolor.a, 255, 1, 1);
            nk_combo_end(nuklear);
        }
    }
    nk_end(nuklear);
*/

/*
    struct nk_panel layout;
    struct nk_rect bounds = nk_rect(scaleWidth-192, 0, 192, scaleHeight);
    if(nk_begin(context, &layout, "WolfEditor", bounds, 0)) {
        nk_window_set_position(context, nk_vec2(bounds.x, bounds.y));
        static int valToggle = 1;
        static int valSlide = 20;

        nk_layout_row_dynamic(context, 30, 1);
        nk_labelf(context, NK_TEXT_ALIGN_LEFT, "scaleWidth: %d", scaleWidth);
        nk_labelf(context, NK_TEXT_ALIGN_LEFT, "scaleHeigth: %d", scaleHeight);

        nk_layout_row_static(context, 30, 80, 1);
        if(nk_button_label(context, "OK", NK_BUTTON_DEFAULT)) {
            printf("You pressed the button\n");
        }

        nk_layout_row_dynamic(context, 30, 2);
        if(nk_option_label(context, "Left" , valToggle == 1)) { valToggle = 1; }
        if(nk_option_label(context, "Right", valToggle == 2)) { valToggle = 2; }

        nk_layout_row_dynamic(context, 22, 1);
        nk_property_int(context, "Slidey:", 0, &valSlide, 100, 10, 1);

        struct nk_panel combo;
        nk_layout_row_dynamic(context, 20, 1);
        nk_label(context, "BGColor:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(context, 25, 1);
        if(nk_combo_begin_color(context, &combo, bgcolor, 400)) {
            nk_layout_row_dynamic(context, 120, 1);
            bgcolor = nk_color_picker(context, bgcolor, NK_RGBA);
            nk_layout_row_dynamic(context, 25, 1);
            bgcolor.r = (nk_byte)nk_propertyi(context, "#R:", 0, bgcolor.r, 255, 1, 1);
            bgcolor.g = (nk_byte)nk_propertyi(context, "#G:", 0, bgcolor.g, 255, 1, 1);
            bgcolor.b = (nk_byte)nk_propertyi(context, "#B:", 0, bgcolor.b, 255, 1, 1);
            bgcolor.a = (nk_byte)nk_propertyi(context, "#A:", 0, bgcolor.a, 255, 1, 1);
            nk_combo_end(context);
        }

        nk_layout_row_static(context, 48, 48, 3);
        nk_button_image(context, texture1, NK_BUTTON_DEFAULT);
        nk_button_image(context, texture2, NK_BUTTON_DEFAULT);
        nk_button_image(context, texture3, NK_BUTTON_DEFAULT);

        nk_end(context);
    }
*/

//==================================================================================================================================
