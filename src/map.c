#include "map.h"

// 맵 생성
Map* map_create(int width, int height) {
    Map* map = (Map*)malloc(sizeof(Map));
    if (!map) return NULL;
    
    map->width = width;
    map->height = height;
    map->fireboy_start_x = 0;
    map->fireboy_start_y = 0;
    map->watergirl_start_x = 0;
    map->watergirl_start_y = 0;
    map->exit_x = 0;
    map->exit_y = 0;
    
    // 2D 배열 할당
    map->tiles = (TileType**)malloc(height * sizeof(TileType*));
    if (!map->tiles) {
        free(map);
        return NULL;
    }
    
    for (int i = 0; i < height; i++) {
        map->tiles[i] = (TileType*)malloc(width * sizeof(TileType));
        if (!map->tiles[i]) {
            // 메모리 할당 실패 시 정리
            for (int j = 0; j < i; j++) {
                free(map->tiles[j]);
            }
            free(map->tiles);
            free(map);
            return NULL;
        }
        // 빈 공간으로 초기화
        for (int j = 0; j < width; j++) {
            map->tiles[i][j] = TILE_EMPTY;
        }
    }
    
    return map;
}

// 맵 메모리 해제
void map_destroy(Map* map) {
    if (!map) return;
    
    if (map->tiles) {
        for (int i = 0; i < map->height; i++) {
            free(map->tiles[i]);
        }
        free(map->tiles);
    }
    free(map);
}

// 맵 파일에서 로드
Map* map_load_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("맵 파일을 열 수 없습니다: %s\n", filename);
        return NULL;
    }
    
    char line[MAX_MAP_WIDTH + 10]; // 여유 공간
    int width = 0;
    int height = 0;
    
    // 먼저 맵 크기 계산
    while (fgets(line, sizeof(line), file)) {
        int len = strlen(line);
        // 개행 문자 제거
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        
        if (len > width) {
            width = len;
        }
        height++;
    }
    
    if (width == 0 || height == 0) {
        fclose(file);
        printf("맵 파일이 비어있습니다: %s\n", filename);
        return NULL;
    }
    
    // 맵 생성
    Map* map = map_create(width, height);
    if (!map) {
        fclose(file);
        return NULL;
    }
    
    // 파일 다시 읽기
    rewind(file);
    int y = 0;
    
    while (fgets(line, sizeof(line), file) && y < height) {
        int len = strlen(line);
        // 개행 문자 제거
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        
        for (int x = 0; x < width; x++) {
            if (x < len) {
                char ch = line[x];
                map->tiles[y][x] = (TileType)ch;
                
                // 특수 타일 위치 저장
                if (ch == TILE_FIREBOY_START) {
                    map->fireboy_start_x = x;
                    map->fireboy_start_y = y;
                } else if (ch == TILE_WATERGIRL_START) {
                    map->watergirl_start_x = x;
                    map->watergirl_start_y = y;
                } else if (ch == TILE_EXIT) {
                    map->exit_x = x;
                    map->exit_y = y;
                }
            } else {
                map->tiles[y][x] = TILE_EMPTY;
            }
        }
        y++;
    }
    
    fclose(file);
    return map;
}

// 해당 위치가 이동 가능한지 확인
bool map_is_walkable(const Map* map, int x, int y, bool is_fireboy) {
    if (!map || x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return false;
    }
    
    TileType tile = map->tiles[y][x];
    
    switch (tile) {
        case TILE_EMPTY:
            return true; // 공백은 공중 이동 가능
        case TILE_WALL:
        case TILE_FLOOR:  // 바닥도 벽처럼 통과 불가
            return false;
        case TILE_SWITCH:
        case TILE_BOX:
        case TILE_MOVING_PLATFORM:
        case TILE_GEM:
        case TILE_FIREBOY_START:  // 시작 위치도 이동 가능
        case TILE_WATERGIRL_START: // 시작 위치도 이동 가능
            return true;
        case TILE_FIRE_TERRAIN:
            return is_fireboy; // Fireboy만 통과 가능
        case TILE_WATER_TERRAIN:
            return !is_fireboy; // Watergirl만 통과 가능
        case TILE_DOOR:
            // 나중에 스위치와 연결하여 구현
            return true;
        default:
            return true;
    }
}

// 타일 가져오기
TileType map_get_tile(const Map* map, int x, int y) {
    if (!map || x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return TILE_EMPTY;
    }
    return map->tiles[y][x];
}

// 타일 설정
void map_set_tile(Map* map, int x, int y, TileType tile) {
    if (!map || x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return;
    }
    map->tiles[y][x] = tile;
}

