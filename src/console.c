#include "console.h"

// 콘솔 초기화
void console_init(void) {
    // Unix/macOS/Linux 터미널 설정 (ANSI 코드 지원)
    // 대부분의 최신 터미널은 기본적으로 ANSI 지원
    console_reset_color();
}

// 콘솔 정리
void console_cleanup(void) {
    console_reset_color();
    console_show_cursor();
}

// 화면 클리어
void console_clear(void) {
    system("clear");
}

// 커서 위치 설정
void console_set_cursor_position(int x, int y) {
    // ANSI 커서 위치 설정
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

// 커서 숨기기
void console_hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

// 커서 보이기
void console_show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

// 색상 설정 (전경, 배경)
void console_set_color(ConsoleColor fg, ConsoleColor bg) {
    if (fg == COLOR_RESET) {
        printf("\033[0m");
    } else {
        int fg_code = 30 + fg;
        int bg_code = 40 + bg;
        printf("\033[%d;%dm", fg_code, bg_code);
    }
    fflush(stdout);
}

// 색상 리셋
void console_reset_color(void) {
    printf("\033[0m");
    fflush(stdout);
}

// 전경색만 설정
void console_set_color_fg(ConsoleColor color) {
    console_set_color(color, COLOR_BLACK);
}

// 배경색만 설정
void console_set_color_bg(ConsoleColor color) {
    console_set_color(COLOR_WHITE, color);
}

// 텍스트 속성 설정
void console_set_attribute(ConsoleAttribute attr) {
    if (attr == ATTR_BOLD) {
        printf("\033[1m");
    } else if (attr == ATTR_DIM) {
        printf("\033[2m");
    } else {
        printf("\033[0m");
    }
    fflush(stdout);
}

