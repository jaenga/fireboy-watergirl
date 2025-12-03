#include "player.h"
#include "console.h"
#include <math.h>
#include <stdio.h>

// 물리 상수
#define MOVE_SPEED 3.0f          // 좌우 이동 속도 (타일/초) - 낮을수록 느리게 이동
#define GRAVITY 30.0f             // 중력 가속도 (타일/초²)
#define JUMP_POWER 18.0f           // 점프 힘 (타일/초)
#define MAX_FALL_SPEED 20.0f      // 최대 낙하 속도 (타일/초)
#define GROUND_CHECK_OFFSET 0.1f  // 지상 체크 오프셋

// 플레이어 초기화
void player_init(Player* player, PlayerType type, int start_x, int start_y) {
    if (!player) return;
    
    player->type = type;
    player->x = start_x;
    player->y = start_y;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->state = PLAYER_STATE_ALIVE;
    player->is_on_ground = true; // 4단계에서는 항상 지상에 있다고 가정
}

// 바닥에 있는지 확인
static bool check_ground(const Map* map, int x, int y, bool is_fireboy) {
    if (y + 1 >= map->height) {
        return true; // 맵 밖 = 바닥에 있음 (낙하 방지)
    }
    
    // 현재 위치의 타일 확인
    TileType current_tile = map_get_tile(map, x, y);
    
    // 현재 위치가 바닥 타일이면, 바로 아래가 공백이어야 바닥 위에 서 있는 것
    // (바닥 타일 위에 서 있는 경우)
    if (current_tile == TILE_FLOOR) {
        TileType tile_below = map_get_tile(map, x, y + 1);
        // 바로 아래가 공백이면 바닥 위에 서 있음
        if (tile_below == TILE_EMPTY) {
            return true; // 바닥 위에 서 있음
        }
        // 바로 아래가 벽이나 다른 바닥이면 그 위에 서 있는 것
        if (tile_below == TILE_WALL || tile_below == TILE_FLOOR) {
            return true;
        }
    }
    
    // 바로 아래 타일 확인
    TileType tile_below = map_get_tile(map, x, y + 1);
    
    // 바닥, 벽은 지면으로 간주
    if (tile_below == TILE_FLOOR || tile_below == TILE_WALL) {
        return true;
    }
    
    // 시작 위치도 지면으로 간주 (시작 위치 타일 자체가 바닥 역할)
    if (tile_below == TILE_FIREBOY_START || tile_below == TILE_WATERGIRL_START) {
        return true;
    }
    
    // 속성 지형 체크
    if (tile_below == TILE_FIRE_TERRAIN && is_fireboy) {
        return true;
    }
    if (tile_below == TILE_WATER_TERRAIN && !is_fireboy) {
        return true;
    }
    
    // 공백이면 공중
    if (tile_below == TILE_EMPTY) {
        return false;
    }
    
    // 기타 타일들 (상자, 스위치 등)도 지면으로 간주
    return true;
}

// 천장에 닿았는지 확인
static bool check_ceiling(const Map* map, int x, int y, bool is_fireboy) {
    (void)is_fireboy; // 나중에 속성 지형 고려 시 사용
    if (y - 1 < 0) {
        return false; // 맵 밖
    }
    
    TileType tile_above = map_get_tile(map, x, y - 1);
    
    // 벽이나 바닥이 위에 있으면 천장 충돌
    if (tile_above == TILE_WALL || tile_above == TILE_FLOOR) {
        return true;
    }
    
    return false;
}

// 플레이어 업데이트 (입력 처리, 물리, 이동)
void player_update(Player* player, const Map* map, bool left_pressed, bool right_pressed, bool jump_pressed, float delta_time) {
    if (!player || !map || player->state == PLAYER_STATE_DEAD) {
        return;
    }
    
    bool is_fireboy = (player->type == PLAYER_FIREBOY);
    int player_idx = is_fireboy ? 0 : 1; // 플레이어 인덱스
    
    // 지상 상태 확인
    player->is_on_ground = check_ground(map, player->x, player->y, is_fireboy);
    
    // 점프 처리 (지상에 있을 때만, 한 번만 실행되도록)
    static bool last_jump_state[2] = {false, false}; // [0]=fireboy, [1]=watergirl
    bool jump_just_pressed = jump_pressed && !last_jump_state[player_idx];
    last_jump_state[player_idx] = jump_pressed;
    
    if (jump_just_pressed && player->is_on_ground && player->vy >= 0) {
        player->vy = -JUMP_POWER; // 위로 점프
    }
    
    // 중력 적용
    if (!player->is_on_ground) {
        player->vy += GRAVITY * delta_time;
        // 최대 낙하 속도 제한
        if (player->vy > MAX_FALL_SPEED) {
            player->vy = MAX_FALL_SPEED;
        }
    } else {
        // 지상에 있으면 수직 속도 0으로 (바닥에 착지)
        if (player->vy > 0) {
            player->vy = 0;
        }
    }
    
    // 좌우 이동 처리 (속도 기반, 점프 중에도 작동)
    static float vx_accumulator[2] = {0.0f, 0.0f}; // [0]=fireboy, [1]=watergirl
    // player_idx는 위에서 이미 정의됨
    
    // 공중 이동 속도 (지상보다 약간 느림, 선택사항)
    float air_control = 1.0f; // 공중에서도 100% 제어 가능 (0.8f로 하면 80% 제어)
    float current_move_speed = player->is_on_ground ? MOVE_SPEED : (MOVE_SPEED * air_control);
    
    // 입력에 따라 수평 속도 설정 (지상/공중 모두 작동)
    if (left_pressed) {
        vx_accumulator[player_idx] -= current_move_speed * delta_time;
    } else if (right_pressed) {
        vx_accumulator[player_idx] += current_move_speed * delta_time;
    }
    
    // 반응 속도: 0.3f 임계값으로 빠른 반응 (낮을수록 빠른 반응)
    // 이동 속도: MOVE_SPEED로 제어 (낮을수록 느린 이동)
    float move_threshold = 0.3f; // 반응 속도 임계값 (빠른 반응 유지)
    float move_step = 0.5f; // 이동 단위 (이동 속도 제어)
    
    while (fabsf(vx_accumulator[player_idx]) >= move_threshold) {
        if (vx_accumulator[player_idx] > 0.0f) {
            // 오른쪽 이동
            int new_x = player->x + 1;
            if (new_x >= 0 && new_x < map->width) {
                // 목적지 위치의 타일 확인
                TileType target_tile = map_get_tile(map, new_x, player->y);
                
                // 벽이면 이동 불가
                if (target_tile == TILE_WALL) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 바닥 타일은 지상에 있을 때만 막힘 (공중에서는 통과 가능)
                if (target_tile == TILE_FLOOR && player->is_on_ground) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 공백이나 다른 통과 가능한 타일이면 이동
                // (map_is_walkable 체크는 이미 위에서 벽/바닥 체크를 했으므로 생략)
                player->x = new_x;
                vx_accumulator[player_idx] -= move_step;
            } else {
                vx_accumulator[player_idx] = 0.0f;
                break;
            }
        } else if (vx_accumulator[player_idx] < 0.0f) {
            // 왼쪽 이동
            int new_x = player->x - 1;
            if (new_x >= 0 && new_x < map->width) {
                // 목적지 위치의 타일 확인
                TileType target_tile = map_get_tile(map, new_x, player->y);
                
                // 벽이면 이동 불가
                if (target_tile == TILE_WALL) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 바닥 타일은 지상에 있을 때만 막힘 (공중에서는 통과 가능)
                if (target_tile == TILE_FLOOR && player->is_on_ground) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 공백이나 다른 통과 가능한 타일이면 이동
                // (map_is_walkable 체크는 이미 위에서 벽/바닥 체크를 했으므로 생략)
                player->x = new_x;
                vx_accumulator[player_idx] += move_step; // 음수이므로 더하기
            } else {
                vx_accumulator[player_idx] = 0.0f;
                break;
            }
        }
    }
    
    // 입력이 없으면 속도 감소 (마찰 - 지상에서만, 공중에서는 관성 유지)
    if (!left_pressed && !right_pressed) {
        if (player->is_on_ground) {
            // 지상에서는 마찰로 속도 감소
            if (fabsf(vx_accumulator[player_idx]) > 0.1f) {
                vx_accumulator[player_idx] *= 0.8f;
            } else {
                vx_accumulator[player_idx] = 0.0f;
            }
        }
        // 공중에서는 관성 유지 (속도 감소 안 함)
    }
    
    // 수직 이동 적용 (타일 단위로 처리)
    // 속도를 누적해서 처리하기 위해 정적 변수 사용
    static float vy_accumulator[2] = {0.0f, 0.0f}; // [0]=fireboy, [1]=watergirl
    // player_idx는 위에서 이미 정의됨
    
    // 속도 누적
    vy_accumulator[player_idx] += player->vy * delta_time;
    
    // 누적된 속도가 1타일 이상이면 이동
    while (fabsf(vy_accumulator[player_idx]) >= 1.0f) {
        if (vy_accumulator[player_idx] > 0.0f) {
            // 아래로 이동 (낙하)
            int new_y = player->y + 1;
            if (new_y >= map->height) {
                // 맵 밖으로 나가면 멈춤
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            // 아래로 이동할 때는 목적지 위치의 타일과 그 아래 타일을 확인
            TileType current_tile = map_get_tile(map, player->x, new_y);
            TileType tile_below = (new_y + 1 < map->height) ? map_get_tile(map, player->x, new_y + 1) : TILE_EMPTY;
            
            // 목적지가 공백이면 계속 낙하 (바닥 타일 위로 내려가는 경우도 포함)
            if (current_tile == TILE_EMPTY) {
                player->y = new_y;
                player->is_on_ground = false;
                vy_accumulator[player_idx] -= 1.0f;
                continue; // 계속 낙하
            }
            
            // 목적지가 바닥 타일이고, 바로 아래가 공백이면 바닥 위에 착지
            if (current_tile == TILE_FLOOR && tile_below == TILE_EMPTY) {
                // 바닥 타일 위에 착지
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            // 벽이나 바닥 타일이 바로 아래에 있으면 착지
            if (tile_below == TILE_WALL || tile_below == TILE_FLOOR) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            // 시작 위치 타일도 착지 가능
            if (tile_below == TILE_FIREBOY_START || tile_below == TILE_WATERGIRL_START) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            // 속성 지형 체크
            if ((tile_below == TILE_FIRE_TERRAIN && is_fireboy) ||
                (tile_below == TILE_WATER_TERRAIN && !is_fireboy)) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            // 기타 경우도 계속 낙하 (바닥 타일을 통과할 수 있도록)
            player->y = new_y;
            player->is_on_ground = false;
            vy_accumulator[player_idx] -= 1.0f;
        } else if (vy_accumulator[player_idx] < 0.0f) {
            // 위로 이동 (점프)
            int new_y = player->y - 1;
            if (new_y < 0) {
                // 맵 밖으로 나가면 멈춤
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            bool ceiling = check_ceiling(map, player->x, new_y, is_fireboy);
            if (ceiling) {
                // 천장에 닿으면 속도 0으로
                player->y = new_y + 1;
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            } else {
                // 천장이 아니면 계속 상승
                player->y = new_y;
                vy_accumulator[player_idx] += 1.0f; // 음수이므로 더하기
            }
        }
    }
    
    // 최종 지상 상태 확인
    player->is_on_ground = check_ground(map, player->x, player->y, is_fireboy);
}

// 플레이어 리셋 (시작 위치로 복귀)
void player_reset(Player* player, int start_x, int start_y) {
    if (!player) return;
    
    player->x = start_x;
    player->y = start_y;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->state = PLAYER_STATE_ALIVE;
    player->is_on_ground = true;
}

// 충돌 체크 (현재는 사용 안 함, 나중을 위해)
bool player_check_collision(const Player* player, const Map* map, int new_x, int new_y) {
    if (!player || !map) return false;
    
    // 맵 경계 체크
    if (new_x < 0 || new_x >= map->width || new_y < 0 || new_y >= map->height) {
        return true; // 충돌
    }
    
    // 벽 충돌 체크
    bool is_fireboy = (player->type == PLAYER_FIREBOY);
    if (!map_is_walkable(map, new_x, new_y, is_fireboy)) {
        return true; // 충돌
    }
    
    return false; // 충돌 없음
}

