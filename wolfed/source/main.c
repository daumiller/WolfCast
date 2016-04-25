#include <stdio.h>
#include <GLFW/glfw3.h>
#include "library/nuklear_glfw.h"

#define STARTUP_WIDTH  960
#define STARTUP_HEIGHT 720

int main(int argc, char ** argv) {
    if(!glfwInit()) { printf("glfwInit() failed\n"); return -1; }
    GLFWwindow *window = glfwCreateWindow(STARTUP_WIDTH, STARTUP_HEIGHT, "WolfEd", NULL, NULL);
    glfwMakeContextCurrent(window);

    struct nk_context *nuklear = nk_glfw_init(window, true, true);
    nk_glfw_font_begin(NULL);
    nk_glfw_font_end();

    struct nk_color bgcolor = nk_rgb(28, 48, 62);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        nk_glfw_frame_input();

        {
            struct nk_panel layout, colors;
            static int angle, direction = 0;

            if(nk_begin(nuklear, &layout, "WolfEd", nk_rect(50, 50, 235, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
                nk_layout_row_static(nuklear, 30, 80, 1);
                if(nk_button_label(nuklear, "Okay", NK_BUTTON_DEFAULT)) { printf("Okay Then...\n"); }

                nk_layout_row_dynamic(nuklear, 30, 2);
                if(nk_option_label(nuklear, "Left",  direction == 0)) { direction = 0; }
                if(nk_option_label(nuklear, "Right", direction == 1)) { direction = 1; }

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
        }

        glClear(GL_COLOR_BUFFER_BIT);
        {
            float bgcolorUB[4];
            nk_color_fv(bgcolorUB, bgcolor);
            glClearColor(bgcolorUB[0], bgcolorUB[1], bgcolorUB[2], bgcolorUB[3]);
        }
        nk_glfw_frame_render();
        glfwSwapBuffers(window);
    }

    nk_glfw_shutdown();
    glfwTerminate();
    return 0;
}
