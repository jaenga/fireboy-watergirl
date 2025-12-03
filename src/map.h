#ifndef MAP_H
#define MAP_H

#include "common.h"

// 타일 종류
typedef enum {
    TILE_EMPTY = ' ',      // 빈 공간
    TILE_WALL = '#',       // 벽
    TILE_FLOOR = '.',      // 바닥
    TILE_FIRE_TERRAIN = 'F', // 불 지형 (Fireboy만 통과 가능)
    TILE_WATER_TERRAIN = 'W', // 물 지형 (Watergirl만 통과 가능)
    TILE_BOX = 'B',        // 상자
    TILE_SWITCH = 'S',     // 스위치
    TILE_DOOR = 'D',       // 도어
    TILE_MOVING_PLATFORM = 'P', // 이동 발판
    TILE_GEM = 'G',        // 보석
    TILE_FIREBOY_START = 'f',  // Fireboy 시작 위치
    TILE_WATERGIRL_START = 'w', // Watergirl 시작 위치
    TILE_EXIT = 'E'        // 출구
} TileType;

// 맵 구조체
typedef struct {
    int width;
    int height;
    TileType** tiles;  // 2D 배열
    int fireboy_start_x;
    int fireboy_start_y;
    int watergirl_start_x;
    int watergirl_start_y;
    int exit_x;
    int exit_y;
} Map;

// 함수 선언
Map* map_create(int width, int height);
void map_destroy(Map* map);
Map* map_load_from_file(const char* filename);
bool map_is_walkable(const Map* map, int x, int y, bool is_fireboy);
TileType map_get_tile(const Map* map, int x, int y);
void map_set_tile(Map* map, int x, int y, TileType tile);

#endif // MAP_H

