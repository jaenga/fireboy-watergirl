#include "input.h"

#ifdef PLATFORM_UNIX
    static struct termios old_termios;
    static bool input_initialized = false;
#endif

static PlayerInput current_input = {0};
static PlayerInput key_states = {0}; // 키 상태 추적 (키를 누르고 있는 동안 true 유지)
static bool quit_requested = false;
static int last_stage_key = -1; // 마지막에 눌린 숫자키 (1-3)

// 키 입력 타임스탬프 (마지막 키 입력 시간)
static struct timespec last_key_time[6] = {0}; // fireboy.left, fireboy.right, fireboy.jump, watergirl.left, watergirl.right, watergirl.jump
#define KEY_TIMEOUT_MS 100 // 100ms 타임아웃

// 현재 시간 가져오기
static void get_current_time(struct timespec* ts) {
#ifdef PLATFORM_WINDOWS
    // Windows
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // FILETIME은 100-nanosecond intervals since January 1, 1601
    // timespec은 seconds and nanoseconds since January 1, 1970
    uli.QuadPart -= 116444736000000000ULL; // Convert to Unix epoch
    ts->tv_sec = (long)(uli.QuadPart / 10000000ULL);
    ts->tv_nsec = (long)((uli.QuadPart % 10000000ULL) * 100);
#else
    // Unix/macOS
    clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

// 시간 차이 계산 (밀리초)
static long time_diff_ms(const struct timespec* t1, const struct timespec* t2) {
    long sec_diff = t1->tv_sec - t2->tv_sec;
    long nsec_diff = t1->tv_nsec - t2->tv_nsec;
    return sec_diff * 1000 + nsec_diff / 1000000;
}

// 논블로킹 문자 입력 처리
#ifdef PLATFORM_WINDOWS
int input_getch_non_blocking(void) {
    if (_kbhit()) {
        int ch = _getch();
        // 화살표 키는 두 바이트이므로 첫 바이트는 무시
        if (ch == 224 || ch == 0) {
            _getch(); // 두 번째 바이트 버리기
            return -1;
        }
        return ch;
    }
    return -1;
}
#else
// Unix/macOS에서 특수 키 입력 처리
int input_getch_non_blocking(void) {
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
        // 키 반복 활성화 (키를 누르고 있는 동안 계속 입력이 들어오도록)
        new_termios.c_iflag |= ICRNL; // CR을 NL로 변환
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
    // 점프 키는 매 프레임 초기화 (한 번만 점프하도록)
    current_input.fireboy.jump = false;
    current_input.watergirl.jump = false;
    current_input.fireboy.enter = false;
    current_input.watergirl.enter = false;
    current_input.fireboy.up = false;
    current_input.fireboy.down = false;
    current_input.watergirl.up = false;
    current_input.watergirl.down = false;
    current_input.fireboy.enter = false;
    
    // 현재 시간 가져오기
    struct timespec current_time;
    get_current_time(&current_time);
    
#ifdef PLATFORM_WINDOWS
    // Windows: 비동기 키 입력 처리 (모든 입력 버퍼 읽기)
    while (_kbhit()) {
        int ch = _getch();
        
        // 화살표 키는 두 바이트
        if (ch == 224 || ch == 0) {
            ch = _getch();
            switch (ch) {
                case KEY_UP_ARROW:
                    key_states.fireboy.jump = true;
                    current_input.fireboy.jump = true; // 점프는 이번 프레임에만 true
                    get_current_time(&last_key_time[2]); // fireboy.jump
                    break;
                case KEY_LEFT_ARROW:
                    key_states.fireboy.left = true;
                    get_current_time(&last_key_time[0]); // fireboy.left
                    break;
                case KEY_RIGHT_ARROW:
                    key_states.fireboy.right = true;
                    get_current_time(&last_key_time[1]); // fireboy.right
                    break;
            }
        } else {
            switch (ch) {
                case 'w':
                case 'W':
                    key_states.watergirl.jump = true;
                    current_input.watergirl.jump = true; // 점프는 이번 프레임에만 true
                    get_current_time(&last_key_time[5]); // watergirl.jump
                    break;
                case 'a':
                case 'A':
                    key_states.watergirl.left = true;
                    get_current_time(&last_key_time[3]); // watergirl.left
                    break;
                case 'd':
                case 'D':
                    key_states.watergirl.right = true;
                    get_current_time(&last_key_time[4]); // watergirl.right
                    break;
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
    // Unix/macOS: 비동기 키 입력 처리 (모든 입력 버퍼 읽기)
    int ch;
    while ((ch = input_getch_non_blocking()) != -1) {
        // ESC 시퀀스 처리 (화살표 키)
        if (ch == 27) {
            int ch2 = input_getch_non_blocking();
            if (ch2 == -1) {
                // ESC 키만 눌림
                current_input.fireboy.escape = true;
                quit_requested = true;
            } else if (ch2 == '[') {
                int ch3 = input_getch_non_blocking();
                if (ch3 != -1) {
                    // 화살표 키 처리
                    switch (ch3) {
                        case 'A': // 위 화살표
                            key_states.fireboy.up = true;
                            current_input.fireboy.up = true;
                            key_states.fireboy.jump = true;
                            current_input.fireboy.jump = true;
                            get_current_time(&last_key_time[2]);
                            break;
                        case 'B': // 아래 화살표
                            key_states.fireboy.down = true;
                            current_input.fireboy.down = true;
                            break;
                        case 'C': // 오른쪽 화살표
                            key_states.fireboy.right = true;
                            get_current_time(&last_key_time[1]);
                            break;
                        case 'D': // 왼쪽 화살표
                            key_states.fireboy.left = true;
                            get_current_time(&last_key_time[0]);
                            break;
                        default:
                            break;
                    }
                }
            }
        } else {
            switch (ch) {
                case 'w':
                case 'W':
                    key_states.watergirl.up = true;
                    current_input.watergirl.up = true;
                    key_states.watergirl.jump = true;
                    current_input.watergirl.jump = true;
                    get_current_time(&last_key_time[5]);
                    break;
                case 's':
                case 'S':
                    key_states.watergirl.down = true;
                    current_input.watergirl.down = true;
                    break;
                case 'a':
                case 'A':
                    key_states.watergirl.left = true;
                    get_current_time(&last_key_time[3]);
                    break;
                case 'd':
                case 'D':
                    key_states.watergirl.right = true;
                    get_current_time(&last_key_time[4]);
                    break;
                case KEY_ENTER:
                case 13:
                    current_input.fireboy.enter = true;
                    current_input.watergirl.enter = true;
                    break;
                case '1':
                case '2':
                case '3':
                    // 숫자키 저장 (스테이지 전환용)
                    last_stage_key = ch - '0';
                    break;
            }
        }
    }
#endif
    
    // 타임아웃 체크: 마지막 입력 시간이 100ms 이상 지나면 false로 설정
    // fireboy.left (인덱스 0)
    if (key_states.fireboy.left) {
        long diff = time_diff_ms(&current_time, &last_key_time[0]);
        if (diff > KEY_TIMEOUT_MS) {
            key_states.fireboy.left = false;
        }
    }
    
    // fireboy.right (인덱스 1)
    if (key_states.fireboy.right) {
        long diff = time_diff_ms(&current_time, &last_key_time[1]);
        if (diff > KEY_TIMEOUT_MS) {
            key_states.fireboy.right = false;
        }
    }
    
    // fireboy.jump (인덱스 2) - 점프는 이미 위에서 처리됨
    
    // watergirl.left (인덱스 3)
    if (key_states.watergirl.left) {
        long diff = time_diff_ms(&current_time, &last_key_time[3]);
        if (diff > KEY_TIMEOUT_MS) {
            key_states.watergirl.left = false;
        }
    }
    
    // watergirl.right (인덱스 4)
    if (key_states.watergirl.right) {
        long diff = time_diff_ms(&current_time, &last_key_time[4]);
        if (diff > KEY_TIMEOUT_MS) {
            key_states.watergirl.right = false;
        }
    }
    
    // watergirl.jump (인덱스 5) - 점프는 이미 위에서 처리됨
    
    // 키 상태를 current_input에 복사 (점프는 제외, 이미 위에서 처리됨)
    current_input.fireboy.left = key_states.fireboy.left;
    current_input.fireboy.right = key_states.fireboy.right;
    current_input.watergirl.left = key_states.watergirl.left;
    current_input.watergirl.right = key_states.watergirl.right;
}

// 플레이어 입력 가져오기
PlayerInput input_get_player_input(void) {
    return current_input;
}

// 마지막에 눌린 스테이지 키 반환 (1-3) 및 리셋
int input_get_stage_key(void) {
    int key = last_stage_key;
    last_stage_key = -1; // 읽은 후 리셋
    return key;
}

// 종료 요청 확인
bool input_is_quit_requested(void) {
    return quit_requested;
}
