#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "nuklear_glfw.h"

//==================================================================================================================================
typedef struct {
    struct nk_buffer commands;
    struct nk_draw_null_texture textureNull;
    GLuint textureFont;
    struct nk_draw_vertex *vertices;
    nk_draw_index *elements;
    enum nk_anti_aliasing aliasing;

    GLFWwindow *window;
    struct nk_context context;
    struct nk_font_atlas fonts;
    uint32_t *textBuffer;
    uint32_t textLength;
    float scroll;
    int width, height;
    int pixelWidth, pixelHeight;
} nk_glfw_context;

static nk_glfw_context nk_glfw;

static void nk_glfw_clipboard_copy(nk_handle user, const char *text, int length);
static void nk_glfw_clipboard_paste(nk_handle user, struct nk_text_edit *edit);

//==================================================================================================================================
struct nk_context *nk_glfw_init(GLFWwindow *window, bool antiAlias, bool installCallbacks) {
    nk_glfw.window = window;
    glfwGetWindowSize(nk_glfw.window, &nk_glfw.width, &nk_glfw.height);
    glfwGetFramebufferSize(nk_glfw.window, &nk_glfw.pixelWidth, &nk_glfw.pixelHeight);
    glViewport(0, 0, nk_glfw.pixelWidth, nk_glfw.pixelHeight);

    nk_glfw.aliasing = antiAlias ? NK_ANTI_ALIASING_ON : NK_ANTI_ALIASING_OFF;
    if(installCallbacks) {
        glfwSetKeyCallback(window, nk_glfw_callback_key);
        glfwSetCharCallback(window, nk_glfw_callback_char);
        glfwSetScrollCallback(window, nk_glfw_callback_scroll);
        glfwSetFramebufferSizeCallback(window, nk_glfw_callback_resize);
    }

    nk_init_default(&nk_glfw.context, 0);
    nk_glfw.context.clip.copy     = nk_glfw_clipboard_copy;
    nk_glfw.context.clip.paste    = nk_glfw_clipboard_paste;
    nk_glfw.context.clip.userdata = nk_handle_ptr(0);

    nk_buffer_init_default(&nk_glfw.commands);
    nk_glfw.vertices   = malloc(SIZE_BUFFER_VERTEX);
    nk_glfw.elements   = malloc(SIZE_BUFFER_ELEMENT);
    nk_glfw.textBuffer = malloc(SIZE_BUFFER_TEXT);
    nk_glfw.textLength = 0;

    return &nk_glfw.context;
}

void nk_glfw_shutdown(void) {
    nk_font_atlas_clear(&nk_glfw.fonts);
    nk_buffer_free(&nk_glfw.commands);
    free(nk_glfw.vertices);
    free(nk_glfw.elements);
    free(nk_glfw.textBuffer);
    nk_free(&nk_glfw.context);
}

void nk_glfw_sizes(int *width, int *height, int *pixelWidth, int *pixelHeight) {
    if(width)  { *width  = nk_glfw.width;  }
    if(height) { *height = nk_glfw.height; }
    if(pixelWidth)  { *pixelWidth  = nk_glfw.pixelWidth;  }
    if(pixelHeight) { *pixelHeight = nk_glfw.pixelHeight; }
}

//==================================================================================================================================
void nk_glfw_font_begin(struct nk_font_atlas **atlas) {
    nk_font_atlas_init_default(&nk_glfw.fonts);
    nk_font_atlas_begin(&nk_glfw.fonts);
    if(atlas) { *atlas = &nk_glfw.fonts; }
}

void nk_glfw_font_end(void) {
    int width, height;
    const void *image = nk_font_atlas_bake(&nk_glfw.fonts, &width, &height, NK_FONT_ATLAS_RGBA32);

    glGenTextures(1, &nk_glfw.textureFont);
    glBindTexture(GL_TEXTURE_2D, nk_glfw.textureFont);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    nk_font_atlas_end(&nk_glfw.fonts, nk_handle_id((int)nk_glfw.textureFont), &nk_glfw.textureNull);
    if(nk_glfw.fonts.default_font) {
        nk_style_set_font(&nk_glfw.context, &nk_glfw.fonts.default_font->handle);
    }
}

//==================================================================================================================================
void nk_glfw_callback_char(GLFWwindow *window, unsigned int codepoint) {
    if(nk_glfw.textLength < SIZE_BUFFER_TEXT) {
        nk_glfw.textBuffer[nk_glfw.textLength] = codepoint;
        nk_glfw.textLength++;
    }
}

void nk_glfw_callback_scroll(GLFWwindow *window, double xOffset, double yOffset) {
    nk_glfw.scroll += yOffset;
}

void nk_glfw_callback_resize(GLFWwindow *window, int pixelWidth, int pixelHeight) {
    nk_glfw.pixelWidth = pixelWidth;
    nk_glfw.pixelHeight = pixelHeight;
    glfwGetWindowSize(nk_glfw.window, &nk_glfw.width, &nk_glfw.height);
    glViewport(0, 0, nk_glfw.pixelWidth, nk_glfw.pixelHeight);
}

void nk_glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
}

//==================================================================================================================================
static void nk_glfw_clipboard_copy(nk_handle user, const char *text, int length) {
    if(!length) { return; }
    char *buffer = malloc(length+1);
    memcpy(buffer, text, length);
    buffer[length] = 0x00;
    glfwSetClipboardString(nk_glfw.window, buffer);
    free(buffer);
}

static void nk_glfw_clipboard_paste(nk_handle user, struct nk_text_edit *edit) {
    const char *text = glfwGetClipboardString(nk_glfw.window);
    if(text) { nk_textedit_paste(edit, text, nk_strlen(text)); }
}

//==================================================================================================================================
#define NK_KEY(x, y) nk_input_key(&nk_glfw.context, NK_KEY_##x, y)
#define NK_BTN(x, y) nk_input_button(&nk_glfw.context, NK_BUTTON_##x, mouseXi, mouseYi, y)
#define GLFW_KEY(x) (glfwGetKey(nk_glfw.window, GLFW_KEY_##x) == GLFW_PRESS)
#define GLFW_BTN(x) (glfwGetMouseButton(nk_glfw.window, GLFW_MOUSE_BUTTON_##x) == GLFW_PRESS)

void nk_glfw_input(void) {
    nk_input_begin(&nk_glfw.context);
    for(uint32_t idx=0; idx<nk_glfw.textLength; idx++) { nk_input_unicode(&nk_glfw.context, nk_glfw.textBuffer[idx]); }

    NK_KEY(DEL            , GLFW_KEY(DELETE   ));
    NK_KEY(ENTER          , GLFW_KEY(ENTER    ));
    NK_KEY(TAB            , GLFW_KEY(TAB      ));
    NK_KEY(BACKSPACE      , GLFW_KEY(BACKSPACE));
    NK_KEY(TEXT_LINE_START, GLFW_KEY(HOME     ));
    NK_KEY(TEXT_LINE_END  , GLFW_KEY(END      ));
    NK_KEY(SHIFT          , GLFW_KEY(LEFT_SHIFT) || GLFW_KEY(RIGHT_SHIFT));

    if(GLFW_KEY(LEFT_SUPER) || GLFW_KEY(RIGHT_SUPER)) {
        NK_KEY(CUT            , GLFW_KEY(X    ));
        NK_KEY(COPY           , GLFW_KEY(C    ));
        NK_KEY(PASTE          , GLFW_KEY(V    ));
        NK_KEY(TEXT_UNDO      , GLFW_KEY(Z    ));
        NK_KEY(TEXT_REDO      , GLFW_KEY(Y    ));
        NK_KEY(TEXT_START     , GLFW_KEY(UP   ));
        NK_KEY(TEXT_END       , GLFW_KEY(DOWN ));
        NK_KEY(TEXT_LINE_START, GLFW_KEY(LEFT ));
        NK_KEY(TEXT_LINE_END  , GLFW_KEY(RIGHT));
    } else {
        NK_KEY(UP   , GLFW_KEY(UP   ));
        NK_KEY(DOWN , GLFW_KEY(DOWN ));
        NK_KEY(LEFT , GLFW_KEY(LEFT ));
        NK_KEY(RIGHT, GLFW_KEY(RIGHT));
        NK_KEY(CUT  , 0);
        NK_KEY(COPY , 0);
        NK_KEY(PASTE, 0);
        NK_KEY(SHIFT, 0);
    }

    double mouseX, mouseY; int mouseXi, mouseYi;
    glfwGetCursorPos(nk_glfw.window, &mouseX, &mouseY);
    mouseXi = (int)mouseX; mouseYi = (int)mouseY;
    nk_input_motion(&nk_glfw.context, mouseXi, mouseYi);
    nk_input_scroll(&nk_glfw.context, nk_glfw.scroll);
    NK_BTN(LEFT  , GLFW_BTN(LEFT  ));
    NK_BTN(MIDDLE, GLFW_BTN(MIDDLE));
    NK_BTN(RIGHT , GLFW_BTN(RIGHT ));

    nk_glfw.textLength = 0;
    nk_glfw.scroll = 0.0f;
    nk_input_end(&nk_glfw.context);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void nk_glfw_render(void) {
    glPushAttrib(GL_ENABLE_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0,(GLfloat)nk_glfw.width, (GLfloat)nk_glfw.height,0.0, -1.0,+1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    {
        GLvoid *ptrVertPos = (GLvoid *)( (int8_t *)nk_glfw.vertices + offsetof(struct nk_draw_vertex, position) );
        GLvoid *ptrVertTex = (GLvoid *)( (int8_t *)nk_glfw.vertices + offsetof(struct nk_draw_vertex, uv      ) );
        GLvoid *ptrVertCol = (GLvoid *)( (int8_t *)nk_glfw.vertices + offsetof(struct nk_draw_vertex, col     ) );
        glVertexPointer  (2, GL_FLOAT,         sizeof(struct nk_draw_vertex), ptrVertPos);
        glTexCoordPointer(2, GL_FLOAT,         sizeof(struct nk_draw_vertex), ptrVertTex);
        glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(struct nk_draw_vertex), ptrVertCol);
    }

    {
        struct nk_convert_config convertConfig;
        memset(&convertConfig, 0, sizeof(struct nk_convert_config));
        convertConfig.global_alpha = 1.0f;
        convertConfig.shape_AA = convertConfig.line_AA = nk_glfw.aliasing;
        convertConfig.circle_segment_count = convertConfig.curve_segment_count = convertConfig.arc_segment_count = 22;
        convertConfig.null = nk_glfw.textureNull;
        struct nk_buffer buffVertices, buffElements;
        nk_buffer_init_fixed(&buffVertices, nk_glfw.vertices, SIZE_BUFFER_VERTEX);
        nk_buffer_init_fixed(&buffElements, nk_glfw.elements, SIZE_BUFFER_ELEMENT);
        nk_convert(&nk_glfw.context, &nk_glfw.commands, &buffVertices, &buffElements, &convertConfig);
    }

    {
        double scaleX = nk_glfw.pixelWidth  / nk_glfw.width;
        double scaleY = nk_glfw.pixelHeight / nk_glfw.height;
        GLint clipX, clipY, clipW, clipH;

        const struct nk_draw_command *currCommand;
        const nk_draw_index *currElement = nk_glfw.elements;

        nk_draw_foreach(currCommand, &nk_glfw.context, &nk_glfw.commands) {
            if(!currCommand->elem_count) { continue; }

            clipX = (GLint)(scaleX * (double)currCommand->clip_rect.x);
            clipY = (GLint)(nk_glfw.height - (int)(scaleY * (double)(currCommand->clip_rect.y + currCommand->clip_rect.h)));
            clipW = (GLint)(scaleX * (double)currCommand->clip_rect.w);
            clipH = (GLint)(scaleY * (double)currCommand->clip_rect.h);

            glBindTexture(GL_TEXTURE_2D, (GLuint)currCommand->texture.id);
            glScissor(clipX, clipY, clipW, clipH);
            glDrawElements(GL_TRIANGLES, (GLsizei)currCommand->elem_count, GL_UNSIGNED_SHORT, currElement);
            currElement += currCommand->elem_count;
        }

        nk_clear(&nk_glfw.context);
    }

    glPopClientAttrib();
    glPopAttrib();
}

//==================================================================================================================================
