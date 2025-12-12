#include "map.h"
#include "player.h"
#include <math.h>

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
    map->box_count = 0;
    for (int i = 0; i < MAX_BOXES; i++) {
        map->boxes[i].x = 0;
        map->boxes[i].y = 0;
        map->boxes[i].vy = 0.0f;  // 초기 속도 0
        map->boxes[i].active = false;
    }
    
    // 스위치/도어 초기화
    map->switch_count = 0;
    for (int i = 0; i < MAX_SWITCHES; i++) {
        map->switches[i].x = 0;
        map->switches[i].y = 0;
        map->switches[i].activated = false;
    }
    
    map->door_count = 0;
    for (int i = 0; i < MAX_DOORS; i++) {
        map->doors[i].x = 0;
        map->doors[i].y = 0;
        map->doors[i].is_open = false;
    }
    
    // 이동 발판 초기화
    map->platform_count = 0;
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        map->platforms[i].x = 0.0f;
        map->platforms[i].y = 0.0f;
        map->platforms[i].vx = 0.0f;
        map->platforms[i].vy = 0.0f;
        map->platforms[i].min_x = 0;
        map->platforms[i].max_x = 0;
        map->platforms[i].min_y = 0;
        map->platforms[i].max_y = 0;
        map->platforms[i].vertical = false;
        map->platforms[i].active = false;
    }
    
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
                TileType tile = (TileType)ch;
                map->tiles[y][x] = tile;
                
                // 특수 타일 위치 저장 / 오브젝트 정보 기록
                if (ch == TILE_FIREBOY_START) {
                    map->fireboy_start_x = x;
                    map->fireboy_start_y = y;
                } else if (ch == TILE_WATERGIRL_START) {
                    map->watergirl_start_x = x;
                    map->watergirl_start_y = y;
                } else if (ch == TILE_EXIT) {
                    map->exit_x = x;
                    map->exit_y = y;
                } else if (ch == TILE_BOX) {
                    // 상자 위치 기록
                    if (map->box_count < MAX_BOXES) {
                        int idx = map->box_count++;
                        map->boxes[idx].x = x;
                        map->boxes[idx].y = y;
                        map->boxes[idx].vy = 0.0f;  // 초기 속도 0
                        map->boxes[idx].active = true;
                    }
                } else if (ch == TILE_SWITCH) {
                    // 스위치 위치 기록
                    if (map->switch_count < MAX_SWITCHES) {
                        int idx = map->switch_count++;
                        map->switches[idx].x = x;
                        map->switches[idx].y = y;
                        map->switches[idx].activated = false;
                    }
                } else if (ch == TILE_DOOR) {
                    // 도어 위치 기록
                    if (map->door_count < MAX_DOORS) {
                        int idx = map->door_count++;
                        map->doors[idx].x = x;
                        map->doors[idx].y = y;
                        map->doors[idx].is_open = false; // 처음엔 닫혀있음
                    }
                } else if (ch == TILE_MOVING_PLATFORM) {
                    // 이동 발판 위치 기록 (기본: 위아래 왕복)
                    if (map->platform_count < MAX_PLATFORMS) {
                        int idx = map->platform_count++;
                        map->platforms[idx].x = (float)x;
                        map->platforms[idx].y = (float)y;
                        map->platforms[idx].vx = 0.0f;
                        map->platforms[idx].vy = -2.0f; // 위로 이동 시작
                        map->platforms[idx].vertical = true;
                        // 시작 위치 기준으로 위아래 4칸 범위 내에서 왕복 (맵 안으로 클램프)
                        int range = 4;
                        int min_y = y - range;
                        int max_y = y + range;
                        if (min_y < 0) min_y = 0;
                        if (max_y >= map->height) max_y = map->height - 1;
                        map->platforms[idx].min_y = min_y;
                        map->platforms[idx].max_y = max_y;
                        map->platforms[idx].min_x = x;
                        map->platforms[idx].max_x = x;
                        map->platforms[idx].active = true;
                    }
                    // 발판은 오버레이로만 그려지므로 타일은 빈 칸으로 설정
                    map->tiles[y][x] = TILE_EMPTY;
                }
            } else {
                map->tiles[y][x] = TILE_EMPTY;
            }
        }
        y++;
    }
    
    // 토글 플랫폼(T) 및 수직 벽(V) 파싱
    map->toggle_platform_count = 0;
    map->vertical_wall_count = 0;
    
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            // 토글 플랫폼 'T' 처리
            if (map->tiles[py][px] == 'T') {
                // 연속된 'T'들을 하나의 플랫폼으로 처리
                int platform_width = 1;
                while (px + platform_width < width && map->tiles[py][px + platform_width] == 'T') {
                    platform_width++;
                }
                
                if (map->toggle_platform_count < MAX_PLATFORMS) {
                    int idx = map->toggle_platform_count++;
                    map->toggle_platforms[idx].x = px;
                    map->toggle_platforms[idx].width = platform_width;
                    map->toggle_platforms[idx].y = (float)py;
                    map->toggle_platforms[idx].original_y = py;
                    
                    // 아래로 내려가면서 흰색 바닥(TILE_FLOOR) 찾기
                    int target_y = py;
                    for (int search_y = py + 1; search_y < height; search_y++) {
                        // 바닥/벽/물/불이 나오면 그 바로 위까지
                        TileType below = map->tiles[search_y][px];
                        if (below == TILE_FLOOR || below == TILE_WALL || 
                            below == TILE_WATER_TERRAIN || below == TILE_FIRE_TERRAIN) {
                            target_y = search_y - 1;
                            break;
                        }
                    }
                    
                    map->toggle_platforms[idx].target_y = target_y;
                    map->toggle_platforms[idx].moving_down = false;
                    map->toggle_platforms[idx].target_is_down = false;
                    
                    // 가장 가까운 스위치 찾기
                    int closest_switch = -1;
                    float min_dist = 999999.0f;
                    for (int si = 0; si < map->switch_count; si++) {
                        int sx = map->switches[si].x;
                        int sy = map->switches[si].y;
                        float dist = sqrtf((float)((sx - px) * (sx - px) + (sy - py) * (sy - py)));
                        if (dist < min_dist) {
                            min_dist = dist;
                            closest_switch = si;
                        }
                    }
                    map->toggle_platforms[idx].linked_switch = closest_switch;
                    
                    // 'T' 타일들을 EMPTY로 변경 (오버레이로만 렌더링)
                    for (int w = 0; w < platform_width; w++) {
                        map->tiles[py][px + w] = TILE_EMPTY;
                    }
                }
                
                px += platform_width - 1; // 이미 처리한 'T'들 건너뛰기
            }
            // 수직 벽 'V' 처리
            else if (map->tiles[py][px] == 'V') {
                if (map->vertical_wall_count < MAX_PLATFORMS) {
                    int idx = map->vertical_wall_count++;
                    map->vertical_walls[idx].x = px;
                    map->vertical_walls[idx].y = (float)py;
                    map->vertical_walls[idx].original_y = py;
                    
                    // 같은 x 열에서 'v' 찾기 (타겟 위치)
                    int target_y = py;
                    for (int search_y = py - 1; search_y >= 0; search_y--) {
                        if (map->tiles[search_y][px] == 'v') {
                            target_y = search_y;
                            break;
                        }
                    }
                    
                    map->vertical_walls[idx].target_y = target_y;
                    map->vertical_walls[idx].is_up = false;
                    
                    // 가장 가까운 스위치 찾기
                    int closest_switch = -1;
                    float min_dist = 999999.0f;
                    for (int si = 0; si < map->switch_count; si++) {
                        int sx = map->switches[si].x;
                        int sy = map->switches[si].y;
                        float dist = sqrtf((float)((sx - px) * (sx - px) + (sy - py) * (sy - py)));
                        if (dist < min_dist) {
                            min_dist = dist;
                            closest_switch = si;
                        }
                    }
                    map->vertical_walls[idx].linked_switch = closest_switch;
                    
                    // 'V' 타일을 EMPTY로 변경
                    map->tiles[py][px] = TILE_EMPTY;
                }
            }
            // 't'와 'v'도 EMPTY로 변경
            else if (map->tiles[py][px] == 't' || map->tiles[py][px] == 'v') {
                map->tiles[py][px] = TILE_EMPTY;
            }
        }
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
        case TILE_FIRE_GEM:
        case TILE_WATER_GEM:
        case TILE_FIREBOY_START:  // 시작 위치도 이동 가능
        case TILE_WATERGIRL_START: // 시작 위치도 이동 가능
            return true;
        case TILE_FIRE_TERRAIN:
            return is_fireboy; // Fireboy만 통과 가능
        case TILE_WATER_TERRAIN:
            return !is_fireboy; // Watergirl만 통과 가능
        case TILE_DOOR:
            // 도어는 열려있으면 TILE_EMPTY로 바뀌어서 여기까지 오지 않음
            // 닫혀있으면 TILE_DOOR로 유지되어 막힘
            return false; // 기본적으로는 막힘
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

// 상자 관련 헬퍼 구현
int map_get_box_count(const Map* map) {
    if (!map) return 0;
    return map->box_count;
}

int map_get_box_x(const Map* map, int index) {
    if (!map || index < 0 || index >= map->box_count) return -1;
    return map->boxes[index].x;
}

int map_get_box_y(const Map* map, int index) {
    if (!map || index < 0 || index >= map->box_count) return -1;
    return map->boxes[index].y;
}

// (x, y)에 있는 상자의 인덱스를 찾기 (없으면 -1)
int map_find_box(const Map* map, int x, int y) {
    if (!map) return -1;
    for (int i = 0; i < map->box_count; i++) {
        if (map->boxes[i].active && map->boxes[i].x == x && map->boxes[i].y == y) {
            return i;
        }
    }
    return -1;
}

// 상자를 새 위치로 이동 (타일 배열과 boxes 배열 동기화)
bool map_move_box(Map* map, int index, int new_x, int new_y) {
    if (!map || index < 0 || index >= map->box_count) return false;
    if (new_x < 0 || new_x >= map->width || new_y < 0 || new_y >= map->height) return false;
    if (!map->boxes[index].active) return false;

    int old_x = map->boxes[index].x;
    int old_y = map->boxes[index].y;

    // 새 위치가 비어 있는지 확인
    TileType target = map_get_tile(map, new_x, new_y);
    // 공백이거나 스위치 위로는 이동 가능하게 허용
    // (스위치 타일은 boxes/switches 배열로 따로 추적하므로, 타일 배열에서는 BOX로 덮어써도 됨)
    if (target != TILE_EMPTY && target != TILE_SWITCH) {
        return false;
    }

    // 타일 갱신
    map_set_tile(map, old_x, old_y, TILE_EMPTY);
    map_set_tile(map, new_x, new_y, TILE_BOX);

    // 상자 좌표 갱신
    map->boxes[index].x = new_x;
    map->boxes[index].y = new_y;

    return true;
}

// 상자 중력/물리 업데이트 (매 프레임 호출)
void map_update_boxes(Map* map, float delta_time) {
    if (!map) return;
    
    // 상자 물리 상수 (플레이어와 동일)
    const float BOX_GRAVITY = 30.0f;         // 중력 가속도
    const float BOX_MAX_FALL_SPEED = 20.0f;  // 최대 낙하 속도
    
    // 각 상자에 대해 중력 적용
    for (int i = 0; i < map->box_count; i++) {
        if (!map->boxes[i].active) continue;
        
        int box_x = map->boxes[i].x;
        int box_y = map->boxes[i].y;
        
        // 상자 아래 타일 확인 (지상 체크)
        bool is_on_ground = false;
        if (box_y + 1 >= map->height) {
            is_on_ground = true; // 맵 밖 = 바닥에 있음
        } else {
            TileType tile_below = map_get_tile(map, box_x, box_y + 1);
            // 벽, 바닥, 스위치, 다른 상자는 지면으로 간주
            if (tile_below == TILE_WALL || tile_below == TILE_FLOOR ||
                tile_below == TILE_SWITCH || tile_below == TILE_BOX) {
                is_on_ground = true;
            }
        }
        
        // 중력 적용
        if (!is_on_ground) {
            // 공중에 있으면 중력으로 속도 증가
            map->boxes[i].vy += BOX_GRAVITY * delta_time;
            // 최대 낙하 속도 제한
            if (map->boxes[i].vy > BOX_MAX_FALL_SPEED) {
                map->boxes[i].vy = BOX_MAX_FALL_SPEED;
            }
        } else {
            // 지상에 있으면 수직 속도 0으로
            if (map->boxes[i].vy > 0) {
                map->boxes[i].vy = 0;
            }
        }
        
        // 속도 누적 (플레이어와 동일한 방식)
        static float vy_accumulator[MAX_BOXES] = {0.0f}; // 각 상자별 속도 누적
        
        vy_accumulator[i] += map->boxes[i].vy * delta_time;
        
        // 누적된 속도가 1타일 이상이면 이동
        while (fabsf(vy_accumulator[i]) >= 1.0f) {
            if (vy_accumulator[i] > 0.0f) {
                // 아래로 이동 (낙하)
                int new_y = box_y + 1;
                if (new_y >= map->height) {
                    // 맵 밖으로 나가면 멈춤
                    map->boxes[i].vy = 0;
                    vy_accumulator[i] = 0.0f;
                    break;
                }
                
                TileType tile_below = map_get_tile(map, box_x, new_y);
                
                // 목적지가 공백이거나 스위치면 계속 낙하
                if (tile_below == TILE_EMPTY || tile_below == TILE_SWITCH) {
                    if (map_move_box(map, i, box_x, new_y)) {
                        box_y = new_y; // 위치 업데이트
                        vy_accumulator[i] -= 1.0f;
                        continue; // 계속 낙하
                    } else {
                        // 이동 실패 시 멈춤
                        map->boxes[i].vy = 0;
                        vy_accumulator[i] = 0.0f;
                        break;
                    }
                }
                
                // 벽이나 바닥이 바로 아래에 있으면 착지
                if (tile_below == TILE_WALL || tile_below == TILE_FLOOR) {
                    map->boxes[i].vy = 0;
                    vy_accumulator[i] = 0.0f;
                    break;
                }
                
                // 다른 상자가 아래에 있으면 착지
                if (tile_below == TILE_BOX) {
                    map->boxes[i].vy = 0;
                    vy_accumulator[i] = 0.0f;
                    break;
                }
                
                // 기타 경우도 멈춤
                map->boxes[i].vy = 0;
                vy_accumulator[i] = 0.0f;
                break;
            }
        }
    }
}

// 상자 상태 리셋 (맵 리셋 시 호출)
void map_reset_boxes(Map* map) {
    if (!map) return;
    
    // 모든 상자의 속도를 0으로 초기화
    for (int i = 0; i < map->box_count; i++) {
        if (map->boxes[i].active) {
            map->boxes[i].vy = 0.0f;
        }
    }
    
    // static 변수 리셋을 위해 중력 업데이트를 한 번 호출 (누적값 초기화 효과)
    // 실제로는 다음 프레임부터 중력이 적용됨
}

// 스위치/도어 관련 헬퍼 구현
int map_get_switch_count(const Map* map) {
    if (!map) return 0;
    return map->switch_count;
}

int map_get_switch_x(const Map* map, int index) {
    if (!map || index < 0 || index >= map->switch_count) return -1;
    return map->switches[index].x;
}

int map_get_switch_y(const Map* map, int index) {
    if (!map || index < 0 || index >= map->switch_count) return -1;
    return map->switches[index].y;
}

bool map_is_switch_activated(const Map* map, int index) {
    if (!map || index < 0 || index >= map->switch_count) return false;
    return map->switches[index].activated;
}

int map_find_switch(const Map* map, int x, int y) {
    if (!map) return -1;
    for (int i = 0; i < map->switch_count; i++) {
        if (map->switches[i].x == x && map->switches[i].y == y) {
            return i;
        }
    }
    return -1;
}

// 스위치 활성화 체크 (플레이어나 상자가 스위치 위에 있는지 확인)
void map_update_switches(Map* map, int fireboy_x, int fireboy_y, int watergirl_x, int watergirl_y) {
    if (!map) return;
    
    for (int i = 0; i < map->switch_count; i++) {
        int switch_x = map->switches[i].x;
        int switch_y = map->switches[i].y;
        
        // 플레이어가 스위치 위에 있는지 확인
        bool player_on_switch = (fireboy_x == switch_x && fireboy_y == switch_y) ||
                                (watergirl_x == switch_x && watergirl_y == switch_y);
        
        // 상자가 스위치 위에 있는지 확인
        bool box_on_switch = false;
        for (int j = 0; j < map->box_count; j++) {
            if (map->boxes[j].active && 
                map->boxes[j].x == switch_x && map->boxes[j].y == switch_y) {
                box_on_switch = true;
                break;
            }
        }
        
        // 플레이어나 상자가 스위치 위에 있으면 활성화
        map->switches[i].activated = (player_on_switch || box_on_switch);
    }
}

// 도어 열림/닫힘 업데이트 (스위치 상태에 따라)
void map_update_doors(Map* map) {
    if (!map) return;
    
    // 모든 스위치가 활성화되어 있는지 확인 (간단한 버전: 하나라도 활성화되면 모든 도어 열림)
    bool any_switch_active = false;
    for (int i = 0; i < map->switch_count; i++) {
        if (map->switches[i].activated) {
            any_switch_active = true;
            break;
        }
    }
    
    // 모든 도어 상태 업데이트
    for (int i = 0; i < map->door_count; i++) {
        map->doors[i].is_open = any_switch_active;
        
        // 도어가 열려있으면 타일을 EMPTY로 변경 (완전히 사라짐)
        // 닫혀있으면 TILE_DOOR로 유지 (벽처럼 막힘)
        if (map->doors[i].is_open) {
            map_set_tile(map, map->doors[i].x, map->doors[i].y, TILE_EMPTY);
        } else {
            map_set_tile(map, map->doors[i].x, map->doors[i].y, TILE_DOOR);
        }
    }
}

// 이동 발판 업데이트 (좌우/위아래 왕복 + 위에 있는 플레이어 함께 이동)
void map_update_platforms(Map* map, float delta_time, struct Player* fireboy, struct Player* watergirl) {
    if (!map) return;
    
    for (int i = 0; i < map->platform_count; i++) {
        if (!map->platforms[i].active) continue;
        
        float old_fx = map->platforms[i].x;
        float old_fy = map->platforms[i].y;
        int old_x = (int)roundf(old_fx);
        int old_y = (int)roundf(old_fy);
        
        float new_fx = old_fx;
        float new_fy = old_fy;
        
        if (map->platforms[i].vertical) {
            // 위아래 이동
            new_fy = old_fy + map->platforms[i].vy * delta_time;
            
            // 범위 체크
            if (new_fy < map->platforms[i].min_y) {
                new_fy = (float)map->platforms[i].min_y;
                map->platforms[i].vy = fabsf(map->platforms[i].vy);
            } else if (new_fy > map->platforms[i].max_y) {
                new_fy = (float)map->platforms[i].max_y;
                map->platforms[i].vy = -fabsf(map->platforms[i].vy);
            }
            
            // 벽/바닥 충돌 체크
            int check_y = (int)roundf(new_fy);
            if (check_y >= 0 && check_y < map->height) {
                TileType tile_at_target = map_get_tile(map, old_x, check_y);
                if (tile_at_target == TILE_WALL || tile_at_target == TILE_FLOOR) {
                    // 벽이나 바닥에 부딪히면 방향 반전하고 이전 위치 유지
                    map->platforms[i].vy = -map->platforms[i].vy;
                    new_fy = old_fy;
                }
            }
        } else {
            // 좌우 이동
            new_fx = old_fx + map->platforms[i].vx * delta_time;
            
            // 범위 체크
            if (new_fx < map->platforms[i].min_x) {
                new_fx = (float)map->platforms[i].min_x;
                map->platforms[i].vx = fabsf(map->platforms[i].vx);
            } else if (new_fx > map->platforms[i].max_x) {
                new_fx = (float)map->platforms[i].max_x;
                map->platforms[i].vx = -fabsf(map->platforms[i].vx);
            }
            
            // 벽/바닥 충돌 체크
            int check_x = (int)roundf(new_fx);
            if (check_x >= 0 && check_x < map->width) {
                TileType tile_at_target = map_get_tile(map, check_x, old_y);
                if (tile_at_target == TILE_WALL || tile_at_target == TILE_FLOOR) {
                    // 벽이나 바닥에 부딪히면 방향 반전하고 이전 위치 유지
                    map->platforms[i].vx = -map->platforms[i].vx;
                    new_fx = old_fx;
                }
            }
        }
        
        int new_x = (int)roundf(new_fx);
        int new_y = (int)roundf(new_fy);
        int delta_x = new_x - old_x;
        int delta_y = new_y - old_y;
        
        // 플레이어를 발판과 함께 이동 (발판 위에 있는 경우)
        Player* players[2] = { fireboy, watergirl };
        for (int p = 0; p < 2; p++) {
            Player* pl = players[p];
            if (!pl) continue;
            
            // 플레이어가 발판 위에 있는지 체크 (발판의 이전 위치 기준)
            bool on_platform = false;
            if (pl->x == old_x && pl->y == old_y - 1) {
                // 발판 바로 위에 서 있음
                on_platform = true;
            }
            
            // 발판이 움직이면 플레이어도 함께 이동
            if (on_platform && (delta_x != 0 || delta_y != 0)) {
                int target_x = pl->x + delta_x;
                int target_y = pl->y + delta_y;
                if (target_x >= 0 && target_x < map->width &&
                    target_y >= 0 && target_y < map->height) {
                    TileType t = map_get_tile(map, target_x, target_y);
                    if (t != TILE_WALL && t != TILE_FLOOR && t != TILE_DOOR) {
                        pl->x = target_x;
                        pl->y = target_y;
                        // 발판과 함께 이동할 때 수직 속도를 0으로 (중력 무효화)
                        pl->vy = 0.0f;
                        pl->is_on_ground = true;
                    }
                }
            }
        }
        
        // 타일 배열은 건드리지 않음! 렌더러에서 오버레이로 그림
        map->platforms[i].x = new_fx;
        map->platforms[i].y = new_fy;
    }
}

// 토글 플랫폼 업데이트
void map_update_toggle_platforms(Map* map, float delta_time) {
    if (!map) return;
    
    const float platform_speed = 3.0f; // 초당 3타일 이동
    
    for (int i = 0; i < map->toggle_platform_count; i++) {
        // 연결된 스위치가 있는지 확인
        int switch_idx = map->toggle_platforms[i].linked_switch;
        if (switch_idx < 0 || switch_idx >= map->switch_count) {
            continue;
        }
        
        // 스위치가 활성화되면 목표 위치 설정
        bool switch_on = map->switches[switch_idx].activated;
        bool should_be_down = switch_on;
        
        // 목표 상태가 바뀌면 이동 시작
        if (should_be_down != map->toggle_platforms[i].target_is_down) {
            map->toggle_platforms[i].target_is_down = should_be_down;
        }
        
        // 현재 목표 위치
        float target = should_be_down ? 
            (float)map->toggle_platforms[i].target_y : 
            (float)map->toggle_platforms[i].original_y;
        
        // 현재 위치
        float current = map->toggle_platforms[i].y;
        
        // 목표에 도달하지 않았으면 이동
        if (fabsf(current - target) > 0.1f) {
            if (current < target) {
                // 아래로 이동
                map->toggle_platforms[i].y += platform_speed * delta_time;
                if (map->toggle_platforms[i].y > target) {
                    map->toggle_platforms[i].y = target;
                }
            } else {
                // 위로 이동
                map->toggle_platforms[i].y -= platform_speed * delta_time;
                if (map->toggle_platforms[i].y < target) {
                    map->toggle_platforms[i].y = target;
                }
            }
        }
    }
}

// 수직 벽 업데이트
void map_update_vertical_walls(Map* map, float delta_time) {
    if (!map) return;
    (void)delta_time; // 경고 방지 (현재는 즉시 사라지므로 사용 안 함)
    
    for (int i = 0; i < map->vertical_wall_count; i++) {
        // 연결된 스위치가 있는지 확인
        int switch_idx = map->vertical_walls[i].linked_switch;
        if (switch_idx < 0 || switch_idx >= map->switch_count) {
            continue;
        }
        
        // 스위치에 박스가 있으면 V 벽이 사라짐
        bool box_on_switch = false;
        int switch_x = map->switches[switch_idx].x;
        int switch_y = map->switches[switch_idx].y;
        for (int j = 0; j < map->box_count; j++) {
            if (map->boxes[j].active && 
                map->boxes[j].x == switch_x && map->boxes[j].y == switch_y) {
                box_on_switch = true;
                break;
            }
        }
        bool should_hide = box_on_switch;
        
        // V 벽이 사라져야 하는지 확인
        int wall_x = map->vertical_walls[i].x;
        int orig_y = map->vertical_walls[i].original_y;
        int target_y = map->vertical_walls[i].target_y;
        
        // 벽이 있는 전체 범위 (original_y ~ target_y)
        int min_y = (orig_y < target_y) ? orig_y : target_y;
        int max_y = (orig_y > target_y) ? orig_y : target_y;
        
        if (should_hide) {
            // 박스가 스위치 위에 있으면 V 벽 전체를 EMPTY로 변경 (사라짐)
            for (int y = min_y; y <= max_y; y++) {
                if (y >= 0 && y < map->height && wall_x >= 0 && wall_x < map->width) {
                    map->tiles[y][wall_x] = TILE_EMPTY;
                }
            }
        } else {
            // 박스가 없으면 V 벽을 다시 표시 (original_y부터 target_y까지)
            for (int y = orig_y; y <= target_y; y++) {
                if (y >= 0 && y < map->height && wall_x >= 0 && wall_x < map->width) {
                    map->tiles[y][wall_x] = TILE_VERTICAL_WALL;
                }
            }
        }
    }
}

