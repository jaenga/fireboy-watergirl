#include "menu.h"
#include "console.h"
#include "input.h"
#include "ranking.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// 문자열의 화면 표시 폭 계산 (한글 2칸, ASCII 1칸)
// ============================================================================
static int get_display_width(const char* str) {
    int width = 0;
    int i = 0;
    
    while (str[i] != '\0') {
        unsigned char ch = (unsigned char)str[i];
        
        if ((ch & 0x80) == 0) {
            // ASCII (1바이트)
            width += 1;
            i += 1;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2바이트 UTF-8
            width += 1;
            i += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3바이트 UTF-8 (한글)
            width += 2;
            i += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4바이트 UTF-8
            width += 2;
            i += 4;
        } else {
            i += 1;
        }
    }
    
    return width;
}

// ============================================================================
// 타이틀 영역 그리기 (전체 화면: 타이틀 + 메뉴 + 안내)
// ============================================================================
static void draw_title(int selected) {
    
    console_set_cursor_position(0, 0);
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    console_set_attribute(ATTR_BOLD);
    
    // 상단 테두리
    printf("╔════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                                ║\n");
    
    // FIREBOY 타이틀
    console_set_color(COLOR_RED, COLOR_BLACK);
    printf("║               ███████ ██ ██████  ███████ ██████   ██████  ██    ██             ║\n");
    printf("║               ██      ██ ██   ██ ██      ██   ██ ██    ██  ██  ██              ║\n");
    printf("║               █████   ██ ██████  █████   ██████  ██    ██   ████               ║\n");
    printf("║               ██      ██ ██   ██ ██      ██   ██ ██    ██    ██                ║\n");
    printf("║               ██      ██ ██   ██ ███████ ██████   ██████     ██                ║\n");
    
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("║                                                                                ║\n");
    
    // WATERGIRL 타이틀
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("║       ██     ██  █████  ████████ ███████ ██████   ██████  ██ ██████  ██        ║\n");
    printf("║       ██     ██ ██   ██    ██    ██      ██   ██ ██       ██ ██   ██ ██        ║\n");
    printf("║       ██  █  ██ ███████    ██    █████   ██████  ██   ███ ██ ██████  ██        ║\n");
    printf("║       ██ ███ ██ ██   ██    ██    ██      ██   ██ ██    ██ ██ ██   ██ ██        ║\n");
    printf("║        ███ ███  ██   ██    ██    ███████ ██   ██  ██████  ██ ██   ██ ██████    ║\n");
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    printf("║                                                                                ║\n");
    printf("║                                                                                ║\n");
    printf("║                                                                                ║\n");

    
    // 메뉴 아이템들 
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 0) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 게임하기 ◀                                  ║\n");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   게임하기                                     ║\n");
    }
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 1) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 게임설명 ◀                                  ║\n");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   게임설명                                     ║\n");
    }
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 2) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 랭킹보기 ◀                                  ║\n");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   랭킹보기                                     ║\n");
    }
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 3) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                  ▶ 종료 ◀                                    ║\n");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                     종료                                       ║\n");
    }
    
    // 빈 줄
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("║                                                                                ║\n");
    printf("║                                                                                ║\n");

    // 하단 안내
    printf("║                                                                                ║\n");
    console_set_color(COLOR_CYAN, COLOR_BLACK); 
    printf("║                    ↑↓ 또는 W/S: 메뉴 이동 | Enter: 선택                        ║\n");
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    console_set_attribute(ATTR_BOLD);
    printf("║                                                                                ║\n");
    
    // 하단 테두리
    printf("╚════════════════════════════════════════════════════════════════════════════════╝");
    console_reset_color();
    fflush(stdout);
}

// ============================================================================
// 메뉴 아이템만 다시 그리기 (선택 변경 시 사용)
// ============================================================================
static void draw_menu_items_only(int selected) {
    const int menu_start_y = 16;
    
    // 게임하기
    console_set_cursor_position(0, menu_start_y);
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 0) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 게임하기 ◀                                  ║");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   게임하기                                     ║");
    }
    
    // 게임설명
    console_set_cursor_position(0, menu_start_y + 1);
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 1) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 게임설명 ◀                                  ║");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   게임설명                                     ║");
    }
    
    // 랭킹보기
    console_set_cursor_position(0, menu_start_y + 2);
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 2) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                ▶ 랭킹보기 ◀                                  ║");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                   랭킹보기                                     ║");
    }
    
    // 종료
    console_set_cursor_position(0, menu_start_y + 3);
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    if (selected == 3) {
        console_set_attribute(ATTR_BOLD);
        printf("║                                  ▶ 종료 ◀                                    ║");
    } else {
        console_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("║                                     종료                                       ║");
    }
    
    console_reset_color();
    fflush(stdout);
}

// ============================================================================
// 메인 메뉴
// ============================================================================
MenuResult menu_show_main(void) {
    MenuResult result = {0};
    result.start_game = false;
    result.exit_game = false;
    
    const int menu_count = 4;
    int selected = 0;
    int last_selected = -1;
    
    // 초기 화면 그리기
    console_clear();
    draw_title(selected);
    
    while (true) {
        input_update();
        PlayerInput player_input = input_get_player_input();
        
        if (player_input.fireboy.up || player_input.watergirl.up) {
            selected = (selected - 1 + menu_count) % menu_count;
            #ifdef PLATFORM_WINDOWS
            Sleep(150);
            #else
            usleep(150000);
            #endif
        } else if (player_input.fireboy.down || player_input.watergirl.down) {
            selected = (selected + 1) % menu_count;
            #ifdef PLATFORM_WINDOWS
            Sleep(150);
            #else
            usleep(150000);
            #endif
        } else if (player_input.fireboy.enter || player_input.watergirl.enter) {
            #ifdef PLATFORM_WINDOWS
            Sleep(200);
            #else
            usleep(200000);
            #endif
            
            switch (selected) {
                case 0: // 게임하기
                    if (menu_get_player_name(result.player_name, MAX_NAME_LENGTH)) {
                        result.start_game = true;
                        result.exit_game = false;
                        return result;
                    }
                    // 메뉴로 돌아올 때 화면 다시 그리기
                    console_clear();
                    draw_title(selected);
                    last_selected = -1;
                    break;
                    
                case 1: // 게임설명
                    menu_show_instructions();
                    // 메뉴로 돌아올 때 화면 다시 그리기
                    console_clear();
                    draw_title(selected);
                    last_selected = -1;
                    break;
                    
                case 2: // 랭킹보기
                    menu_show_ranking();
                    // 메뉴로 돌아올 때 화면 다시 그리기
                    console_clear();
                    draw_title(selected);
                    last_selected = -1;
                    break;
                    
                case 3: // 종료
                    result.exit_game = true;
                    result.start_game = false;
                    return result;
            }
        } else if (player_input.fireboy.escape) {
            result.exit_game = true;
            result.start_game = false;
            return result;
        }
        
        // 선택된 항목이 변경되었을 때만 메뉴 아이템만 다시 그리기 (깜빡임 방지)
        if (selected != last_selected) {
            draw_menu_items_only(selected);
            last_selected = selected;
        }
        
        #ifdef PLATFORM_WINDOWS
        Sleep(50);
        #else
        usleep(50000);
        #endif
    }
    
    return result;
}

// 게임 설명
void menu_show_instructions(void) {
    console_clear();
    console_set_cursor_position(0, 2);
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    console_set_attribute(ATTR_BOLD);
    printf("                      📖 게임 설명 📖\n\n");
    console_reset_color();
    
    console_set_color(COLOR_WHITE, COLOR_BLACK);
    printf("     🎯 목표: Fireboy와 Watergirl을 각자의 문으로 인도하세요!\n\n");
    
    printf("     🔥 Fireboy (빨간색)\n");
    printf("        - 물(파란색)을 피하세요!\n");
    printf("        - 불(빨간색)은 안전합니다\n");
    printf("        - 조작: ← → 이동, ↑ 점프\n\n");
    
    printf("     💧 Watergirl (파란색)\n");
    printf("        - 불(빨간색)을 피하세요!\n");
    printf("        - 물(파란색)은 안전합니다\n");
    printf("        - 조작: A D 이동, W 점프\n\n");
    
    printf("     💎 보석을 모두 수집하세요!\n");
    printf("     🚪 스위치를 밟아 문을 여세요!\n");
    printf("     ⚙️  협력해서 퍼즐을 풀어보세요!\n\n");
    console_reset_color();
    
    console_set_color(COLOR_BLACK, COLOR_GREEN);
    printf("          아무 키나 눌러 돌아가기          ");
    console_reset_color();
    printf("\n");
    fflush(stdout);
    
    // 키 입력 대기
    while (!input_is_quit_requested()) {
        input_update();
        PlayerInput inp = input_get_player_input();
        if (inp.fireboy.enter || inp.watergirl.enter || inp.fireboy.escape) {
            break;
        }
        #ifdef PLATFORM_WINDOWS
        Sleep(50);
        #else
        usleep(50000);
        #endif
    }
}

// 랭킹 화면
void menu_show_ranking(void) {
    RankingSystem ranking;
    ranking_load(&ranking, "rankings.dat");
    ranking_display(&ranking);
}

// 플레이어 이름 입력
bool menu_get_player_name(char* name, int max_length) {
    console_clear();
    console_set_cursor_position(0, 12);
    
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    console_set_attribute(ATTR_BOLD);
    printf("                플레이어 이름을 입력하세요\n\n");  // 12줄, 13줄 빈 줄
    console_reset_color();
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    printf("                    (한글/영문/숫자 가능, ESC: 취소)\n\n");  // 14줄, 15줄 빈 줄
    console_reset_color();
    
    console_set_color(COLOR_WHITE, COLOR_BLACK);
    printf("                      이름: ");  // 16줄
    console_reset_color();
    fflush(stdout);
    
    // "이름: " 다음 위치 계산
    // "                      " = 22 바이트 (공백 22개)
    // "이름" = 6 바이트 (한글 2글자, UTF-8로 각 3바이트)
    // ": " = 2 바이트 (콜론 1바이트 + 공백 1바이트)
    // 총 30 바이트 위치에서 시작
    int cursor_x = 30;  // "                      이름: " 다음 위치
    int cursor_y = 16;  // 16줄 (12 + \n\n + 14 + \n\n + 16)
    
    // 입력 필드 초기 위치 설정
    console_set_cursor_position(cursor_x, cursor_y);
    console_show_cursor();
    fflush(stdout);
    
    name[0] = '\0';
    
#ifdef PLATFORM_UNIX
    // 입력 시스템 정리 (non-canonical 모드 해제)
    input_cleanup();
    
    // 원래 터미널 설정 저장
    struct termios saved_termios, new_termios;
    tcgetattr(STDIN_FILENO, &saved_termios);
    new_termios = saved_termios;
    
    // Non-canonical 모드, ECHO 비활성화 (직접 화면 제어)
    new_termios.c_lflag &= ~(ICANON | ECHO);  // non-canonical, ECHO 비활성화
    new_termios.c_cc[VMIN] = 1;      // 최소 1바이트 읽기
    new_termios.c_cc[VTIME] = 0;     // 타임아웃 없음
    
    // 입력 모드 설정
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
    
    // 입력 버퍼 클리어
    usleep(200000);
    tcflush(STDIN_FILENO, TCIFLUSH);
    char buffer[256] = {0};
    int buf_len = 0;  // 바이트 길이
    int char_count = 0;  // 문자 개수 (UTF-8 문자 기준)
    int screen_width = 0;  // 화면 폭 (커서 위치 계산용)
    
    while (true) {
        unsigned char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            // ESC 키 처리
            if (ch == 27) {
                // ESC 시퀀스 확인 (화살표 키인지 확인)
                fd_set readfds;
                struct timeval timeout;
                FD_ZERO(&readfds);
                FD_SET(STDIN_FILENO, &readfds);
                timeout.tv_sec = 0;
                timeout.tv_usec = 50000;  // 50ms 대기
                
                if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0) {
                    unsigned char ch2;
                    if (read(STDIN_FILENO, &ch2, 1) == 1) {
                        if (ch2 == '[') {
                            // 화살표 키는 무시
                            unsigned char ch3;
                            read(STDIN_FILENO, &ch3, 1);
                            continue;
                        }
                    }
                }
                // ESC 키만 눌림 (취소)
                tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
                fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
                input_init();
                console_hide_cursor();
                return false;
            }
            // Enter 키 처리
            else if (ch == '\n' || ch == '\r') {
                if (buf_len == 0) {
                    // 이름이 비어있으면 에러 메시지
                    console_set_cursor_position(28, 16);
                    console_set_color(COLOR_RED, COLOR_BLACK);
                    printf("이름을 입력하세요!");
                    console_reset_color();
                    fflush(stdout);
                    usleep(1000000);
                    console_set_cursor_position(28, 16);
                    printf("                  ");
                    fflush(stdout);
                    continue;
                }
                buffer[buf_len] = '\0';
                if (buf_len >= max_length) {
                    buffer[max_length - 1] = '\0';
                }
                memcpy(name, buffer, max_length);
                tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
                fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
                input_init();
                console_hide_cursor();
                return true;
            }
            // Backspace 또는 Delete 처리
            else if (ch == 127 || ch == 8) {
                if (buf_len > 0) {
                    // UTF-8 문자 경계 찾기 (마지막 문자 제거)
                    int bytes_to_remove = 1;
                    int char_start_pos = buf_len - 1;
                    // UTF-8 문자의 시작 바이트 찾기 (최대 4바이트 뒤까지 확인)
                    int check_start = buf_len - 1;
                    int check_end = (buf_len > 4) ? buf_len - 4 : 0;
                    for (int i = check_start; i >= check_end; i--) {
                        unsigned char b = (unsigned char)buffer[i];
                        // UTF-8 시작 바이트 확인 (0xxxxxxx 또는 11xxxxxx)
                        if ((b & 0x80) == 0 || (b & 0xC0) == 0xC0) {
                            bytes_to_remove = buf_len - i;
                            char_start_pos = i;
                            break;
                        }
                    }
                    
                    // 제거할 문자의 화면 폭 계산
                    unsigned char first_byte = (unsigned char)buffer[char_start_pos];
                    int char_screen_width = 1;  // 기본값
                    if ((first_byte & 0x80) == 0) {
                        char_screen_width = 1;  // ASCII
                    } else if ((first_byte & 0xF0) == 0xE0) {
                        char_screen_width = 2;  // 3바이트 UTF-8 (한글)
                    } else if ((first_byte & 0xF8) == 0xF0) {
                        char_screen_width = 2;  // 4바이트 UTF-8 (이모지 등)
                    }
                    // 2바이트 UTF-8은 1칸으로 가정
                    
                    // 버퍼에서 문자 제거
                    buf_len -= bytes_to_remove;
                    char_count--;
                    if (char_count < 0) char_count = 0;
                    screen_width -= char_screen_width;
                    if (screen_width < 0) screen_width = 0;
                    
                    // 백스페이스 처리: 제거된 문자의 화면 폭만큼 공백 출력
                    console_set_cursor_position(cursor_x + screen_width, cursor_y);
                    for (int i = 0; i < char_screen_width; i++) {
                        printf(" ");
                    }
                    console_set_cursor_position(cursor_x + screen_width, cursor_y);
                    fflush(stdout);
                }
            }
            // 일반 문자 처리 (UTF-8 지원)
            else if (buf_len < max_length - 1) {
                // UTF-8 문자 시작 바이트 확인
                unsigned char first_byte = (unsigned char)ch;
                int utf8_bytes_needed = 1;
                int char_screen_width = 1;  // 화면 폭 (기본값)
                
                if ((first_byte & 0x80) == 0) {
                    utf8_bytes_needed = 1;  // ASCII
                    char_screen_width = 1;
                } else if ((first_byte & 0xE0) == 0xC0) {
                    utf8_bytes_needed = 2;
                    char_screen_width = 1;  // 2바이트 UTF-8 (보통 1칸)
                } else if ((first_byte & 0xF0) == 0xE0) {
                    utf8_bytes_needed = 3;  // 한글
                    char_screen_width = 2;
                } else if ((first_byte & 0xF8) == 0xF0) {
                    utf8_bytes_needed = 4;
                    char_screen_width = 2;  // 4바이트 UTF-8 (이모지 등)
                }
                
                // 첫 바이트 저장
                buffer[buf_len++] = ch;
                
                // UTF-8 문자의 나머지 바이트 읽기
                if (utf8_bytes_needed > 1 && buf_len < max_length - 1) {
                    for (int i = 1; i < utf8_bytes_needed && buf_len < max_length - 1; i++) {
                        unsigned char next_byte;
                        if (read(STDIN_FILENO, &next_byte, 1) == 1) {
                            buffer[buf_len++] = next_byte;
                        } else {
                            break;
                        }
                    }
                }
                
                char_count++;
                screen_width += char_screen_width;
                
                // 입력된 문자만 출력 (화면 다시 그리지 않음)
                // 현재 입력 위치에 문자 출력
                int output_pos = buf_len - utf8_bytes_needed;
                console_set_cursor_position(cursor_x + screen_width - char_screen_width, cursor_y);
                for (int i = 0; i < utf8_bytes_needed; i++) {
                    printf("%c", buffer[output_pos + i]);
                }
                // 커서를 입력 끝 위치로 이동 (화면 폭 기준)
                console_set_cursor_position(cursor_x + screen_width, cursor_y);
                fflush(stdout);
            }
        }
    }
    
#else
    // Windows용 코드 (기존 로직 유지하되 영문/숫자만 지원)
    int cursor_x = 34;
    int cursor_y = 14;
    int name_len = 0;
    
    // 입력 버퍼 클리어
    Sleep(200);
    
    while (true) {
        int ch = input_getch_non_blocking();
        if (ch != -1) {
            // ESC로 취소
            if (ch == 27) {
                console_hide_cursor();
                return false;
            }
            // Enter로 확정
            else if (ch == '\n' || ch == '\r' || ch == 10 || ch == 13) {
                if (name_len == 0) {
                    // 이름이 비어있으면 에러 메시지
                    console_set_cursor_position(28, 16);
                    console_set_color(COLOR_RED, COLOR_BLACK);
                    printf("이름을 입력하세요!");
                    console_reset_color();
                    fflush(stdout);
                    Sleep(1000);
                    console_set_cursor_position(28, 16);
                    printf("                  ");
                    fflush(stdout);
                    continue;
                }
                name[name_len] = '\0';
                console_hide_cursor();
                return true;
            }
            // Backspace
            else if (ch == 127 || ch == 8) {
                if (name_len > 0) {
                    name_len--;
                    console_set_cursor_position(cursor_x + name_len, cursor_y);
                    printf(" ");
                    console_set_cursor_position(cursor_x + name_len, cursor_y);
                    fflush(stdout);
                    Sleep(100);
                }
            }
            // 유효한 문자 (영문, 숫자, _, -)
            else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || 
                     (ch >= '0' && ch <= '9') || ch == '_' || ch == '-') {
                if (name_len < max_length - 1) {
                    name[name_len] = ch;
                    console_set_cursor_position(cursor_x + name_len, cursor_y);
                    printf("%c", ch);
                    fflush(stdout);
                    name_len++;
                    Sleep(100);
                }
            }
        }
        
        Sleep(50);
    }
#endif
}

