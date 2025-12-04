#include "common.h"
#include "console.h"
#include "input.h"
#include "map.h"
#include "renderer.h"
#include "player.h"

// 게임 초기화
void game_init(void) {
    console_init();
    input_init();
    console_clear();
    console_hide_cursor();
}

// 게임 정리
void game_cleanup(void) {
    console_reset_color();
    console_show_cursor();
    console_set_cursor_position(0, 25);
    input_cleanup();
}

// 게임 루프
void game_loop(void) {
    console_clear();
    
    printf("=== 게임 시작 ===\n\n");
    printf("맵 파일 로딩 중...\n");
    
    const char* stage_path = "stages/stage1.txt";
    Map* map = map_load_from_file(stage_path);
    if (!map) {
        printf("맵 로드 실패!\n");
        printf("아무 키나 눌러 종료하세요...\n");
        while (!input_is_quit_requested()) {
            input_update();
            if (input_get_player_input().fireboy.enter || 
                input_get_player_input().watergirl.enter) {
                break;
            }
        }
        return;
    }
    
    printf("맵 로드 성공! (크기: %dx%d)\n", map->width, map->height);
    printf("Enter 키를 눌러 게임을 시작하세요...\n");
    
    // 입력 대기
    while (!input_is_quit_requested()) {
        input_update();
        if (input_get_player_input().fireboy.enter || 
            input_get_player_input().watergirl.enter) {
            break;
        }
    }
    
    // 플레이어 초기화
    Player fireboy, watergirl;
    player_init(&fireboy, PLAYER_FIREBOY, map->fireboy_start_x, map->fireboy_start_y);
    player_init(&watergirl, PLAYER_WATERGIRL, map->watergirl_start_x, map->watergirl_start_y);
    
    int death_count = 0;
    
    // 플레이어 이전 위치 추적
    int prev_fireboy_x = fireboy.x;
    int prev_fireboy_y = fireboy.y;
    int prev_watergirl_x = watergirl.x;
    int prev_watergirl_y = watergirl.y;
    
    // 렌더러 초기화
    renderer_init(80, 30);
    
    // 카메라 위치
    int camera_x = 0;
    int camera_y = 0;
    
    // 프레임 타이밍
    float delta_time = 0.05f;
    
    // 게임 루프
    while (!input_is_quit_requested()) {
        input_update();
        
        if (input_get_player_input().fireboy.escape) {
            break;
        }
        
        PlayerInput input = input_get_player_input();
        
        // 디버깅: 입력 상태 (HUD 아래 표시)
        console_set_cursor_position(0, 28);
        console_reset_color();
        printf("Fireboy: ←=%d →=%d ↑=%d | Watergirl: A=%d D=%d W=%d", 
               input.fireboy.left, input.fireboy.right, input.fireboy.jump,
               input.watergirl.left, input.watergirl.right, input.watergirl.jump);
        for (int i = 0; i < 20; i++) printf(" ");
        
        // 디버깅: 플레이어 위치 및 상태
        console_set_cursor_position(0, 27);
        console_reset_color();
        int fire_gems = player_get_fire_gem_count();
        int water_gems = player_get_water_gem_count();
        int total_gems = player_get_total_gem_count();
        printf("Fireboy: pos=(%2d,%2d) vy=%.1f ground=%d | Watergirl: pos=(%2d,%2d) vy=%.1f ground=%d | 사망: %d회 | 보석 F:%d W:%d 합:%d", 
               fireboy.x, fireboy.y, fireboy.vy, fireboy.is_on_ground,
               watergirl.x, watergirl.y, watergirl.vy, watergirl.is_on_ground,
               death_count, fire_gems, water_gems, total_gems);
        for (int i = 0; i < 5; i++) printf(" ");
        
        // 이동 발판 업데이트 (플레이어보다 먼저)
        map_update_platforms(map, delta_time, &fireboy, &watergirl);
        
        // 플레이어 업데이트
        player_update(&fireboy, map, input.fireboy.left, input.fireboy.right, input.fireboy.jump, delta_time);
        player_update(&watergirl, map, input.watergirl.left, input.watergirl.right, input.watergirl.jump, delta_time);

        // 사망 처리
        if (fireboy.state == PLAYER_STATE_DEAD || watergirl.state == PLAYER_STATE_DEAD) {
            death_count++;
            
            console_set_cursor_position(0, 15);
            console_set_color(COLOR_RED, COLOR_BLACK);
            if (fireboy.state == PLAYER_STATE_DEAD) {
                printf("              💀 Fireboy 사망! 사망 횟수: %d회 - 재시작...              ", death_count);
            } else {
                printf("              💀 Watergirl 사망! 사망 횟수: %d회 - 재시작...              ", death_count);
            }
            console_reset_color();
            fflush(stdout);
            
            #ifdef PLATFORM_WINDOWS
            Sleep(500);
            #else
            usleep(500000);
            #endif
            
            map_destroy(map);
            map = map_load_from_file(stage_path);
            if (!map) {
                fprintf(stderr, "맵 재로드 실패: %s\n", stage_path);
                break;
            }
            
            player_reset_gem_count();
            player_init(&fireboy, PLAYER_FIREBOY, map->fireboy_start_x, map->fireboy_start_y);
            player_init(&watergirl, PLAYER_WATERGIRL, map->watergirl_start_x, map->watergirl_start_y);
            
            prev_fireboy_x = fireboy.x;
            prev_fireboy_y = fireboy.y;
            prev_watergirl_x = watergirl.x;
            prev_watergirl_y = watergirl.y;
            
            console_clear();
            renderer_init(80, 30);
            
            continue;
        }
        
        // 이전 위치 타일 다시 그리기
        if (prev_fireboy_x != fireboy.x || prev_fireboy_y != fireboy.y) {
            TileType tile = map_get_tile(map, prev_fireboy_x, prev_fireboy_y);
            int screen_x = (prev_fireboy_x - camera_x) * 2;
            int screen_y = prev_fireboy_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 29) {
                render_tile(tile, (prev_fireboy_x - camera_x), (prev_fireboy_y - camera_y));
            }
            prev_fireboy_x = fireboy.x;
            prev_fireboy_y = fireboy.y;
        }
        
        if (prev_watergirl_x != watergirl.x || prev_watergirl_y != watergirl.y) {
            TileType tile = map_get_tile(map, prev_watergirl_x, prev_watergirl_y);
            int screen_x = (prev_watergirl_x - camera_x) * 2;
            int screen_y = prev_watergirl_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 29) {
                render_tile(tile, (prev_watergirl_x - camera_x), (prev_watergirl_y - camera_y));
            }
            prev_watergirl_x = watergirl.x;
            prev_watergirl_y = watergirl.y;
        }
        
        // 맵 렌더링
        render_map_no_flicker_with_players(map, camera_x, camera_y,
                                          fireboy.x, fireboy.y,
                                          watergirl.x, watergirl.y);
        
        render_player(&fireboy, camera_x, camera_y);
        render_player(&watergirl, camera_x, camera_y);
        
        // HUD 표시
        console_set_cursor_position(0, 29);
        console_reset_color();
        printf("Fireboy: ← → 이동 ↑ 점프 | Watergirl: A D 이동 W 점프 | ESC: 종료");
        for (int i = 0; i < 10; i++) printf(" ");
        
        fflush(stdout);
        
#ifdef PLATFORM_WINDOWS
        Sleep(50);
#else
        usleep(50000);
#endif
    }
    
    map_destroy(map);
    renderer_cleanup();
}

// 메인 함수
int main(void) {
    game_init();
    game_loop();
    game_cleanup();
    
    printf("\n프로그램을 종료합니다.\n");
    return 0;
}
