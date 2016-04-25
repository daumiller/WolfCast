#include <math.h>
#include <GLFW/glfw3.h>
#include "wolfcast.h"
#include "handle.h"

#define RAD0   0.0000000000000
#define RAD360 6.2831853071795

//==================================================================================================================================
static void HandleResize(GLFWwindow *window, int pixelWidth, int pixelHeight);
static void HandleKeys(GLFWwindow* window, int key, int scancode, int action, int mods);

static void CameraMove(double magnitude);
static void CameraRotate(double theta);

//==================================================================================================================================
bool InitHandle() {
    glfwSetFramebufferSizeCallback(state.window, HandleResize);
    glfwSetKeyCallback(state.window, HandleKeys);
    return true;
}

void CleanupHandle() {
}

//==================================================================================================================================
static void HandleResize(GLFWwindow *window, int pixelWidth, int pixelHeight) {
    state.pixelWidth  = pixelWidth;
    state.pixelHeight = pixelHeight;
    glfwGetWindowSize(window, &state.scaleWidth, &state.scaleHeight);
    glViewport(0, 0, pixelWidth, pixelHeight);
}

static void HandleKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if(mods & GLFW_MOD_SUPER) {
        } else {
            if(key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GL_TRUE); }
        }
    }

    if((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
             if((key == GLFW_KEY_A) || (key == GLFW_KEY_LEFT )) { CameraRotate(-0.1); }
        else if((key == GLFW_KEY_D) || (key == GLFW_KEY_RIGHT)) { CameraRotate( 0.1); }
        else if((key == GLFW_KEY_A) || (key == GLFW_KEY_UP   )) { CameraMove  ( 3.0); }
        else if((key == GLFW_KEY_S) || (key == GLFW_KEY_DOWN )) { CameraMove  (-3.0); }
    }
}

//==================================================================================================================================
static void CameraMove(double magnitude) {
    state.cameraX += magnitude * cos(state.cameraAngle);
    state.cameraY += magnitude * sin(state.cameraAngle);
}

static void CameraRotate(double theta) {
    state.cameraAngle += theta;
    while(state.cameraAngle >= RAD360) { state.cameraAngle -= RAD360; }
    while(state.cameraAngle <  RAD0  ) { state.cameraAngle += RAD360; }
}
