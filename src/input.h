#ifndef INPUT_H
#define INPUT_H

#include "common.h"

// 키 입력 상태
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool jump;      // 스페이스바
    bool escape;    // ESC
    bool enter;     // Enter
} KeyState;

// 플레이어별 입력
typedef struct {
    KeyState fireboy;   // 방향키 + 스페이스
    KeyState watergirl; // WASD + 스페이스
} PlayerInput;

// 함수 선언
void input_init(void);
void input_cleanup(void);
void input_update(void);
PlayerInput input_get_player_input(void);
bool input_is_quit_requested(void);
int input_getch_non_blocking(void); // 논블로킹 문자 입력
int input_get_stage_key(void); // 마지막에 눌린 스테이지 키 반환 (1-3, 없으면 -1)

// 키 코드 정의
#ifdef PLATFORM_WINDOWS
    #define KEY_UP_ARROW 72
    #define KEY_DOWN_ARROW 80
    #define KEY_LEFT_ARROW 75
    #define KEY_RIGHT_ARROW 77
    #define KEY_ESC 27
    #define KEY_ENTER 13
    #define KEY_SPACE 32
#else
    // Unix/macOS
    #define KEY_ESC 27
    #define KEY_ENTER 10
    #define KEY_SPACE 32
#endif

#endif // INPUT_H
