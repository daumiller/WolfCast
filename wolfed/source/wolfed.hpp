extern "C" {
    #include "texture.h"
    #include "map.h"
}
#include "imgui/imguiGlfw.h"

//==================================================================================================================================
#define TOOL_EDIT     0
#define TOOL_PLACE    1
#define TOOL_INSPECT  2
#define TOOL_SETTINGS 3

#define PLACE_ERASE   4
#define PLACE_DOOR    5
#define PLACE_SPAWN   6
#define PLACE_KEY     7
#define PLACE_WEAPON  8
#define PLACE_AMMO    9
#define PLACE_ENEMY  10

//==================================================================================================================================
typedef struct {
    GLFWwindow *window;
    ImVec4 windowBgColor;
    int width, height;
    int fbWidth, fbHeight;
    bool textEditing;
    void (*ifNotTextEditing)();
    const char *errorMessage;
    char *mapPath;
    bool showDialogOpen, showDialogSaveAs, showDialogError;

    TextureSet *tiles;
    GLuint *icons;

    int toolSelected;
    int editTileSelected;
    int placeSelected;
    int inspectTileX, inspectTileY;

    int tileDataLeft, tileDataTop, tileDataWidth, tileDataHeight;
    int tileViewLeft, tileViewTop, tileViewWidth, tileViewHeight;
    Map *map;
} State;

//==================================================================================================================================
void HandleGlfwError(int code, const char *desc);
void HandleResize(GLFWwindow *window, int fbWidth, int fbHeight);
void HandleKeys(GLFWwindow* window, int key, int scancode, int action, int mods);
void HandleOpen();
void HandleOpenFile(const char *path);
void HandleNew();
void HandleSave();
void HandleSaveAs();
void HandleSaveAsFile(const char *path);
void HandleExit();
void HandleToolEdit();
void HandleToolPlace();
void HandleToolInspect();
void HandleToolSettings();
void HandleEditSelect(uint8 tile);
void HandlePlaceSelect(uint8 place);
void HandleCanvasData(int tileX, int tileY);
void HandleCanvasView(int tileX, int tileY);
void HandleViewOffset(int left, int right, int up, int down);
void HandleViewOffsetLeft();
void HandleViewOffsetRight();
void HandleViewOffsetUp();
void HandleViewOffsetDown();

//==================================================================================================================================
void DrawCanvas();
void DrawSidebarMenu();
void DrawSidebarStatus();
void DrawSidebarTools();
void DrawSidebarEdit();
void DrawSidebarPlace();
void DrawSidebarInspect();
void DrawSidebarSettings();
void DrawDialogOpen();
void DrawDialogSaveAs();
void DrawDialogError();
