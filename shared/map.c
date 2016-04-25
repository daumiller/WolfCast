#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

//==================================================================================================================================
static Entity *entityRead(FILE *file);
static void entityWrite(Entity *entity, FILE *file);
static void entityFree(Entity *entity);

static uint8 **gridRead8(FILE *file, int width, int height);
static void gridWrite8(uint8 **grid, int width, int height, FILE *file);
static void gridFree8(uint8 **grid, int height);
static uint8 **gridAlloc8(int width, int height);

static uint16 **gridRead16(FILE *file, int width, int height);
static void gridWrite16(uint16 **grid, int width, int height, FILE *file);
static void gridFree16(uint16 **grid, int height);
static uint16 **gridAlloc16(int width, int height);

static char *stringRead(FILE *file);
static void stringWrite(const char *string, FILE *file);

//==================================================================================================================================
Map *MapLoad(const char *path) {
    FILE *file = fopen(path, "rb");
    if(!file) {
        fprintf(stderr, "ERROR: Unable to open map file (%s).\n", path);
        return NULL;
    }

    char signature[4];
    fread(signature, 4, 1, file);
    if((signature[0] != '4') || (signature[1] != '3') || (signature[2] != '3') || (signature[3] != '1')) {
        fprintf(stderr, "ERROR: Map has bad signature (%s).\n", path);
        fclose(file);
        return NULL;
    }

    Map *map = malloc(sizeof(Map));
    map->id   = stringRead(file);
    map->name = stringRead(file);
    fread(&(map->width), 4, 1, file);
    fread(&(map->height), 4, 1, file);
    fread(&(map->blockWidth), 4, 1, file);
    fread(&(map->blockHeight), 4, 1, file);
    fread(&(map->floor), 4, 1, file);
    fread(&(map->ceiling), 4, 1, file);

    fread(&(map->entityCount), 2, 1, file);
    map->entity = malloc(sizeof(Entity *) * map->entityCount);
    for(uint16 idx=0; idx<map->entityCount; idx++) {
        map->entity[idx] = entityRead(file);
    }

    map->wallGrid   = gridRead8 (file, map->blockWidth, map->blockHeight);
    map->entityGrid = gridRead16(file, map->blockWidth, map->blockHeight);

    return map;
}

Map *MapNew(int blockWidth, int blockHeight) {
    Map *map = malloc(sizeof(Map));
    map->id   = strdup("ExMx");
    map->name = strdup("Utitled Map");
    map->width  = blockWidth  << UNIT_BLOCK_SHIFT;
    map->height = blockHeight << UNIT_BLOCK_SHIFT;
    map->blockWidth  = blockWidth;
    map->blockHeight = blockHeight;
    map->floor   = DEFAULT_COLOR_FLOOR;
    map->ceiling = DEFAULT_COLOR_CEILING;

    map->entityCount = 0; map->entity = NULL;
    map->wallGrid   = gridAlloc8 (blockWidth, blockHeight);
    map->entityGrid = gridAlloc16(blockWidth, blockHeight);
    return map;
}

int MapSave(Map *map, const char *path) {
    MapContract(map);

    FILE *fout = fopen(path, "wb");
    if(!fout) { return 0; }

    fwrite("4331", 4, 1, fout);
    stringWrite(map->id ? map->id : "", fout);
    stringWrite(map->name ? map->name : "", fout);
    fwrite(&map->width, 4, 1, fout);
    fwrite(&map->height, 4, 1, fout);
    fwrite(&map->blockWidth, 4, 1, fout);
    fwrite(&map->blockHeight, 4, 1, fout);
    fwrite(&map->floor, 4, 1, fout);
    fwrite(&map->ceiling, 4, 1, fout);
    fwrite(&map->entityCount, 2, 1, fout);
    for(uint16 idx=0; idx < map->entityCount; idx++) {
        entityWrite(map->entity[idx], fout);
    }
    gridWrite8 (map->wallGrid  , map->blockWidth, map->blockHeight, fout);
    gridWrite16(map->entityGrid, map->blockWidth, map->blockHeight, fout);

    fclose(fout);
    return 1;
}

void MapFree(Map *map) {
    if(map->id)   { free(map->id); }
    if(map->name) { free(map->name); }
    for(uint16 idx=0; idx<map->entityCount; idx++) { entityFree(map->entity[idx]); } if(map->entityCount) { free(map->entity); }
    gridFree8 (map->wallGrid  , map->blockHeight);
    gridFree16(map->entityGrid, map->blockHeight);
    free(map);
}

void MapExpand(Map *map, int left, int top, int right, int bottom) {
    int newWidth = map->blockWidth + left + right;
    int newHeight = map->blockHeight + top + bottom;

    int cutoffY = top + map->blockHeight;

    uint8  **newwallGrid   = malloc(sizeof(uint8 *) * newHeight);
    uint16 **newEntityGrid = malloc(sizeof(uint16 *) * newHeight);
    for(int y=0; y<newHeight; y++) {
        uint8  *tileRow   = calloc(newWidth, 1);
        uint16 *entityRow = calloc(newWidth, 2);
        if((y >= top) && (y < cutoffY)) {
            memcpy(tileRow  +left, map->wallGrid  [y-top], map->blockWidth);
            memcpy(entityRow+left, map->entityGrid[y-top], map->blockWidth << 1);
        }
        newwallGrid[y]   = tileRow;
        newEntityGrid[y] = entityRow;
    }

    gridFree8 (map->wallGrid,   map->blockHeight); map->wallGrid   = newwallGrid;
    gridFree16(map->entityGrid, map->blockHeight); map->entityGrid = newEntityGrid;
    map->blockWidth  = newWidth;  map->width  = newWidth  << UNIT_BLOCK_SHIFT;
    map->blockHeight = newHeight; map->height = newHeight << UNIT_BLOCK_SHIFT;
}

void MapContract(Map *map) {
    int yMax = 0, yMin = map->blockHeight - 1; if(map->blockHeight == 0) { yMin = 0; }
    int xMax = 0, xMin = map->blockWidth - 1;  if(map->blockWidth  == 0) { xMin = 0; }
    int hit;

    for(int y=0; y < map->blockHeight; y++) {
        hit = 0;
        for(int x=0; x < map->blockWidth; x++) {
            if(map->wallGrid[y][x] > 0) {
                hit = 1;
                if(x < xMin) { xMin = x; }
                if(x > xMax) { xMax = x; }
            }
        }
        if(hit) {
            if(y < yMin) { yMin = y; }
            if(y > yMax) { yMax = y; }
        }
    }

    int newHeight = (yMax - yMin) + 1;
    int newWidth  = (xMax - xMin) + 1;
    if((yMin == xMin == 0) && (newHeight == map->blockHeight) && (newWidth == map->blockWidth)) {
        return;
    }

    uint8  **newwallGrid   = malloc(sizeof(uint8  *) * newHeight);
    uint16 **newEntityGrid = malloc(sizeof(uint16 *) * newHeight);
    for(int y=0; y<newHeight; y++) {
        newwallGrid[y]   = calloc(newWidth, 1);
        newEntityGrid[y] = calloc(newWidth, 2);
        memcpy(newwallGrid[y],   map->wallGrid  [y+yMin]+xMin, newWidth);
        memcpy(newEntityGrid[y], map->entityGrid[y+yMin]+xMin, newWidth << 1);
    }

    gridFree8 (map->wallGrid,   map->blockHeight); map->wallGrid   = newwallGrid;
    gridFree16(map->entityGrid, map->blockHeight); map->entityGrid = newEntityGrid;
    map->blockWidth  = newWidth;  map->width  = newWidth  << UNIT_BLOCK_SHIFT;
    map->blockHeight = newHeight; map->height = newHeight << UNIT_BLOCK_SHIFT;
}

int MapEntityAdd(Map *map, int x, int y, int type) {
    Entity *entity = malloc(sizeof(Entity));
    entity->type = (uint8)(type & 0xFF);
    entity->flags = 0;
    entity->x = (double)x;
    entity->y = (double)y;
    entity->facing = 90.0;

    map->entityCount++;
    map->entity = realloc(map->entity, sizeof(Entity *) * map->entityCount);
    map->entity[map->entityCount - 1] = entity;

    uint16 index = map->entityCount - 1;
    int tileX = x >> UNIT_BLOCK_SHIFT;
    int tileY = y >> UNIT_BLOCK_SHIFT;
    map->entityGrid[tileY][tileX] = index + 1;

    return index;
}

void MapEntityDel(Map *map, uint16 index) {
    int tileX = (int)(map->entity[index]->x) >> UNIT_BLOCK_SHIFT;
    int tileY = (int)(map->entity[index]->y) >> UNIT_BLOCK_SHIFT;
    map->entityGrid[tileY][tileX] = 0;

    free(map->entity[index]);
    for(; index < (map->entityCount - 1); index++) {
        map->entity[index] = map->entity[index+1];
        tileX = (int)(map->entity[index]->x) >> UNIT_BLOCK_SHIFT;
        tileY = (int)(map->entity[index]->y) >> UNIT_BLOCK_SHIFT;
        map->entityGrid[tileY][tileX] = index+1;
    }
    map->entity[map->entityCount-1] = NULL;
    map->entityCount--;
    // map->entity = realloc(map->entity, sizeof(Entity *) * map->entityCount);
}

//==================================================================================================================================
static Entity *entityRead(FILE *file) {
    char buff[4];
    Entity *entity = malloc(sizeof(Entity));
    fread(&(entity->type), 1, 1, file);
    fread(&(entity->flags), 4, 1, file);
    fread(buff, 4, 1, file); entity->x      = (double)*((int *)buff);
    fread(buff, 4, 1, file); entity->y      = (double)*((int *)buff);
    fread(buff, 4, 1, file); entity->facing = (double)*((float *)buff);
    return entity;
}
static void entityWrite(Entity *entity, FILE *file){
    int32 x = (int32)entity->x;
    int32 y = (int32)entity->y;
    float f = (float)entity->facing;

    fwrite(&entity->type, 1, 1, file);
    fwrite(&entity->flags, 4, 1, file);
    fwrite(&x, 4, 1, file);
    fwrite(&y, 4, 1, file);
    fwrite(&f, 4, 1, file);
}
static void entityFree(Entity *entity) {
    free(entity);
}

static uint8 **gridRead8(FILE *file, int width, int height) {
    uint8 **grid = malloc(sizeof(uint8 *) * height);
    for(int row=0; row<height; row++) {
        grid[row] = malloc(width);
        fread(grid[row], width, 1, file);
    }
    return grid;
}
static void gridWrite8(uint8 **grid, int width, int height, FILE *file) {
    for(int row=0; row<height; row++) {
        fwrite(grid[row], width, 1, file);
    }

}
static void gridFree8(uint8 **grid, int height) {
    for(int row=0; row<height; row++) {
        free(grid[row]);
    }
    free(grid);
}
static uint8 **gridAlloc8(int width, int height) {
    uint8 **grid = malloc(sizeof(uint8 *) * height);
    for(int row=0; row<height; row++) {
        grid[row] = calloc(width, 1);
    }
    return grid;
}

static uint16 **gridRead16(FILE *file, int width, int height) {
    uint16 **grid = malloc(sizeof(uint16 *) * height);
    for(int row=0; row<height; row++) {
        grid[row] = malloc(width << 1);
        fread(grid[row], width, 2, file);
    }
    return grid;
}
static void gridWrite16(uint16 **grid, int width, int height, FILE *file) {
    for(int row=0; row<height; row++) {
        fwrite(grid[row], width, 2, file);
    }

}
static void gridFree16(uint16 **grid, int height) {
    for(int row=0; row<height; row++) {
        free(grid[row]);
    }
    free(grid);
}
static uint16 **gridAlloc16(int width, int height) {
    uint16 **grid = malloc(sizeof(uint16 *) * height);
    for(int row=0; row<height; row++) {
        grid[row] = calloc(width, 2);
    }
    return grid;
}

static char *stringRead(FILE *file) {
    uint8 len;
    fread(&len, 1, 1, file);
    char *str = malloc(len + 1);
    fread(str, len, 1, file);
    str[len] = 0x00;
    return str;
}
static void stringWrite(const char *string, FILE *file) {
    uint8 len = (uint8)(strlen(string) & 0xFF);
    fwrite(&len, 1, 1, file);
    fwrite(string, len, 1, file);
}
