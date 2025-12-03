#include "console.h"

#ifdef PLATFORM_WINDOWS
    static HANDLE hConsole = NULL;
#endif

// 콘솔 초기화
void console_init(void) {
#ifdef PLATFORM_WINDOWS
    // 콘솔 창 크기 설정
    system("mode con cols=80 lines=25");
    
    // 콘솔 창 제목 설정
    SetConsoleTitle("Fireboy & Watergirl - Forest Temple");
    
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // 화면 버퍼 크기 설정
    COORD bufferSize = {80, 25};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    
    // Windows에서 ANSI 색상 지원 활성화 (Windows 10 이상)
    DWORD dwMode = 0;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#else
    // Unix/macOS에서는 터미널 설정 (ANSI 코드 지원)
    // 대부분의 최신 터미널은 기본적으로 ANSI 지원
#endif
    console_reset_color();
}

// 콘솔 정리
void console_cleanup(void) {
    console_reset_color();
    console_show_cursor();
}

// 화면 클리어
void console_clear(void) {
#ifdef PLATFORM_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

// 커서 위치 설정
void console_set_cursor_position(int x, int y) {
#ifdef PLATFORM_WINDOWS
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(hConsole, coord);
#else
    // ANSI 커서 위치 설정
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
#endif
}

// 커서 숨기기
void console_hide_cursor(void) {
#ifdef PLATFORM_WINDOWS
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    printf("\033[?25l");
    fflush(stdout);
#endif
}

// 커서 보이기
void console_show_cursor(void) {
#ifdef PLATFORM_WINDOWS
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    printf("\033[?25h");
    fflush(stdout);
#endif
}

// 색상 설정 (전경, 배경)
void console_set_color(ConsoleColor fg, ConsoleColor bg) {
#ifdef PLATFORM_WINDOWS
    if (hConsole != NULL) {
        WORD wColor = ((bg & 0x0F) << 4) | (fg & 0x0F);
        SetConsoleTextAttribute(hConsole, wColor);
    }
#else
    if (fg == COLOR_RESET) {
        printf("\033[0m");
    } else {
        int fg_code = 30 + fg;
        int bg_code = 40 + bg;
        printf("\033[%d;%dm", fg_code, bg_code);
    }
    fflush(stdout);
#endif
}

// 색상 리셋
void console_reset_color(void) {
#ifdef PLATFORM_WINDOWS
    if (hConsole != NULL) {
        SetConsoleTextAttribute(hConsole, 7); // 기본 회색
    }
#else
    printf("\033[0m");
    fflush(stdout);
#endif
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
#ifdef PLATFORM_WINDOWS
    // Windows에서는 색상 코드로 대체
#else
    if (attr == ATTR_BOLD) {
        printf("\033[1m");
    } else if (attr == ATTR_DIM) {
        printf("\033[2m");
    } else {
        printf("\033[0m");
    }
    fflush(stdout);
#endif
}

