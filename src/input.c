#include "input.h"

#ifdef PLATFORM_UNIX
    static struct termios old_termios;
    static bool input_initialized = false;
#endif

static PlayerInput current_input = {0};
static bool quit_requested = false;

// Unix/macOS에서 특수 키 입력 처리
#ifdef PLATFORM_UNIX
static int getch_non_blocking(void) {
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            return (unsigned char)ch;
        }
    }
    return -1;
}
#endif

// 입력 시스템 초기화
void input_init(void) {
#ifdef PLATFORM_UNIX
    if (!input_initialized) {
        struct termios new_termios;
        tcgetattr(STDIN_FILENO, &old_termios);
        new_termios = old_termios;
        new_termios.c_lflag &= ~(ICANON | ECHO);
        new_termios.c_cc[VMIN] = 0;
        new_termios.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        input_initialized = true;
    }
#endif
}

// 입력 시스템 정리
void input_cleanup(void) {
#ifdef PLATFORM_UNIX
    if (input_initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
        input_initialized = false;
    }
#endif
}

// 입력 업데이트
void input_update(void) {
    // 이전 프레임 입력 초기화
    memset(&current_input, 0, sizeof(PlayerInput));
    
#ifdef PLATFORM_WINDOWS
    // Windows: 비동기 키 입력 처리
    if (_kbhit()) {
        int ch = _getch();
        
        // 화살표 키는 두 바이트
        if (ch == 224 || ch == 0) {
            ch = _getch();
            switch (ch) {
                case KEY_UP_ARROW:
                    current_input.fireboy.jump = true; // 위 화살표 = 점프
                    break;
                case KEY_LEFT_ARROW:
                    current_input.fireboy.left = true;
                    break;
                case KEY_RIGHT_ARROW:
                    current_input.fireboy.right = true;
                    break;
                // 아래 화살표는 사용 안 함
            }
        } else {
            switch (ch) {
                case 'w':
                case 'W':
                    current_input.watergirl.jump = true; // W = 점프
                    break;
                case 'a':
                case 'A':
                    current_input.watergirl.left = true;
                    break;
                case 'd':
                case 'D':
                    current_input.watergirl.right = true;
                    break;
                // S는 사용 안 함, 스페이스바도 사용 안 함
                case KEY_ESC:
                    current_input.fireboy.escape = true;
                    quit_requested = true;
                    break;
                case KEY_ENTER:
                    current_input.fireboy.enter = true;
                    break;
            }
        }
    }
#else
    // Unix/macOS: 비동기 키 입력 처리
    int ch = getch_non_blocking();
    
    if (ch != -1) {
        // ESC 시퀀스 처리 (화살표 키)
        if (ch == 27) {
            int ch2 = getch_non_blocking();
            if (ch2 == -1) {
                // ESC 키만 눌림
                current_input.fireboy.escape = true;
                quit_requested = true;
            } else if (ch2 == '[') {
                int ch3 = getch_non_blocking();
                switch (ch3) {
                    case 'A': // 위 화살표 = 점프
                        current_input.fireboy.jump = true;
                        break;
                    case 'C': // 오른쪽 화살표
                        current_input.fireboy.right = true;
                        break;
                    case 'D': // 왼쪽 화살표
                        current_input.fireboy.left = true;
                        break;
                    // 아래 화살표(B)는 사용 안 함
                }
            }
        } else {
            switch (ch) {
                case 'w':
                case 'W':
                    current_input.watergirl.jump = true; // W = 점프
                    break;
                case 'a':
                case 'A':
                    current_input.watergirl.left = true;
                    break;
                case 'd':
                case 'D':
                    current_input.watergirl.right = true;
                    break;
                // S는 사용 안 함, 스페이스바도 사용 안 함
                case KEY_ENTER:
                    current_input.fireboy.enter = true;
                    break;
            }
        }
    }
#endif
}

// 키가 눌렸는지 확인
bool input_is_key_pressed(char key) {
    // 간단한 문자 키 확인용
    (void)key; // 경고 방지
    return false; // 나중에 필요시 구현
}

// 플레이어 입력 가져오기
PlayerInput input_get_player_input(void) {
    return current_input;
}

// 종료 요청 확인
bool input_is_quit_requested(void) {
    return quit_requested;
}
