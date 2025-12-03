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

// 기본 테스트 함수
void test_console(void) {
    console_clear();
    
    printf("=== 콘솔 환경 테스트 ===\n\n");
    
    // 색상 테스트
    console_set_color_fg(COLOR_RED);
    printf("빨간색 텍스트\n");
    
    console_set_color_fg(COLOR_GREEN);
    printf("초록색 텍스트\n");
    
    console_set_color_fg(COLOR_BLUE);
    printf("파란색 텍스트\n");
    
    console_set_color_fg(COLOR_YELLOW);
    printf("노란색 텍스트\n");
    
    console_reset_color();
    printf("\n");
    
    // 커서 위치 테스트
    console_set_cursor_position(0, 10);
    console_set_color_fg(COLOR_CYAN);
    printf("커서가 (0, 10) 위치에 있습니다.\n");
    
    console_reset_color();
    console_set_cursor_position(0, 15);
    printf("아무 키나 눌러 계속하세요...\n");
    
    // 입력 대기
    while (!input_is_quit_requested()) {
        input_update();
        if (input_get_player_input().fireboy.enter || 
            input_get_player_input().watergirl.enter) {
            break;
        }
    }
}

// 입력 테스트 함수
void test_input(void) {
    console_clear();
    
    printf("=== 입력 시스템 테스트 ===\n\n");
    printf("Fireboy: ← → 이동, ↑ 점프\n");
    printf("Watergirl: A D 이동, W 점프\n");
    printf("ESC: 종료\n\n");
    printf("키를 눌러보세요...\n\n");
    
    while (!input_is_quit_requested()) {
        input_update();
        PlayerInput input = input_get_player_input();
        
        // 커서를 (0, 5)로 이동해서 입력 상태 표시
        console_set_cursor_position(0, 5);
        console_reset_color();
        
        printf("Fireboy 입력: ");
        if (input.fireboy.left) printf("← ");
        if (input.fireboy.right) printf("→ ");
        if (input.fireboy.jump) printf("↑(점프) ");
        printf("\n");
        
        printf("Watergirl 입력: ");
        if (input.watergirl.left) printf("A ");
        if (input.watergirl.right) printf("D ");
        if (input.watergirl.jump) printf("W(점프) ");
        printf("\n");
        
        // 화면 깜빡임 방지
        fflush(stdout);
        
        // 짧은 지연
#ifdef PLATFORM_WINDOWS
        Sleep(10);
#else
        usleep(10000); // 10ms
#endif
    }
}

// 맵 테스트 함수
void test_map(void) {
    console_clear();
    
    printf("=== 맵 시스템 테스트 ===\n\n");
    printf("맵 파일 로딩 중...\n");
    
    Map* map = map_load_from_file("stages/stage1.txt");
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
    printf("Fireboy 시작 위치: (%d, %d)\n", map->fireboy_start_x, map->fireboy_start_y);
    printf("Watergirl 시작 위치: (%d, %d)\n", map->watergirl_start_x, map->watergirl_start_y);
    printf("\nEnter 키를 눌러 맵을 보세요...\n");
    
    // 입력 대기
    while (!input_is_quit_requested()) {
        input_update();
        if (input_get_player_input().fireboy.enter || 
            input_get_player_input().watergirl.enter) {
            break;
        }
    }
    
    // 맵 렌더링
    if (!input_is_quit_requested()) {
        renderer_init(80, 25);
        
        while (!input_is_quit_requested()) {
            input_update();
            
            // ESC로 종료
            if (input_get_player_input().fireboy.escape) {
                break;
            }
            
            // 맵 렌더링 (깜빡임 없이)
            render_map_no_flicker(map, 0, 0);
            
            // HUD 표시 (마지막 줄)
            console_set_cursor_position(0, 24);
            console_reset_color();
            printf("ESC: 종료");
            
            fflush(stdout);
            
            // 짧은 지연
#ifdef PLATFORM_WINDOWS
            Sleep(50);
#else
            usleep(50000); // 50ms
#endif
        }
    }
    
    map_destroy(map);
}

// 게임 루프 (4단계: 캐릭터 기본 이동)
void game_loop(void) {
    console_clear();
    
    printf("=== 게임 시작 ===\n\n");
    printf("맵 파일 로딩 중...\n");
    
    Map* map = map_load_from_file("stages/stage1.txt");
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
    
    // 플레이어 이전 위치 추적 (렌더링을 위해)
    int prev_fireboy_x = fireboy.x;
    int prev_fireboy_y = fireboy.y;
    int prev_watergirl_x = watergirl.x;
    int prev_watergirl_y = watergirl.y;
    
    // 렌더러 초기화
    renderer_init(80, 25);
    
    // 카메라 위치 (현재는 고정, 나중에 플레이어 따라가도록 수정)
    int camera_x = 0;
    int camera_y = 0;
    
    // 게임 루프
    while (!input_is_quit_requested()) {
        input_update();
        
        // ESC로 종료
        if (input_get_player_input().fireboy.escape) {
            break;
        }
        
        // 입력 가져오기
        PlayerInput input = input_get_player_input();
        
        // 디버깅: 입력 상태 및 플레이어 위치 확인 (HUD 아래에 표시)
        console_set_cursor_position(0, 23);
        console_reset_color();
        printf("Fireboy: ←=%d →=%d ↑=%d | Watergirl: A=%d D=%d W=%d", 
               input.fireboy.left, input.fireboy.right, input.fireboy.jump,
               input.watergirl.left, input.watergirl.right, input.watergirl.jump);
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 20; i++) printf(" ");
        
        // 추가 디버깅: 플레이어 위치 (매 프레임)
        console_set_cursor_position(0, 22);
        console_reset_color();
        printf("Fireboy: pos=(%2d,%2d) | Watergirl: pos=(%2d,%2d)", 
               fireboy.x, fireboy.y, watergirl.x, watergirl.y);
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 20; i++) printf(" ");
        
        // 플레이어 업데이트 (좌우 이동만)
        player_update(&fireboy, map, input.fireboy.left, input.fireboy.right);
        player_update(&watergirl, map, input.watergirl.left, input.watergirl.right);
        
        // 플레이어가 이동한 경우 이전 위치의 타일 다시 그리기
        if (prev_fireboy_x != fireboy.x || prev_fireboy_y != fireboy.y) {
            TileType tile = map_get_tile(map, prev_fireboy_x, prev_fireboy_y);
            int screen_x = (prev_fireboy_x - camera_x) * 2;
            int screen_y = prev_fireboy_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 24) {
                render_tile(tile, (prev_fireboy_x - camera_x), (prev_fireboy_y - camera_y));
            }
            prev_fireboy_x = fireboy.x;
            prev_fireboy_y = fireboy.y;
        }
        
        if (prev_watergirl_x != watergirl.x || prev_watergirl_y != watergirl.y) {
            TileType tile = map_get_tile(map, prev_watergirl_x, prev_watergirl_y);
            int screen_x = (prev_watergirl_x - camera_x) * 2;
            int screen_y = prev_watergirl_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 24) {
                render_tile(tile, (prev_watergirl_x - camera_x), (prev_watergirl_y - camera_y));
            }
            prev_watergirl_x = watergirl.x;
            prev_watergirl_y = watergirl.y;
        }
        
        // 맵 렌더링 (플레이어 위치 제외)
        render_map_no_flicker_with_players(map, camera_x, camera_y,
                                          fireboy.x, fireboy.y,
                                          watergirl.x, watergirl.y);
        
        // 플레이어 렌더링
        render_player(&fireboy, camera_x, camera_y);
        render_player(&watergirl, camera_x, camera_y);
        
        // HUD 표시 (마지막 줄)
        console_set_cursor_position(0, 24);
        console_reset_color();
        printf("Fireboy: ← → 이동 ↑ 점프 | Watergirl: A D 이동 W 점프 | ESC: 종료");
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 10; i++) printf(" ");
        
        fflush(stdout);
        
        // 프레임 타이밍
#ifdef PLATFORM_WINDOWS
        Sleep(50);
#else
        usleep(50000); // 50ms
#endif
    }
    
    // 정리
    map_destroy(map);
    renderer_cleanup();
}

// 메인 함수
int main(void) {
    game_init();
    
    // 콘솔 테스트 (선택적 - 주석 처리 가능)
    // test_console();
    
    // 입력 테스트 (선택적 - 주석 처리 가능)
    // if (!input_is_quit_requested()) {
    //     test_input();
    // }
    
    // 맵 테스트 (선택적 - 주석 처리 가능)
    // if (!input_is_quit_requested()) {
    //     test_map();
    // }
    
    // 게임 루프 실행 (4단계)
    if (!input_is_quit_requested()) {
        game_loop();
    }
    
    game_cleanup();
    
    printf("\n프로그램을 종료합니다.\n");
    return 0;
}
