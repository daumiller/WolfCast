#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "wolfcast.h"
#include "raycast.h"
#include "render.h"

#define COLOR_CEILING_3F 0.445, 0.445, 0.445
#define COLOR_FLOOR_3F   0.222, 0.222, 0.222
#define RAD0   0.0000000000000
#define RAD180 3.1415926535897
#define RAD360 6.2831853071795
#define TEXTURE_WIDTH 64.0

static double centerY;
static int sliceCount;
static Hit *sliceHit = NULL;
static double halfWorldToScreen = (DRAW_WIDTH / SCREEN_DISTANCE_UNITS) / 2.0;

//==================================================================================================================================
static void RaycastView(double x, double y, double angle);

//==================================================================================================================================
bool InitRender() {
    sliceCount = (int)DRAW_WIDTH;
    sliceHit = malloc(sizeof(Hit) * sliceCount);
    centerY = DRAW_HEIGHT / 2.0;
    return true;
}

void CleanupRender() {
    free(sliceHit);
}

//==================================================================================================================================
void RenderView(double x, double y, double angle) {
    glPushMatrix();
    glTranslatef(0.0f, centerY, 0.0f);
    glLineWidth(state.pixelWidth / DRAW_WIDTH);

    // Draw Ceiling & Floor
    glBegin(GL_QUADS);
    glColor3f(COLOR_CEILING_3F);
    glVertex3d(       0.0, -centerY, 0.9);
    glVertex3d(DRAW_WIDTH, -centerY, 0.9);
    glVertex3d(DRAW_WIDTH,      0.0, 0.9);
    glVertex3d(       0.0,      0.0, 0.9);
    glColor3f(COLOR_FLOOR_3F);
    glVertex3d(       0.0,     0.0, 0.9);
    glVertex3d(DRAW_WIDTH,     0.0, 0.9);
    glVertex3d(DRAW_WIDTH, centerY, 0.9);
    glVertex3d(       0.0, centerY, 0.9);
    glEnd();

    RaycastView(x, y, angle);

    glEnable(GL_TEXTURE_1D);
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(state.pixelWidth / DRAW_WIDTH);
    
    for(int sliceIndex=0; sliceIndex<sliceCount; sliceIndex++) {
        Hit *hit = &sliceHit[sliceIndex];
        if(hit->type == HIT_TYPE_MISS) { continue; }

        // printf("Column %d hit Wall %d.\n", sliceIndex, (int)hit->wall);
        // TODO: Door width
        
        int uv = (int)(hit->offset * TEXTURE_WIDTH);
        glBindTexture(GL_TEXTURE_1D, state.wallTextures[hit->wall][uv]);
        glBegin(GL_LINES);
        // TODO: distance brightness
        glColor3ub(255, 255, 255);
        // TODO: wall scale
        double height = (UNITS_PER_BLOCK / hit->distance) * SCREEN_DISTANCE_UNITS;
        height *= halfWorldToScreen;

        glTexCoord1i(0); glVertex3d((double)sliceIndex, -height, 0.8);
        glTexCoord1i(1); glVertex3d((double)sliceIndex,  height, 0.8);

        glEnd();
    }
    
    glPopAttrib();
    glDisable(GL_TEXTURE_1D);
    glPopMatrix();
}

//==================================================================================================================================
static void RaycastView(double x, double y, double angle) {
    double theta, ncos = cos(-angle), nsin = sin(-angle);
    for(int sliceIndex=0; sliceIndex<sliceCount; sliceIndex++) {
        // TODO: verify constants
        double localX = 14.0, localY = -8 + ((double)sliceIndex * 16.0/(double)sliceCount);
        double globalX = x + (localX * ncos) + (localY * nsin);
        double globalY = y + (localY * ncos) - (localX * nsin);

        if(globalX == x) {
            theta = (globalY > y) ? RAD0 : RAD180;
        } else {
            theta = atan((y - globalY) / (x - globalX));
            if(globalX < x) { theta += RAD180; }
        }
        // double fishEye = cos(angle - theta); // TODO: remove if okay after 0-360 clipping

        while(theta > RAD360) { theta -= RAD360; }
        while(theta < RAD0  ) { theta += RAD360; }

        Hit *hit = Raycast(state.map, x, y, theta);
        memcpy(&sliceHit[sliceIndex], hit, sizeof(Hit));
        sliceHit[sliceIndex].distance *= cos(angle - theta); // fishEye
    }
}
