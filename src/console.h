#ifndef CONSOLE_H
#define CONSOLE_H

#include "common.h"

// ANSI 색상 코드
typedef enum {
    COLOR_BLACK = 0,
    COLOR_RED = 1,
    COLOR_GREEN = 2,
    COLOR_YELLOW = 3,
    COLOR_BLUE = 4,
    COLOR_MAGENTA = 5,
    COLOR_CYAN = 6,
    COLOR_WHITE = 7,
    COLOR_RESET = 9
} ConsoleColor;

// 텍스트 속성
typedef enum {
    ATTR_NORMAL = 0,
    ATTR_BOLD = 1,
    ATTR_DIM = 2
} ConsoleAttribute;

// 함수 선언
void console_init(void);
void console_cleanup(void);
void console_clear(void);
void console_set_cursor_position(int x, int y);
void console_hide_cursor(void);
void console_show_cursor(void);
void console_set_color(ConsoleColor fg, ConsoleColor bg);
void console_reset_color(void);
void console_set_color_fg(ConsoleColor color);
void console_set_color_bg(ConsoleColor color);
void console_set_attribute(ConsoleAttribute attr);

#endif // CONSOLE_H

