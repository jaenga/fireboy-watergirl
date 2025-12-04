#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"
#include "map.h"

// 플레이어 속성
typedef enum {
    PLAYER_FIREBOY,
    PLAYER_WATERGIRL
} PlayerType;

// 플레이어 상태
typedef enum {
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_DEAD
} PlayerState;

// 플레이어 구조체
typedef struct {
    PlayerType type;        // Fireboy 또는 Watergirl
    int x;                  // X 위치 (맵 좌표)
    int y;                  // Y 위치 (맵 좌표)
    float vx;               // X 속도 (현재는 사용 안 함, 나중을 위해)
    float vy;               // Y 속도 (현재는 사용 안 함, 나중을 위해)
    PlayerState state;      // 생존/사망 상태
    bool is_on_ground;      // 지상에 있는지 (현재는 사용 안 함)
} Player;

// 함수 선언
void player_init(Player* player, PlayerType type, int start_x, int start_y);
void player_update(Player* player, Map* map, bool left_pressed, bool right_pressed, bool jump_pressed, float delta_time);
void player_reset(Player* player, int start_x, int start_y);
bool player_check_collision(const Player* player, const Map* map, int new_x, int new_y);

// 보석 카운트 조회
int player_get_fire_gem_count(void);
int player_get_water_gem_count(void);
int player_get_total_gem_count(void);
void player_reset_gem_count(void);

#endif // PLAYER_H
