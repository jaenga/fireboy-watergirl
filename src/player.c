#include "player.h"
#include "console.h"
#include <math.h>
#include <stdio.h>

// 물리 상수
#define MOVE_SPEED 3.0f
#define GRAVITY 30.0f
#define JUMP_POWER 18.0f
#define MAX_FALL_SPEED 20.0f
#define GROUND_CHECK_OFFSET 0.1f

// 보석 카운트
static int g_fire_gem_count = 0;
static int g_water_gem_count = 0;

int player_get_fire_gem_count(void) {
    return g_fire_gem_count;
}

int player_get_water_gem_count(void) {
    return g_water_gem_count;
}

int player_get_total_gem_count(void) {
    return g_fire_gem_count + g_water_gem_count;
}

void player_reset_gem_count(void) {
    g_fire_gem_count = 0;
    g_water_gem_count = 0;
}

// 플레이어 초기화
void player_init(Player* player, PlayerType type, int start_x, int start_y) {
    if (!player) return;
    
    player->type = type;
    player->x = start_x;
    player->y = start_y;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->state = PLAYER_STATE_ALIVE;
    player->is_on_ground = true;
}

// 지면 체크
static bool check_ground(const Map* map, int x, int y, bool is_fireboy) {
    if (y + 1 >= map->height) {
        return true;
    }
    
    TileType current_tile = map_get_tile(map, x, y);
    
    if (current_tile == TILE_FLOOR) {
        TileType tile_below = map_get_tile(map, x, y + 1);
        if (tile_below == TILE_EMPTY || tile_below == TILE_WALL || tile_below == TILE_FLOOR) {
            return true;
        }
    }
    
    TileType tile_below = map_get_tile(map, x, y + 1);
    
    if (tile_below == TILE_FLOOR || tile_below == TILE_WALL) {
        return true;
    }
    
    if (tile_below == TILE_FIREBOY_START || tile_below == TILE_WATERGIRL_START) {
        return true;
    }
    
    if (tile_below == TILE_FIRE_TERRAIN && is_fireboy) {
        return true;
    }
    if (tile_below == TILE_WATER_TERRAIN && !is_fireboy) {
        return true;
    }
    
    // 이동 발판 체크
    if (map) {
        for (int i = 0; i < map->platform_count; i++) {
            if (!map->platforms[i].active) continue;
            int px = (int)roundf(map->platforms[i].x);
            int py = (int)roundf(map->platforms[i].y);
            if (px == x && py == y + 1) {
                return true;
            }
        }
    }
    
    if (tile_below == TILE_EMPTY) {
        return false;
    }
    
    return true;
}

// 플레이어 업데이트
void player_update(Player* player, Map* map, bool left_pressed, bool right_pressed, bool jump_pressed, float delta_time) {
    if (!player || !map || player->state == PLAYER_STATE_DEAD) {
        return;
    }
    
    bool is_fireboy = (player->type == PLAYER_FIREBOY);
    int player_idx = is_fireboy ? 0 : 1;
    
    TileType current_tile = map_get_tile(map, player->x, player->y);
    
    TileType tile_below = TILE_EMPTY;
    if (player->y + 1 < map->height) {
        tile_below = map_get_tile(map, player->x, player->y + 1);
    }
    
    // 속성 지형 사망 체크
    if (is_fireboy && (current_tile == TILE_WATER_TERRAIN || tile_below == TILE_WATER_TERRAIN)) {
        player->state = PLAYER_STATE_DEAD;
        return;
    }
    
    if (!is_fireboy && (current_tile == TILE_FIRE_TERRAIN || tile_below == TILE_FIRE_TERRAIN)) {
        player->state = PLAYER_STATE_DEAD;
        return;
    }
    
    // 보석 수집
    if (is_fireboy && current_tile == TILE_FIRE_GEM) {
        map_set_tile(map, player->x, player->y, TILE_EMPTY);
        g_fire_gem_count++;
    } else if (!is_fireboy && current_tile == TILE_WATER_GEM) {
        map_set_tile(map, player->x, player->y, TILE_EMPTY);
        g_water_gem_count++;
    }
    
    player->is_on_ground = check_ground(map, player->x, player->y, is_fireboy);
    
    // 점프 처리
    static bool last_jump_state[2] = {false, false};
    bool jump_just_pressed = jump_pressed && !last_jump_state[player_idx];
    last_jump_state[player_idx] = jump_pressed;
    
    if (jump_just_pressed && player->is_on_ground && player->vy >= 0) {
        player->vy = -JUMP_POWER;
    }
    
    // 중력
    if (!player->is_on_ground) {
        player->vy += GRAVITY * delta_time;
        if (player->vy > MAX_FALL_SPEED) {
            player->vy = MAX_FALL_SPEED;
        }
    } else {
        if (player->vy > 0) {
            player->vy = 0;
        }
    }
    
    // 좌우 이동
    static float vx_accumulator[2] = {0.0f, 0.0f};
    
    float air_control = 1.0f;
    float current_move_speed = player->is_on_ground ? MOVE_SPEED : (MOVE_SPEED * air_control);
    
    if (left_pressed) {
        vx_accumulator[player_idx] -= current_move_speed * delta_time;
    } else if (right_pressed) {
        vx_accumulator[player_idx] += current_move_speed * delta_time;
    }
    
    float move_threshold = 0.3f;
    float move_step = 0.5f;
    
    while (fabsf(vx_accumulator[player_idx]) >= move_threshold) {
        if (vx_accumulator[player_idx] > 0.0f) {
            int new_x = player->x + 1;
            if (new_x >= 0 && new_x < map->width) {
                TileType target_tile = map_get_tile(map, new_x, player->y);
                
                if (target_tile == TILE_WALL) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }

                // 상자 밀기
                if (target_tile == TILE_BOX) {
                    int box_index = map_find_box(map, new_x, player->y);
                    int box_new_x = new_x + 1;
                    if (box_index >= 0 && box_new_x >= 0 && box_new_x < map->width) {
                        TileType box_target = map_get_tile(map, box_new_x, player->y);
                        if ((box_target == TILE_EMPTY || box_target == TILE_SWITCH) &&
                            map_move_box(map, box_index, box_new_x, player->y)) {
                            player->x = new_x;
                            vx_accumulator[player_idx] -= move_step;
                            continue;
                        }
                    }
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 속성 지형 사망
                if (is_fireboy && target_tile == TILE_WATER_TERRAIN) {
                    player->state = PLAYER_STATE_DEAD;
                    return;
                }
                
                if (!is_fireboy && target_tile == TILE_FIRE_TERRAIN) {
                    player->state = PLAYER_STATE_DEAD;
                    return;
                }
                
                if (target_tile == TILE_FLOOR && player->is_on_ground) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                player->x = new_x;
                vx_accumulator[player_idx] -= move_step;
            } else {
                vx_accumulator[player_idx] = 0.0f;
                break;
            }
        } else if (vx_accumulator[player_idx] < 0.0f) {
            int new_x = player->x - 1;
            if (new_x >= 0 && new_x < map->width) {
                TileType target_tile = map_get_tile(map, new_x, player->y);
                
                if (target_tile == TILE_WALL) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }

                // 상자 밀기
                if (target_tile == TILE_BOX) {
                    int box_index = map_find_box(map, new_x, player->y);
                    int box_new_x = new_x - 1;
                    if (box_index >= 0 && box_new_x >= 0 && box_new_x < map->width) {
                        TileType box_target = map_get_tile(map, box_new_x, player->y);
                        if ((box_target == TILE_EMPTY || box_target == TILE_SWITCH) &&
                            map_move_box(map, box_index, box_new_x, player->y)) {
                            player->x = new_x;
                            vx_accumulator[player_idx] += move_step;
                            continue;
                        }
                    }
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                // 속성 지형 사망
                if (is_fireboy && target_tile == TILE_WATER_TERRAIN) {
                    player->state = PLAYER_STATE_DEAD;
                    return;
                }
                
                if (!is_fireboy && target_tile == TILE_FIRE_TERRAIN) {
                    player->state = PLAYER_STATE_DEAD;
                    return;
                }
                
                if (target_tile == TILE_FLOOR && player->is_on_ground) {
                    vx_accumulator[player_idx] = 0.0f;
                    break;
                }
                
                player->x = new_x;
                vx_accumulator[player_idx] += move_step;
            } else {
                vx_accumulator[player_idx] = 0.0f;
                break;
            }
        }
    }
    
    // 마찰
    if (!left_pressed && !right_pressed) {
        if (player->is_on_ground) {
            if (fabsf(vx_accumulator[player_idx]) > 0.1f) {
                vx_accumulator[player_idx] *= 0.8f;
            } else {
                vx_accumulator[player_idx] = 0.0f;
            }
        }
    }
    
    // 수직 이동
    static float vy_accumulator[2] = {0.0f, 0.0f};
    vy_accumulator[player_idx] += player->vy * delta_time;
    
    while (fabsf(vy_accumulator[player_idx]) >= 1.0f) {
        if (vy_accumulator[player_idx] > 0.0f) {
            int new_y = player->y + 1;
            if (new_y >= map->height) {
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            TileType current_tile = map_get_tile(map, player->x, new_y);
            TileType tile_below = (new_y + 1 < map->height) ? map_get_tile(map, player->x, new_y + 1) : TILE_EMPTY;
            
            if (current_tile == TILE_EMPTY) {
                player->y = new_y;
                player->is_on_ground = false;
                vy_accumulator[player_idx] -= 1.0f;
                continue;
            }
            
            if (current_tile == TILE_FLOOR && tile_below == TILE_EMPTY) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            if (tile_below == TILE_WALL || tile_below == TILE_FLOOR) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            if (tile_below == TILE_FIREBOY_START || tile_below == TILE_WATERGIRL_START) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            if ((tile_below == TILE_FIRE_TERRAIN && is_fireboy) ||
                (tile_below == TILE_WATER_TERRAIN && !is_fireboy)) {
                player->y = new_y;
                player->vy = 0;
                player->is_on_ground = true;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }
            
            player->y = new_y;
            player->is_on_ground = false;
            vy_accumulator[player_idx] -= 1.0f;
        } else if (vy_accumulator[player_idx] < 0.0f) {
            int new_y = player->y - 1;
            if (new_y < 0) {
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            }

            TileType tile_above = map_get_tile(map, player->x, new_y);
            if (tile_above == TILE_WALL || tile_above == TILE_FLOOR) {
                player->vy = 0;
                vy_accumulator[player_idx] = 0.0f;
                break;
            } else {
                player->y = new_y;
                vy_accumulator[player_idx] += 1.0f;
            }
        }
    }
    
    player->is_on_ground = check_ground(map, player->x, player->y, is_fireboy);
}
