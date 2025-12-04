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

    // 상자(Box) 정보
    // - 맵에서 'B' 문자로 정의된 상자들을 별도로 추적
    // - 나중에 물리/충돌/스위치 연동 시 사용
    int box_count;
    struct {
        int x;
        int y;
        float vy;      // 수직 속도 (중력 적용용)
        bool active;   // 나중에 파괴/비활성화 등을 위해
    } boxes[MAX_BOXES];

    // 스위치/도어 정보
    int switch_count;
    struct {
        int x;
        int y;
        bool activated; // 스위치 활성화 여부
    } switches[MAX_SWITCHES];
    
    int door_count;
    struct {
        int x;
        int y;
        bool is_open;  // 도어 열림/닫힘 상태
    } doors[MAX_DOORS];

    // 이동 발판 정보
    int platform_count;
    struct {
        float x;        // 현재 x 위치 (실수, 타일 단위)
        float y;        // 현재 y 위치 (실수, 타일 단위)
        float vx;       // 수평 속도 (타일/초)
        float vy;       // 수직 속도 (타일/초)
        int min_x;      // 이동 범위 최소 x (포함)
        int max_x;      // 이동 범위 최대 x (포함)
        int min_y;      // 이동 범위 최소 y (포함)
        int max_y;      // 이동 범위 최대 y (포함)
        bool vertical;  // true면 위아래 이동, false면 좌우 이동
        bool active;
    } platforms[MAX_PLATFORMS];
} Map;

// 전방 선언 (순환 의존성 방지)
struct Player;

// 함수 선언
Map* map_create(int width, int height);
void map_destroy(Map* map);
Map* map_load_from_file(const char* filename);
bool map_is_walkable(const Map* map, int x, int y, bool is_fireboy);
TileType map_get_tile(const Map* map, int x, int y);
void map_set_tile(Map* map, int x, int y, TileType tile);

// 상자 관련 헬퍼
int map_get_box_count(const Map* map);
int map_get_box_x(const Map* map, int index);
int map_get_box_y(const Map* map, int index);
int map_find_box(const Map* map, int x, int y);          // (x,y)에 있는 상자의 인덱스, 없으면 -1
bool map_move_box(Map* map, int index, int new_x, int new_y); // 상자 한 칸 이동 (타일/배열 동기화)
void map_update_boxes(Map* map, float delta_time);        // 상자 중력/물리 업데이트
void map_reset_boxes(Map* map);                           // 상자 상태 리셋 (속도 초기화)

// 스위치/도어 관련 헬퍼
int map_get_switch_count(const Map* map);
int map_get_switch_x(const Map* map, int index);
int map_get_switch_y(const Map* map, int index);
bool map_is_switch_activated(const Map* map, int index);
int map_find_switch(const Map* map, int x, int y);       // (x,y)에 있는 스위치 인덱스, 없으면 -1
void map_update_switches(Map* map, int fireboy_x, int fireboy_y, int watergirl_x, int watergirl_y); // 스위치 활성화 체크
void map_update_doors(Map* map);                          // 도어 열림/닫힘 업데이트 (스위치 상태에 따라)
void map_update_platforms(Map* map, float delta_time, struct Player* fireboy, struct Player* watergirl); // 이동 발판 업데이트

#endif // MAP_H

