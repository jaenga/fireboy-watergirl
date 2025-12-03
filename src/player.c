#include "player.h"

// 이동 속도 (타일당 이동 속도, 4단계에서는 정수 단위로 이동)
#define MOVE_SPEED 1

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

// 플레이어 업데이트 (입력 처리 및 이동)
void player_update(Player* player, const Map* map, bool left_pressed, bool right_pressed) {
    if (!player || !map || player->state == PLAYER_STATE_DEAD) {
        return;
    }
    
    int new_x = player->x;
    
    // 좌우 이동 처리
    if (left_pressed) {
        new_x = player->x - MOVE_SPEED;
        printf("[PLAYER] %s left pressed, new_x=%d\n", 
               (player->type == PLAYER_FIREBOY) ? "Fireboy" : "Watergirl", new_x);
        fflush(stdout);
    } else if (right_pressed) {
        new_x = player->x + MOVE_SPEED;
        printf("[PLAYER] %s right pressed, new_x=%d\n", 
               (player->type == PLAYER_FIREBOY) ? "Fireboy" : "Watergirl", new_x);
        fflush(stdout);
    }
    
    // 충돌 체크 및 위치 업데이트
    if (new_x != player->x) {
        // 맵 경계 체크
        if (new_x >= 0 && new_x < map->width) {
            // 벽 충돌 체크 (현재 Y 위치에서)
            bool is_fireboy = (player->type == PLAYER_FIREBOY);
            bool walkable = map_is_walkable(map, new_x, player->y, is_fireboy);
            
            // 디버깅 출력
            printf("\n[PLAYER] %s: (%d,%d) -> (%d,%d), walkable=%d, tile='%c'\n",
                   is_fireboy ? "Fireboy" : "Watergirl",
                   player->x, player->y, new_x, player->y, walkable,
                   (char)map_get_tile(map, new_x, player->y));
            fflush(stdout);
            
            if (walkable) {
                player->x = new_x;
                printf("[PLAYER] %s moved to (%d,%d)\n", 
                       is_fireboy ? "Fireboy" : "Watergirl", player->x, player->y);
                fflush(stdout);
            } else {
                printf("[PLAYER] %s cannot move to (%d,%d) - not walkable\n",
                       is_fireboy ? "Fireboy" : "Watergirl", new_x, player->y);
                fflush(stdout);
            }
        }
    }
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

