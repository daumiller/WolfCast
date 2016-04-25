#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "../shared/map.h"
#include "raycast.h"

#define RAD0   0.0000000000000
#define RAD90  1.5707963267949
#define RAD180 3.1415926535897
#define RAD270 4.7123889803847

#define UNIT_TO_BLOCK(x) ((int)(x) >> UNIT_BLOCK_SHIFT)
#define BLOCK_TO_UNIT(x) ((x) << UNIT_BLOCK_SHIFT)

//==================================================================================================================================
static void RaycastVert(Map *map, double x, double y, double angle);
static void RaycastHorz(Map *map, double x, double y, double angle);
static bool RaycastVertTest(Map *map, double y, double angle, double m, double b, int blockX, double unitY, int blockOffsetX);
static bool RaycastHorzTest(Map *map, double x, double angle, double m, double b, int blockY, double unitX, int blockOffsetY);
static void EntityHit(uint16 entityIndex);

//==================================================================================================================================
static uint16 entities[512];
static uint16 entityCount;
static Hit hitHorz, hitVert;

Hit *Raycast(Map *map, double x, double y, double angle) {
    entityCount = 0;
    RaycastHorz(map, x, y, angle);
    RaycastVert(map, x, y, angle);

    if((hitHorz.type == HIT_TYPE_MISS) || ((hitVert.type != HIT_TYPE_MISS) && (hitVert.distance < hitHorz.distance))) {
        hitVert.entities = entities;
        hitVert.entityCount = entityCount;
        return &hitVert;
    } else {
        hitHorz.entities = entities;
        hitHorz.entityCount = entityCount;
        return &hitHorz;
    }
}

//==================================================================================================================================
static void RaycastVert(Map *map, double x, double y, double angle) {
    hitVert.type = HIT_TYPE_MISS;
    if((angle == RAD90) || (angle == RAD270)) { return; }

    double m = tanf(angle);
    double b = y - (m * x);

    if((RAD90 <= angle) && (angle <= RAD270)) {
        for(int blockX = UNIT_TO_BLOCK(x)-1; blockX >= 0; blockX--) {
            if(blockX >= map->blockWidth) { continue; } // NOCLIP:
            double unitY = b + (m * (double)(BLOCK_TO_UNIT(blockX+1)));
            if(RaycastVertTest(map, y, angle, m, b, blockX, unitY, 1)) {
                hitVert.type = HIT_TYPE_VERTICAL;
                return;
            }
        }
    } else {
        for(int blockX = UNIT_TO_BLOCK(x)+1; blockX < map->blockWidth; blockX++) {
            if(blockX < 0) { continue; } // NOCLIP:
            double unitY = b + (m * (double)BLOCK_TO_UNIT(blockX));
            if(RaycastVertTest(map, y, angle, m, b, blockX, unitY, 0)) {
                hitVert.type = HIT_TYPE_VERTICAL;
                return;
            }
        }
    }
}

static bool RaycastVertTest(Map *map, double y, double angle, double m, double b, int blockX, double unitY, int blockOffsetX) {
    int blockY = UNIT_TO_BLOCK(unitY);
    if((blockY < 0) || (blockY >= map->blockHeight)) { return false; }

    // hit a non-door entity?
    int entityIndex = map->entityGrid[blockY][blockX];
    if((entityIndex > 0) && (map->entity[entityIndex-1]->type > ENTITY_TYPE_DOOR)) {
        EntityHit(entityIndex-1);
    }

    // not a wall?
    if(map->wallGrid[blockY][blockX] == 0) { return false; }

    // is a door?
    double unitX;
    bool isDoor = (entityIndex > 0) && (map->entity[entityIndex-1]->type == ENTITY_TYPE_DOOR);
    if(isDoor) {
        unitX = ((double)blockX + 0.5) * UNITS_PER_BLOCK;
        unitY = (m * unitX) + b;
    } else {
        unitX = BLOCK_TO_UNIT(blockX+blockOffsetX);
    }

    hitVert.wall = map->wallGrid[blockY][blockX];
    hitVert.x = unitX;
    hitVert.y = unitY;
    hitVert.distance = fabs((unitY - y) / sinf(angle));
    // TODO: verify
    unitY = fabs(unitY);
    hitVert.offset = (unitY / (double)UNITS_PER_BLOCK) - (double)UNIT_TO_BLOCK(unitY);
    if(blockOffsetX || isDoor) { hitVert.offset = 1.0 - hitVert.offset; }

    // through a door?
    if((isDoor) && (hitVert.offset > map->entity[entityIndex-1]->closed)) {
        return false;
    }

    return true;
}

//==================================================================================================================================
static void RaycastHorz(Map *map, double x, double y, double angle) {
    hitHorz.type = HIT_TYPE_MISS;
    if((angle == RAD0) || (angle == RAD180)) { return; }

    double m = tanf(angle);
    double b = y - (m * x);

    if((RAD0 <= angle) && (angle <= RAD180)) {
        for(int blockY = UNIT_TO_BLOCK(y)+1; blockY < map->blockHeight; blockY++) {
            if(blockY < 0) { continue; } // NOCLIP:
            double unitX = ((double)BLOCK_TO_UNIT(blockY) - b) / m;
            if(RaycastHorzTest(map, x, angle, m, b, blockY, unitX, 0)) {
                hitHorz.type = HIT_TYPE_HORIZONTAL;
                return;
            }
        }
    } else {
        for(int blockY = UNIT_TO_BLOCK(y)-1; blockY >= 0; blockY--) {
            if(blockY >= map->blockHeight) { continue; } // NOCLIP:
            double unitX = ((double)BLOCK_TO_UNIT(blockY+1) - b) / m;
            if(RaycastHorzTest(map, x, angle, m, b, blockY, unitX, 1)) {
                hitHorz.type = HIT_TYPE_HORIZONTAL;
                return;
            }   
        }
    }
}

static bool RaycastHorzTest(Map *map, double x, double angle, double m, double b, int blockY, double unitX, int blockOffsetY) {
    int blockX = UNIT_TO_BLOCK(unitX);
    if((blockX < 0) || (blockX >= map->blockWidth)) { return false; }

    // hit a non-door entity?
    int entityIndex = map->entityGrid[blockY][blockX];
    if((entityIndex > 0) && (map->entity[entityIndex-1]->type > ENTITY_TYPE_DOOR)) {
        EntityHit(entityIndex);
    }

    // not a wall?
    if(map->wallGrid[blockY][blockX] == 0) { return false; }

    // is a door?
    double unitY;
    bool isDoor = (entityIndex > 0) && (map->entity[entityIndex-1]->type == ENTITY_TYPE_DOOR);
    if(isDoor) {
        unitY = ((double)blockY + 0.5) * UNITS_PER_BLOCK;
        unitX = (unitY - b) / m;
    } else {
        unitY = BLOCK_TO_UNIT(blockY+blockOffsetY);
    }

    hitHorz.wall = map->wallGrid[blockY][blockX];
    hitHorz.x = unitX;
    hitHorz.y = unitY;
    hitHorz.distance = fabs((unitX - x) / cosf(angle));
    // TODO: verify
    unitX = fabs(unitX);
    hitHorz.offset = (unitX / (double)UNITS_PER_BLOCK) - (double)UNIT_TO_BLOCK(unitX);
    if(!blockOffsetY || isDoor) {
        hitHorz.offset = 1.0 - hitHorz.offset;
    }

    // through a door?
    if((isDoor) && (hitHorz.offset > map->entity[entityIndex-1]->closed)) {
        return false;
    }

    return true;
}

//==================================================================================================================================
static void EntityHit(uint16 entityIndex) {
    if(entityCount == 0xFFFF) {
        fprintf(stderr, "ERROR: EntityHit() overflow.\n");
        return;
    }
    for(int index=0; index<entityCount; index++) {
        if(entities[index] == entityIndex) { return; }
    }
    entities[entityCount] = entityIndex;
    entityCount++;
}
