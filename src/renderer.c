#include "renderer.h"

static int screen_width = 80;
static int screen_height = 25;
static TileType* prev_frame_buffer = NULL; // 이전 프레임 버퍼
static bool first_frame = true; // 첫 프레임 여부

// 렌더러 초기화
void renderer_init(int width, int height) {
    screen_width = width;
    screen_height = height;
    first_frame = true; // 첫 프레임 플래그 리셋
    
    // 이전 프레임 버퍼 할당
    if (prev_frame_buffer) {
        free(prev_frame_buffer);
    }
    prev_frame_buffer = (TileType*)malloc(width * height * sizeof(TileType));
    if (prev_frame_buffer) {
        // 초기화 (존재하지 않는 타일로 설정하여 첫 프레임에서 모두 렌더링되도록)
        for (int i = 0; i < width * height; i++) {
            prev_frame_buffer[i] = (TileType)0xFF; // 존재하지 않는 타일
        }
    }
}

// 렌더러 정리
void renderer_cleanup(void) {
    if (prev_frame_buffer) {
        free(prev_frame_buffer);
        prev_frame_buffer = NULL;
    }
}

// 타일을 화면에 렌더링 (유니코드 문자 + 배경색 사용, 타일당 2칸)
void render_tile(TileType tile, int x, int y) {
    console_set_cursor_position(x * 2, y); // 타일당 2칸 사용
    
    switch (tile) {
        case TILE_EMPTY:
            // 빈 공간 - 공백 (검은 배경)
            console_set_color(COLOR_WHITE, COLOR_BLACK);
            printf("  "); // 공백 2칸
            break;
            
        case TILE_WALL:
            // 벽 - 회색 배경
            console_set_color(COLOR_BLACK, COLOR_WHITE);
            printf("  "); // 공백 2칸 (배경색으로 표시)
            break;
            
        case TILE_FLOOR:
            // 바닥/플랫폼 - 회색 배경 (벽과 동일)
            console_set_color(COLOR_BLACK, COLOR_WHITE);
            printf("  "); // 공백 2칸 (배경색으로 표시)
            break;
            
        case TILE_FIRE_TERRAIN:
            // 불 지형 - 빨간 배경
            console_set_color(COLOR_YELLOW, COLOR_RED);
            printf("░░");
            break;
            
        case TILE_WATER_TERRAIN:
            // 물 지형 - 파란 배경
            console_set_color(COLOR_CYAN, COLOR_BLUE);
            printf("≈≈");
            break;
            
        case TILE_BOX:
            // 상자
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("▢ ");
            break;
            
        case TILE_SWITCH:
            console_set_color(COLOR_GREEN, COLOR_BLACK);
            printf("○ ");
            break;
            
        case TILE_DOOR:
            console_set_color(COLOR_MAGENTA, COLOR_BLACK);
            printf("▤ ");
            break;
            
        case TILE_MOVING_PLATFORM:
            console_set_color(COLOR_CYAN, COLOR_BLACK);
            printf("▄▄"); // 이동 발판
            break;
            
        case TILE_GEM:
            // 보석
            console_set_color(COLOR_GREEN, COLOR_BLACK);
            printf("◆ ");
            break;
            
        case TILE_FIREBOY_START:
            // Fireboy 시작 (빨간 배경)
            console_set_color(COLOR_YELLOW, COLOR_RED);
            printf("♂ ");
            break;
            
        case TILE_WATERGIRL_START:
            // Watergirl 시작 (파란 배경)
            console_set_color(COLOR_CYAN, COLOR_BLUE);
            printf("♀ ");
            break;
            
        case TILE_EXIT:
            console_set_color(COLOR_GREEN, COLOR_BLACK);
            printf("◉ "); // 출구
            break;
            
        default:
            console_set_color(COLOR_WHITE, COLOR_BLACK);
            printf("  ");
            break;
    }
    
    console_set_color(COLOR_WHITE, COLOR_BLACK); // 색상 리셋
}

// 맵 렌더링
void render_map(const Map* map, int camera_x, int camera_y) {
    if (!map) return;
    
    // 타일이 2칸씩 차지하므로 가로는 screen_width / 2만큼만 렌더링
    int tiles_per_row = screen_width / 2;
    
    // 카메라 위치 기반으로 맵의 일부만 렌더링
    for (int y = 0; y < screen_height - 1; y++) { // 마지막 줄은 HUD용으로 남김
        int map_y = camera_y + y;
        for (int x = 0; x < tiles_per_row; x++) {
            int map_x = camera_x + x;
            
            if (map_x >= 0 && map_x < map->width && map_y >= 0 && map_y < map->height) {
                TileType tile = map_get_tile(map, map_x, map_y);
                render_tile(tile, x, y);
            } else {
                // 맵 밖 영역은 빈 공간으로
                console_set_cursor_position(x * 2, y);
                console_set_color(COLOR_WHITE, COLOR_BLACK);
                printf("  ");
            }
        }
    }
}

// 깜빡임 없는 맵 렌더링 (더블 버퍼링)
void render_map_no_flicker(const Map* map, int camera_x, int camera_y) {
    if (!map) return;
    
    // 타일이 2칸씩 차지하므로 가로는 screen_width / 2만큼만 렌더링
    int tiles_per_row = screen_width / 2;
    
    // 첫 프레임에서는 전체 화면 클리어
    if (first_frame) {
        console_clear();
        first_frame = false;
    } else {
        // 이후 프레임에서는 커서만 처음 위치로 이동
        console_set_cursor_position(0, 0);
    }
    
    // 변경된 타일만 업데이트
    for (int y = 0; y < screen_height - 1; y++) {
        int map_y = camera_y + y;
        for (int x = 0; x < tiles_per_row; x++) {
            int map_x = camera_x + x;
            int buffer_idx = y * tiles_per_row + x;
            
            TileType current_tile = TILE_EMPTY;
            if (map_x >= 0 && map_x < map->width && map_y >= 0 && map_y < map->height) {
                current_tile = map_get_tile(map, map_x, map_y);
            }
            
            // 이전 프레임과 다를 때만 렌더링
            if (!prev_frame_buffer || prev_frame_buffer[buffer_idx] != current_tile) {
                render_tile(current_tile, x, y);
                if (prev_frame_buffer) {
                    prev_frame_buffer[buffer_idx] = current_tile;
                }
            }
        }
    }
}

// 깜빡임 없는 맵 렌더링 (플레이어 위치 제외)
void render_map_no_flicker_with_players(const Map* map, int camera_x, int camera_y,
                                        int player1_x, int player1_y,
                                        int player2_x, int player2_y) {
    if (!map) return;
    
    // 타일이 2칸씩 차지하므로 가로는 screen_width / 2만큼만 렌더링
    int tiles_per_row = screen_width / 2;
    
    // 첫 프레임에서는 전체 화면 클리어
    if (first_frame) {
        console_clear();
        first_frame = false;
    } else {
        // 이후 프레임에서는 커서만 처음 위치로 이동
        console_set_cursor_position(0, 0);
    }
    
    // 변경된 타일만 업데이트 (플레이어 위치 제외)
    for (int y = 0; y < screen_height - 1; y++) {
        int map_y = camera_y + y;
        for (int x = 0; x < tiles_per_row; x++) {
            int map_x = camera_x + x;
            
            // 플레이어 위치는 건너뛰기
            if ((map_x == player1_x && map_y == player1_y) ||
                (map_x == player2_x && map_y == player2_y)) {
                continue;
            }
            
            int buffer_idx = y * tiles_per_row + x;
            
            TileType current_tile = TILE_EMPTY;
            if (map_x >= 0 && map_x < map->width && map_y >= 0 && map_y < map->height) {
                current_tile = map_get_tile(map, map_x, map_y);
            }
            
            // 이전 프레임과 다를 때만 렌더링
            if (!prev_frame_buffer || prev_frame_buffer[buffer_idx] != current_tile) {
                render_tile(current_tile, x, y);
                if (prev_frame_buffer) {
                    prev_frame_buffer[buffer_idx] = current_tile;
                }
            }
        }
    }
}

// 플레이어 렌더링
void render_player(const Player* player, int camera_x, int camera_y) {
    if (!player) return;
    
    // 맵 좌표를 화면 좌표로 변환
    int screen_x = (player->x - camera_x) * 2; // 타일당 2칸
    int screen_y = player->y - camera_y;
    
    // 화면 범위 체크
    if (screen_x < 0 || screen_x >= screen_width || screen_y < 0 || screen_y >= screen_height - 1) {
        return;
    }
    
    // 플레이어 렌더링
    console_set_cursor_position(screen_x, screen_y);
    if (player->type == PLAYER_FIREBOY) {
        console_set_color(COLOR_YELLOW, COLOR_RED);
        printf("☻ ");
    } else {
        console_set_color(COLOR_CYAN, COLOR_BLUE);
        printf("☺ ");
    }
    console_reset_color();
}
