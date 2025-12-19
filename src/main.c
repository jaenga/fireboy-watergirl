#include "common.h"
#include "console.h"
#include "input.h"
#include "map.h"
#include "renderer.h"
#include "player.h"
#include "menu.h"
#include "ranking.h"

#ifdef __APPLE__
    #include <sys/wait.h>
    #include <signal.h>
#endif

// 스테이지 관리
static int current_stage = 1;
#define MAX_STAGE 3

// 음악 재생 프로세스 ID (macOS에서만 사용)
#ifdef __APPLE__
    static pid_t music_pid = 0;
    
    // 시그널 핸들러 함수
    static void signal_handler(int sig) {
        // 핸들러에서는 waitpid 없이 kill만 수행 (더 안전함)
        if (music_pid > 0) {
            kill(music_pid, SIGTERM);
            music_pid = 0;
        }
        // 기본 동작으로 복귀하여 프로그램 종료
        signal(sig, SIG_DFL);
        raise(sig);
    }
    
    // 시그널 핸들러 초기화 함수
    static void music_init_signal_handlers(void) {
        signal(SIGINT, signal_handler);   // Ctrl+C
        signal(SIGTERM, signal_handler);  // 종료 시그널
        signal(SIGHUP, signal_handler);   // 터미널 닫힘
    }
#endif

// 음악 재생 함수
void music_play(const char* filename) {
#ifdef __APPLE__
    // 이전 음악 중지
    music_stop();
    
    // 새 음악 재생 (백그라운드)
    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스: afplay 직접 실행
        // execlp를 사용하여 PATH에서 afplay를 찾아 실행
        execlp("afplay", "afplay", filename, (char*)NULL);
        // execlp 실패 시 종료
        exit(1);
    } else if (pid > 0) {
        // 부모 프로세스: afplay의 실제 PID 저장
        music_pid = pid;
    }
#else
    // macOS가 아닌 경우 음악 재생 미지원
    (void)filename;
#endif
}

// 음악 중지 함수
void music_stop(void) {
#ifdef __APPLE__
    if (music_pid > 0) {
        // 프로세스가 아직 실행 중인지 확인
        int status;
        if (waitpid(music_pid, &status, WNOHANG) == 0) {
            // 실행 중이면 SIGTERM으로 종료
            kill(music_pid, SIGTERM);
            // 좀비 프로세스 방지를 위해 대기
            waitpid(music_pid, &status, 0);
        }
        music_pid = 0;
    }
#endif
}

// 스테이지 파일 경로 생성
static void get_stage_filename(int stage_id, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "stages/stage%d.txt", stage_id);
}

// 스테이지 로드 및 초기화
static bool load_stage(int stage_id, Map** map, Player* fireboy, Player* watergirl, 
                       int* prev_fireboy_x, int* prev_fireboy_y,
                       int* prev_watergirl_x, int* prev_watergirl_y) {
    char stage_file[256];
    get_stage_filename(stage_id, stage_file, sizeof(stage_file));
    
    // 기존 맵 정리
    if (*map) {
        map_destroy(*map);
    }
    
    // 새 맵 로드
    *map = map_load_from_file(stage_file);
    if (!*map) {
        return false;
    }
    
    // 플레이어 초기화
    player_init(fireboy, PLAYER_FIREBOY, (*map)->fireboy_start_x, (*map)->fireboy_start_y);
    player_init(watergirl, PLAYER_WATERGIRL, (*map)->watergirl_start_x, (*map)->watergirl_start_y);
    
    // 이전 위치 추적 초기화
    *prev_fireboy_x = fireboy->x;
    *prev_fireboy_y = fireboy->y;
    *prev_watergirl_x = watergirl->x;
    *prev_watergirl_y = watergirl->y;
    
    // 렌더러 리셋
    renderer_reset();
    console_clear();
    
    return true;
}

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

// 게임 루프 (4단계: 캐릭터 기본 이동)
void game_loop(const char* player_name) {
    console_clear();
    
    // 스테이지 초기화
    current_stage = 1;
    
    printf("=== 게임 시작 ===\n\n");
    printf("맵 파일 로딩 중...\n");
    
    // 맵 파일 경로 저장 (사망 시 맵 리로드용)
    char map_file_path[256];
    get_stage_filename(current_stage, map_file_path, sizeof(map_file_path));
    
    Map* map = NULL;
    Player fireboy, watergirl;
    int prev_fireboy_x, prev_fireboy_y;
    int prev_watergirl_x, prev_watergirl_y;
    
    // 첫 스테이지 로드
    if (!load_stage(current_stage, &map, &fireboy, &watergirl, 
                    &prev_fireboy_x, &prev_fireboy_y,
                    &prev_watergirl_x, &prev_watergirl_y)) {
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
    
    // 맵 로드 성공 후 스테이지 음악 재생
    char music_file[256];
    snprintf(music_file, sizeof(music_file), "assets/stage%d.mp3", current_stage);
    music_play(music_file);
    
    // 게임 시작 시 보석 개수 리셋
    player_reset_gem_count();
    player_reset_death_count();
    
    // 입력 대기
    while (!input_is_quit_requested()) {
        input_update();
        if (input_get_player_input().fireboy.enter || 
            input_get_player_input().watergirl.enter) {
            break;
        }
    }
    
    // 렌더러 초기화 (화면 크기: 가로 80, 세로 30)
    renderer_init(80, 30);
    
    // 카메라 위치 (현재는 고정, 나중에 플레이어 따라가도록 수정)
    int camera_x = 0;
    int camera_y = 0;
    
    // 프레임 타이밍
    float delta_time = 0.05f; // 50ms = 0.05초 (고정 프레임)
    
    // 게임 타이머 시작
    time_t game_start_time = time(NULL);
    
    // 게임 루프
    while (!input_is_quit_requested()) {
        input_update();
        
        // ESC로 종료
        if (input_get_player_input().fireboy.escape) {
            break;
        }
        
        // 디버그용: 숫자키로 스테이지 전환
        int stage_key = input_get_stage_key();
        if (stage_key >= 1 && stage_key <= 3) {
            int target_stage = stage_key;
            if (target_stage != current_stage && target_stage <= MAX_STAGE) {
                current_stage = target_stage;
                get_stage_filename(current_stage, map_file_path, sizeof(map_file_path));
                if (load_stage(current_stage, &map, &fireboy, &watergirl,
                              &prev_fireboy_x, &prev_fireboy_y,
                              &prev_watergirl_x, &prev_watergirl_y)) {
                    // 스테이지 음악 재생
                    char music_file[256];
                    snprintf(music_file, sizeof(music_file), "assets/stage%d.mp3", current_stage);
                    music_play(music_file);
                    
                    game_start_time = time(NULL); // 타이머 리셋
                    render_map_no_flicker_with_players(map, camera_x, camera_y,
                                                      fireboy.x, fireboy.y,
                                                      watergirl.x, watergirl.y);
                    render_player(&fireboy, camera_x, camera_y);
                    render_player(&watergirl, camera_x, camera_y);
                }
            }
        }
        
        // 입력 가져오기
        PlayerInput input = input_get_player_input();
        
        // 디버깅: 입력 상태 및 플레이어 위치 확인 (HUD 아래에 표시)
        console_set_cursor_position(0, 28);
        console_reset_color();
        printf("Fireboy: ←=%d →=%d ↑=%d | Watergirl: A=%d D=%d W=%d", 
               input.fireboy.left, input.fireboy.right, input.fireboy.jump,
               input.watergirl.left, input.watergirl.right, input.watergirl.jump);
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 20; i++) printf(" ");
        
        // 추가 디버깅: 플레이어 위치 및 상태 (매 프레임)
        console_set_cursor_position(0, 27);
        console_reset_color();
        printf("Fireboy: pos=(%2d,%2d) vy=%.1f ground=%d | Watergirl: pos=(%2d,%2d) vy=%.1f ground=%d", 
               fireboy.x, fireboy.y, fireboy.vy, fireboy.is_on_ground,
               watergirl.x, watergirl.y, watergirl.vy, watergirl.is_on_ground);
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 5; i++) printf(" ");
        
        // 맵 오브젝트 업데이트
        map_update_boxes(map, delta_time);
        map_update_switches(map, fireboy.x, fireboy.y, watergirl.x, watergirl.y);
        map_update_doors(map);
        map_update_platforms(map, delta_time, (struct Player*)&fireboy, (struct Player*)&watergirl);
        map_update_toggle_platforms(map, delta_time);
        map_update_vertical_walls(map, delta_time);
        
        // 플레이어 업데이트 (물리 시스템 포함)
        player_update(&fireboy, map, input.fireboy.left, input.fireboy.right, input.fireboy.jump, delta_time);
        player_update(&watergirl, map, input.watergirl.left, input.watergirl.right, input.watergirl.jump, delta_time);
        
        // Exit 도착 체크 (두 플레이어 모두 도착해야 함)
        bool fireboy_at_exit = (fireboy.x == map->exit_x && fireboy.y == map->exit_y);
        bool watergirl_at_exit = (watergirl.x == map->exit_x && watergirl.y == map->exit_y);
        
        if (fireboy_at_exit && watergirl_at_exit) {
            // 스테이지 클리어!
            time_t game_end_time = time(NULL);
            float elapsed_time = (float)(game_end_time - game_start_time);
            int deaths = player_get_death_count();
            int fire_gems = player_get_fire_gem_count();
            int water_gems = player_get_water_gem_count();
            int total_gems = player_get_total_gem_count();
            
            console_set_cursor_position(20, 15);
            console_set_color(COLOR_GREEN, COLOR_BLACK);
            console_set_attribute(ATTR_BOLD);
            printf("🎉 스테이지 클리어! 🎉");
            console_reset_color();
            console_set_cursor_position(15, 16);
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("시간: %.1f초 | 사망: %d회", elapsed_time, deaths);
            console_reset_color();
            console_set_cursor_position(15, 17);
            console_set_color(COLOR_RED, COLOR_BLACK);
            printf("🔥 Fire 보석: %d", fire_gems);
            console_reset_color();
            printf(" | ");
            console_set_color(COLOR_CYAN, COLOR_BLACK);
            printf("💧 Water 보석: %d", water_gems);
            console_reset_color();
            printf(" | ");
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("합계: %d", total_gems);
            console_reset_color();
            fflush(stdout);
            
            #ifdef PLATFORM_WINDOWS
            Sleep(3000);
            #else
            usleep(3000000);
            #endif
            
            // 마지막 스테이지인지 확인
            if (current_stage >= MAX_STAGE) {
                // Game Clear!
                console_clear();
                console_set_cursor_position(30, 12);
                console_set_color(COLOR_GREEN, COLOR_BLACK);
                console_set_attribute(ATTR_BOLD);
                printf("🎉🎉🎉 Game Clear! 🎉🎉🎉");
                console_reset_color();
                console_set_cursor_position(25, 14);
                console_set_color(COLOR_YELLOW, COLOR_BLACK);
                printf("총 시간: %.1f초 | 총 사망: %d회", elapsed_time, deaths);
                console_reset_color();
                console_set_cursor_position(25, 15);
                console_set_color(COLOR_RED, COLOR_BLACK);
                printf("🔥 Fire 보석: %d", fire_gems);
                console_reset_color();
                printf(" | ");
                console_set_color(COLOR_CYAN, COLOR_BLACK);
                printf("💧 Water 보석: %d", water_gems);
                console_reset_color();
                printf(" | ");
                console_set_color(COLOR_YELLOW, COLOR_BLACK);
                printf("합계: %d", total_gems);
                console_reset_color();
                fflush(stdout);
                
                #ifdef PLATFORM_WINDOWS
                Sleep(5000);
                #else
                usleep(5000000);
                #endif
                
                // 랭킹 저장
                if (player_name && strlen(player_name) > 0) {
                    RankingSystem ranking;
                    ranking_load(&ranking, "rankings.dat");
                    ranking_add_entry(&ranking, player_name, elapsed_time, deaths);
                    ranking_save(&ranking, "rankings.dat");
                }
                
                // 게임 종료
                break;
            } else {
                // 다음 스테이지로 이동
                current_stage++;
                get_stage_filename(current_stage, map_file_path, sizeof(map_file_path));
                
                if (load_stage(current_stage, &map, &fireboy, &watergirl,
                              &prev_fireboy_x, &prev_fireboy_y,
                              &prev_watergirl_x, &prev_watergirl_y)) {
                    // 스테이지 음악 재생
                    char music_file[256];
                    snprintf(music_file, sizeof(music_file), "assets/stage%d.mp3", current_stage);
                    music_play(music_file);
                    
                    game_start_time = time(NULL); // 타이머 리셋
                    
                    // 화면 다시 그리기
                    render_map_no_flicker_with_players(map, camera_x, camera_y,
                                                      fireboy.x, fireboy.y,
                                                      watergirl.x, watergirl.y);
                    render_player(&fireboy, camera_x, camera_y);
                    render_player(&watergirl, camera_x, camera_y);
                } else {
                    // 다음 스테이지 로드 실패
                    printf("다음 스테이지 로드 실패!\n");
                    break;
                }
            }
        }
        
        // 사망 체크
        if (fireboy.state == PLAYER_STATE_DEAD || watergirl.state == PLAYER_STATE_DEAD) {
            // 사망 횟수 증가
            player_increment_death_count();
            int deaths = player_get_death_count();
            
            // 화면 중앙에 사망 메시지 표시
            console_set_cursor_position(20, 15);
            console_set_color(COLOR_RED, COLOR_BLACK);
            console_set_attribute(ATTR_BOLD);
            printf("죽었습니다.. 사망 횟수: %d ", deaths);
            console_reset_color();
            fflush(stdout);
            
            #ifdef PLATFORM_WINDOWS
            Sleep(500);
            #else
            usleep(500000);
            #endif
            
            // 보석 개수 리셋
            player_reset_gem_count();
            
            // 현재 스테이지 다시 로드 (보석 복원)
            if (!load_stage(current_stage, &map, &fireboy, &watergirl,
                           &prev_fireboy_x, &prev_fireboy_y,
                           &prev_watergirl_x, &prev_watergirl_y)) {
                printf("맵 리로드 실패!\n");
                break;
            }
            
            // 화면 완전히 다시 그리기
            renderer_reset(); // 렌더러 상태 리셋 (first_frame = true)
            console_clear();
            render_map_no_flicker_with_players(map, camera_x, camera_y,
                                              fireboy.x, fireboy.y,
                                              watergirl.x, watergirl.y);
            render_player(&fireboy, camera_x, camera_y);
            render_player(&watergirl, camera_x, camera_y);
            
            // HUD 다시 그리기
            console_set_cursor_position(0, 29);
            console_reset_color();
            int fire_gems = player_get_fire_gem_count();
            int water_gems = player_get_water_gem_count();
            int total_gems = player_get_total_gem_count();
            deaths = player_get_death_count(); // 이미 위에서 선언됨
            
            console_set_color(COLOR_RED, COLOR_BLACK);
            printf("🔥F:%d", fire_gems);
            console_reset_color();
            printf(" ");
            console_set_color(COLOR_CYAN, COLOR_BLACK);
            printf("💧W:%d", water_gems);
            console_reset_color();
            printf(" 합:%d | ", total_gems);
            
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("사망:%d회", deaths);
            console_reset_color();
            printf(" | Stage:%d/%d | Fireboy:← → ↑ Watergirl:A D W ESC:종료", current_stage, MAX_STAGE);
            for (int i = 0; i < 3; i++) printf(" ");
            fflush(stdout);
        }
        
        // 플레이어가 이동한 경우 이전 위치의 타일 다시 그리기
        if (prev_fireboy_x != fireboy.x || prev_fireboy_y != fireboy.y) {
            TileType tile = map_get_tile(map, prev_fireboy_x, prev_fireboy_y);
            int screen_x = (prev_fireboy_x - camera_x) * 2;
            int screen_y = prev_fireboy_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 29) {
                render_tile(tile, (prev_fireboy_x - camera_x), (prev_fireboy_y - camera_y));
            }
            prev_fireboy_x = fireboy.x;
            prev_fireboy_y = fireboy.y;
        }
        
        if (prev_watergirl_x != watergirl.x || prev_watergirl_y != watergirl.y) {
            TileType tile = map_get_tile(map, prev_watergirl_x, prev_watergirl_y);
            int screen_x = (prev_watergirl_x - camera_x) * 2;
            int screen_y = prev_watergirl_y - camera_y;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 29) {
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
        console_set_cursor_position(0, 29);
        console_reset_color();
        
        // 보석 카운트 표시
        int fire_gems = player_get_fire_gem_count();
        int water_gems = player_get_water_gem_count();
        int total_gems = player_get_total_gem_count();
        int deaths = player_get_death_count();
        
        console_set_color(COLOR_RED, COLOR_BLACK);
        printf("🔥F:%d", fire_gems);
        console_reset_color();
        printf(" ");
        console_set_color(COLOR_CYAN, COLOR_BLACK);
        printf("💧W:%d", water_gems);
        console_reset_color();
        printf(" 합:%d | ", total_gems);
        
        console_set_color(COLOR_YELLOW, COLOR_BLACK);
        printf("사망:%d회", deaths);
        console_reset_color();
        printf(" | Stage:%d/%d | Fireboy:← → ↑ Watergirl:A D W ESC:종료", current_stage, MAX_STAGE);
        // 공백으로 나머지 공간 채우기
        for (int i = 0; i < 3; i++) printf(" ");
        
        fflush(stdout);
        
        // 프레임 타이밍
#ifdef PLATFORM_WINDOWS
        Sleep(50);
#else
        usleep(50000); // 50ms
#endif
    }
    
    // 정리
    music_stop(); // 게임 종료 시 음악 중지
    map_destroy(map);
    renderer_cleanup();
}

// 메인 함수
int main(void) {
    game_init();
    
#ifdef __APPLE__
    // TODO: 시그널 핸들러 초기화 (프로그램 시작 시 한 번만 호출)
    music_init_signal_handlers();
#endif
    
    // 프로그램 시작 시 intro 음악 재생
    music_play("assets/intro.mp3");
    
    // 메인 메뉴 루프
    while (true) {
        MenuResult result = menu_show_main();
        
        if (result.exit_game) {
            break;
        }
        
        if (result.start_game) {
            game_loop(result.player_name);
            // 게임 종료 후 메뉴로 돌아오면 intro 음악 다시 재생
            music_play("assets/intro.mp3");
        }
    }
    
    // 프로그램 종료 시 음악 중지
    music_stop();
    game_cleanup();
    
    printf("\n프로그램을 종료합니다.\n");
    return 0;
}
