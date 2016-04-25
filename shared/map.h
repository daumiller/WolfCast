#ifndef WOLFCAST_MAP_H
#define WOLFCAST_MAP_H

#include "stdint.h"

#define UNITS_PER_BLOCK       8
#define UNIT_BLOCK_SHIFT      3
#define FEET_PER_BLOCK        5
#define FEET_PER_UNIT         0.625
#define SCREEN_DISTANCE_UNITS 1.75

#define ENTITY_TYPE_INVALID 4
#define ENTITY_TYPE_DOOR    5
#define ENTITY_TYPE_SPAWN   6
#define ENTITY_TYPE_KEY     7
#define ENTITY_TYPE_WEAPON  8
#define ENTITY_TYPE_AMMO    9
#define ENTITY_TYPE_ENEMY  10

#define DOOR_SECURITY_NONE    0
#define DOOR_SECURITY_RED     1
#define DOOR_SECURITY_YELLOW  2
#define DOOR_SECURITY_BLUE    3

#define DEFAULT_COLOR_FLOOR   0x727272FF
#define DEFAULT_COLOR_CEILING 0x393939FF

typedef struct {
    uint8 type;
    uint32 flags;
    double x, y;
    union {
        double facing;
        double closed;
    };
} Entity;

typedef struct {
    char *id;
    char *name;
    int width, height;
    int blockWidth, blockHeight;

    uint32 floor, ceiling;
    uint16 entityCount;
    Entity **entity;

    uint8  **wallGrid;
    uint16 **entityGrid;
} Map;

Map *MapLoad(const char *path);
Map *MapNew(int blockWidth, int blockHeight);
int  MapSave(Map *map, const char *path);
void MapFree(Map *map);

void MapExpand(Map *map, int left, int top, int right, int bottom);
void MapContract(Map *map);

int MapEntityAdd(Map *map, int x, int y, int type);
void MapEntityDel(Map *map, uint16 index);

#endif //WOLFCAST_MAP_H
