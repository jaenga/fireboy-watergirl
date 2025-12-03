#include "common.h"
#include "console.h"
#include "input.h"

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

// 메인 함수
int main(void) {
    game_init();
    
    // 콘솔 테스트
    test_console();
    
    // 입력 테스트
    if (!input_is_quit_requested()) {
        test_input();
    }
    
    game_cleanup();
    
    printf("\n프로그램을 종료합니다.\n");
    return 0;
}
