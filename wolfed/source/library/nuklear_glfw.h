#ifndef NUKLEAR_GLFW_H
#define NUKLEAR_GLFW_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "nuklear.h"

#define SIZE_BUFFER_VERTEX  (512 * 1024)
#define SIZE_BUFFER_ELEMENT (128 * 1024)
#define SIZE_BUFFER_TEXT    (256 * 4)

struct nk_context *nk_glfw_init(GLFWwindow *window, bool antiAlias, bool installCallbacks);
void nk_glfw_shutdown(void);

void nk_glfw_font_begin(struct nk_font_atlas **atlas);
void nk_glfw_font_end(void);

void nk_glfw_input(void);
void nk_glfw_render(void);

void nk_glfw_sizes(int *width, int *height, int *pixelWidth, int *pixelHeight);

void nk_glfw_callback_char(GLFWwindow *window, unsigned int codepoint);
void nk_glfw_callback_scroll(GLFWwindow *window, double xOffset, double yOffset);
void nk_glfw_callback_resize(GLFWwindow *window, int pixelWidth, int pixelHeight);
void nk_glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);

#endif //NUKLEAR_GLFW_H
