#ifndef WOLFCAST_RAYCAST_H
#define WOLFCAST_RAYCAST_H

#include "../shared/stdint.h"
#include "../shared/map.h"

typedef enum {
    HIT_TYPE_MISS,
    HIT_TYPE_VERTICAL,
    HIT_TYPE_HORIZONTAL
} HitType;

typedef struct {
    double  x, y;
    double  distance;
    HitType type;
    uint8   wall;
    double  offset;
    uint16  *entities;
    uint16  entityCount;
} Hit;

Hit *Raycast(Map *map, double x, double y, double angle);

#endif //WOLFCAST_RAYCAST_H
