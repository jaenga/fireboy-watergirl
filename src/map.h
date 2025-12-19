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
    TILE_SWITCH = 'S',     // 플레이어 스위치 (Fireboy/Watergirl이 활성화)
    TILE_BOX_SWITCH = 'X', // 상자 스위치 (상자만 활성화 가능)
    TILE_DOOR = 'D',       // 도어
    TILE_MOVING_PLATFORM = 'P', // 이동 발판
    TILE_TOGGLE_PLATFORM = 'T', // 토글 발판 (스위치로 위/아래 이동)
    TILE_TOGGLE_TARGET = 't',   // 토글 발판 목표 위치 (보이지 않음)
    TILE_VERTICAL_WALL = 'V',   // 수직 이동 벽 (박스+스위치로 제어)
    TILE_VERTICAL_TARGET = 'v', // 수직 벽 목표 위치 (보이지 않음)
    TILE_FIRE_GEM = 'R',   // Fireboy 전용 보석
    TILE_WATER_GEM = 'b',  // Watergirl 전용 보석
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

    // 상자 정보
    int box_count;
    struct {
        int x;
        int y;
        float vy;
        bool active;
    } boxes[MAX_BOXES];

    // 스위치/도어 정보
    int switch_count;
    struct {
        int x;
        int y;
        bool activated;
        bool toggle_state;
        bool is_box_switch;  // true면 상자 스위치, false면 플레이어 스위치
        char group_id[32];  // 스위치 그룹 ID (예: "PLATFORM_1")
    } switches[MAX_SWITCHES];
    
    int door_count;
    struct {
        int x;
        int y;
        bool is_open;
    } doors[MAX_DOORS];

    // 이동 발판 정보
    int platform_count;
    struct {
        float x;
        float y;
        float vx;
        float vy;
        int min_x;
        int max_x;
        int min_y;
        int max_y;
        bool vertical;
        bool active;
    } platforms[MAX_PLATFORMS];
    
    // 토글 발판 정보
    int toggle_platform_count;
    struct {
        int x;
        int width;
        float y;
        int original_y;
        int target_y;
        bool moving_down;
        bool target_is_down;
        int linked_switch;  // 하위 호환성을 위해 유지
        char linked_group[32];  // 구독할 스위치 그룹 ID
    } toggle_platforms[MAX_PLATFORMS];
    
    // 수직 이동 벽 정보
    int vertical_wall_count;
    struct {
        int x;
        float y;
        int original_y;
        int target_y;
        bool is_up;
        int linked_switch;  // 하위 호환성을 위해 유지
        char linked_group[32];  // 구독할 스위치 그룹 ID
    } vertical_walls[MAX_PLATFORMS];
} Map;

// 전방 선언
typedef struct Player Player;

// 함수 선언
Map* map_create(int width, int height);
void map_destroy(Map* map);
Map* map_load_from_file(const char* filename);
bool map_is_walkable(const Map* map, int x, int y, bool is_fireboy);
TileType map_get_tile(const Map* map, int x, int y);
void map_set_tile(Map* map, int x, int y, TileType tile);

// 상자 관련
int map_get_box_count(const Map* map);
int map_get_box_x(const Map* map, int index);
int map_get_box_y(const Map* map, int index);
int map_find_box(const Map* map, int x, int y);
bool map_move_box(Map* map, int index, int new_x, int new_y);
void map_update_boxes(Map* map, float delta_time);
void map_reset_boxes(Map* map);

// 스위치/도어 관련
int map_get_switch_count(const Map* map);
int map_get_switch_x(const Map* map, int index);
int map_get_switch_y(const Map* map, int index);
bool map_is_switch_activated(const Map* map, int index);
int map_find_switch(const Map* map, int x, int y);
void map_update_switches(Map* map, int fireboy_x, int fireboy_y, int watergirl_x, int watergirl_y);
void map_update_doors(Map* map);

// 발판 관련
void map_update_platforms(Map* map, float delta_time, struct Player* fireboy, struct Player* watergirl);
void map_update_toggle_platforms(Map* map, float delta_time);
void map_update_vertical_walls(Map* map, float delta_time);

#endif // MAP_H
