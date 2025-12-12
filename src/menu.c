#include "menu.h"
#include "console.h"
#include "input.h"
#include "ranking.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 타이틀 그리기
static void draw_title(void) {
    console_clear();
    console_set_cursor_position(0, 2);
    
    // FIREBOY 타이틀
    console_set_color(COLOR_RED, COLOR_BLACK);
    printf("               ███████ ██ ██████  ███████ ██████   ██████  ██    ██\n");
    printf("               ██      ██ ██   ██ ██      ██   ██ ██    ██  ██  ██ \n");
    printf("               █████   ██ ██████  █████   ██████  ██    ██   ████  \n");
    printf("               ██      ██ ██   ██ ██      ██   ██ ██    ██    ██   \n");
    printf("               ██      ██ ██   ██ ███████ ██████   ██████     ██   \n");
    console_reset_color();
    printf("\n\n\n");
    
    // WATERGIRL 타이틀
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("       ██     ██  █████  ████████ ███████ ██████   ██████  ██ ██████  ██\n");
    printf("       ██     ██ ██   ██    ██    ██      ██   ██ ██       ██ ██   ██ ██\n");
    printf("       ██  █  ██ ███████    ██    █████   ██████  ██   ███ ██ ██████  ██\n");
    printf("       ██ ███ ██ ██   ██    ██    ██      ██   ██ ██    ██ ██ ██   ██ ██\n");
    printf("        ███ ███  ██   ██    ██    ███████ ██   ██  ██████  ██ ██   ██ ███████\n");
    console_reset_color();
    printf("\n");
    

    
    // 조작 안내
    console_set_cursor_position(0, 28);
    console_set_color(COLOR_GREEN, COLOR_BLACK);
    printf("               ↑↓ 또는 W/S: 메뉴 이동 | Enter: 선택\n");
    console_reset_color();
    fflush(stdout);
}

// 메뉴 아이템 그리기
static void draw_menu_items(int selected) {
    const char* menu_items[] = {
        "🎮 게임하기",
        "📖 게임설명", 
        "🏆 랭킹보기",
        "🚪 종료"
    };
    const int menu_count = 4;
    
    for (int i = 0; i < menu_count; i++) {
        console_set_cursor_position(30, 19 + i * 2);
        printf("                              ");
        console_set_cursor_position(30, 19 + i * 2);
        
        if (i == selected) {
            console_set_color(COLOR_BLACK, COLOR_YELLOW);
            console_set_attribute(ATTR_BOLD);
            printf("  ▶ %s ◀  ", menu_items[i]);
        } else {
            console_set_color(COLOR_WHITE, COLOR_BLACK);
            printf("    %s    ", menu_items[i]);
        }
        console_reset_color();
    }
    fflush(stdout);
}

// 메인 메뉴
MenuResult menu_show_main(void) {
    MenuResult result = {0};
    result.start_game = false;
    result.exit_game = false;
    
    const int menu_count = 4;
    int selected = 0;
    int last_selected = -1;
    
    draw_title();
    
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
                    console_clear();
                    draw_title();
                    last_selected = -1;
                    break;
                    
                case 1: // 게임설명
                    menu_show_instructions();
                    console_clear();
                    draw_title();
                    last_selected = -1;
                    break;
                    
                case 2: // 랭킹보기
                    menu_show_ranking();
                    console_clear();
                    draw_title();
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
        
        if (selected != last_selected) {
            draw_menu_items(selected);
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
    printf("                플레이어 이름을 입력하세요\n\n");
    console_reset_color();
    
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    printf("              (영문/숫자만 가능, ESC: 취소)\n\n");
    console_reset_color();
    
    console_set_color(COLOR_WHITE, COLOR_BLACK);
    printf("                      이름: ");
    console_reset_color();
    console_show_cursor();
    fflush(stdout);
    
    int cursor_x = 34;
    int cursor_y = 14;
    int name_len = 0;
    name[0] = '\0';
    
    // 입력 버퍼 클리어
    #ifdef PLATFORM_WINDOWS
    Sleep(200);
    #else
    usleep(200000);
    #endif
    
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
                    #ifdef PLATFORM_WINDOWS
                    Sleep(1000);
                    #else
                    usleep(1000000);
                    #endif
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
                    #ifdef PLATFORM_WINDOWS
                    Sleep(100);
                    #else
                    usleep(100000);
                    #endif
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
                    #ifdef PLATFORM_WINDOWS
                    Sleep(100);
                    #else
                    usleep(100000);
                    #endif
                }
            }
        }
        
        #ifdef PLATFORM_WINDOWS
        Sleep(50);
        #else
        usleep(50000);
        #endif
    }
}
