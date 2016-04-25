#ifndef WOLFCAST_FONT_H
#define WOLFCAST_FONT_H

#include <stdarg.h>
#include <stdbool.h>
#include "../shared/stdint.h"

bool FontLoad(void);
int FontWriteString(double x, double y, double scale, uint32 color, const char *string);
int FontWriteStringF(double x, double y, double scale, uint32 color, const char *format, ...);
void FontFree(void);

#endif //WOLFCAST_FONT_H
