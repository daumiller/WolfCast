#include <stdio.h>
#include "wolfed.h"

#define DEFAULT_WINDOW_WIDTH  935
#define DEFAULT_WINDOW_HEIGHT 700
#define WINDOW_TITLE "WolfEd"

State state;

static GLuint *IconSetLoad();
static void IconSetFree(GLuint *set);

int main(int argc, char **argv) {
    glfwSetErrorCallback(HandleGlfwError);
    if(!glfwInit()) {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        return -1;
    }
    state.window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if(!state.window) {
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(state.window);
    ImGui_ImplGlfw_Init(state.window, true);
    glfwSetKeyCallback(state.window, HandleKeys); // override, will chain to imguiGlfw
    glfwSetFramebufferSizeCallback(state.window, HandleResize);
    glfwGetFramebufferSize(state.window, &(state.fbWidth), &(state.fbHeight));
    state.textEditing = false;
    state.ifNotTextEditing = NULL;
    state.windowBgColor = ImColor(114, 144, 154);
    state.toolSelected = TOOL_EDIT;
    state.editTileSelected = 0;
    state.placeSelected = PLACE_ERASE;
    state.inspectTileX = state.inspectTileY = 0;
    state.showDialogOpen = state.showDialogSaveAs = state.showDialogError = false;
    state.mapPath = NULL;
    state.errorMessage = NULL;
    state.map = NULL;
    state.tileDataLeft = state.tileDataTop = state.tileDataWidth = state.tileDataHeight = 0;
    state.tileViewLeft = state.tileViewTop = 0;
    HandleResize(state.window, state.fbWidth, state.fbHeight);

    if(argc > 1) {
        state.map = MapLoad(argv[1]);
        if(!state.map) {
            // TODO: Show GUI Error
            fprintf(stderr, "ERROR: File not found.");
            HandleNew();
        }

    } else {
        HandleNew();
    }

    state.tiles = TextureSetLoad();
    state.icons = IconSetLoad();
    if(!state.tiles || !state.icons) {
        fprintf(stderr, "ERROR: Textures or Icons missing.\n");
        ImGui_ImplGlfw_Shutdown();
        glfwTerminate();
        return -1;
    }

    ImWchar unicodeRange[] = { 0x0020,0x00FF, 0x2300,0x23FF, 0 };
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("font/LucidaGrandeUI.ttf", 15, NULL, unicodeRange);
    ImGui::GetStyle().WindowRounding = 0.0f;

    while(!glfwWindowShouldClose(state.window)) {
        state.ifNotTextEditing = NULL;
        state.textEditing = false;

        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();

        // Canvas
        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(ImVec2(state.width-200, state.height), ImGuiSetCond_Always);
        ImGui::Begin("Canvas", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
        DrawCanvas();
        ImGui::End();

        // SideBar
        ImGui::SetNextWindowPos(ImVec2(state.width-200,0), ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200, state.height), ImGuiSetCond_Always);
        ImGui::Begin("SideBar", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
        DrawSidebarMenu();
        DrawSidebarTools();
        if(state.toolSelected == TOOL_EDIT)     { DrawSidebarEdit();     }
        if(state.toolSelected == TOOL_PLACE)    { DrawSidebarPlace();    }
        if(state.toolSelected == TOOL_INSPECT)  { DrawSidebarInspect();  }
        if(state.toolSelected == TOOL_SETTINGS) { DrawSidebarSettings(); }
        ImGui::End();

        if(state.showDialogOpen)   { DrawDialogOpen();   }
        if(state.showDialogSaveAs) { DrawDialogSaveAs(); }
        if(state.showDialogError)  { DrawDialogError();  }

        glClearColor(state.windowBgColor.x, state.windowBgColor.y, state.windowBgColor.z, state.windowBgColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(state.window);

        if((!state.textEditing) && (state.ifNotTextEditing)) {
            state.ifNotTextEditing();
            state.ifNotTextEditing = NULL;
        }
    }

    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();
    TextureSetFree(state.tiles);
    IconSetFree(state.icons);

    return 0;
}

static GLuint *IconSetLoad() {
    GLuint *handles = (GLuint *)malloc(sizeof(GLuint) * 10);
    if(!TextureLoad("icon/edit.png",     &(handles[TOOL_EDIT    ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/place.png",    &(handles[TOOL_PLACE   ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/inspect.png",  &(handles[TOOL_INSPECT ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/settings.png", &(handles[TOOL_SETTINGS]))) { free(handles); return NULL; }

    if(!TextureLoad("icon/erase.png",  &(handles[PLACE_ERASE ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/door.png",   &(handles[PLACE_DOOR  ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/spawn.png",  &(handles[PLACE_SPAWN ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/key.png",    &(handles[PLACE_KEY   ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/weapon.png", &(handles[PLACE_WEAPON]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/ammo.png",   &(handles[PLACE_AMMO  ]))) { free(handles); return NULL; }
    if(!TextureLoad("icon/enemy.png",  &(handles[PLACE_ENEMY ]))) { free(handles); return NULL; }
    return handles;
}

static void IconSetFree(GLuint *set) {
    free(set);
}
