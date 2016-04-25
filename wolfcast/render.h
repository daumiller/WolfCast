#ifndef WOLFCAST_RENDER_H
#define WOLFCAST_RENDER_H

#include <stdbool.h>

bool InitRender(void);
void CleanupRender(void);

void RenderView(double x, double y, double angle);

#endif //WOLFCAST_RENDER_H
