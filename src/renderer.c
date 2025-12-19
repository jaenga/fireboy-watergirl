#include "renderer.h"
#include <math.h>

static int screen_width = 80;
static int screen_height = 25;
static TileType* prev_frame_buffer = NULL; // 이전 프레임 버퍼
static bool first_frame = true; // 첫 프레임 여부

// 이동 발판의 이전 위치 추적
static int prev_platform_x[MAX_PLATFORMS];
static int prev_platform_y[MAX_PLATFORMS];
static bool prev_platform_valid[MAX_PLATFORMS];

// 토글 플랫폼의 이전 위치 추적
static int prev_toggle_platform_y[MAX_PLATFORMS];
static bool prev_toggle_platform_valid[MAX_PLATFORMS];

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
    
    // 발판 위치 추적 초기화
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        prev_platform_valid[i] = false;
    }
}

// 렌더러 정리
void renderer_cleanup(void) {
    if (prev_frame_buffer) {
        free(prev_frame_buffer);
        prev_frame_buffer = NULL;
    }
}

// 렌더러 리셋 (사망 후 화면 다시 그리기용)
void renderer_reset(void) {
    first_frame = true;
    // 이전 프레임 버퍼 초기화
    if (prev_frame_buffer) {
        for (int i = 0; i < screen_width * screen_height; i++) {
            prev_frame_buffer[i] = (TileType)0xFF; // 존재하지 않는 타일
        }
    }
    // 발판 위치 추적 초기화
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        prev_platform_valid[i] = false;
        prev_toggle_platform_valid[i] = false;
    }
}

// 타일을 화면에 렌더링 (유니코드 문자 + 배경색 사용, 타일당 2칸)
// map과 map_x, map_y를 전달하면 스위치/도어 상태를 확인하여 색상 변경
void render_tile(TileType tile, int x, int y) {
    render_tile_with_map(tile, x, y, NULL, -1, -1);
}

// 타일을 화면에 렌더링 (Map 정보 포함, 스위치/도어 상태 확인)
void render_tile_with_map(TileType tile, int screen_x, int screen_y, const Map* map, int map_x, int map_y) {
    console_set_cursor_position(screen_x * 2, screen_y); // 타일당 2칸 사용
    
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
            
        case TILE_POISON_TERRAIN:
            // 독 지형 - 초록색 배경
            console_set_color(COLOR_YELLOW, COLOR_GREEN);
            printf("☠ ");
            break;
            
        case TILE_BOX:
            // 상자
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("▢ ");
            break;
            
        case TILE_SWITCH:
            // 플레이어 스위치는 활성화 상태에 따라 색상 변경
            if (map && map_x >= 0 && map_y >= 0) {
                int switch_idx = map_find_switch(map, map_x, map_y);
                if (switch_idx >= 0 && map_is_switch_activated(map, switch_idx)) {
                    // 활성화됨: 밝은 초록색
                    console_set_color(COLOR_GREEN, COLOR_GREEN);
                    printf("● ");
                } else {
                    // 비활성화: 어두운 초록색
                    console_set_color(COLOR_GREEN, COLOR_BLACK);
                    printf("○ ");
                }
            } else {
                // 기본 색상 (Map 정보 없을 때)
                console_set_color(COLOR_GREEN, COLOR_BLACK);
                printf("○ ");
            }
            break;
            
        case TILE_BOX_SWITCH:
            // 상자 스위치는 활성화 상태에 따라 색상 변경 (파란색으로 구분)
            if (map && map_x >= 0 && map_y >= 0) {
                int switch_idx = map_find_switch(map, map_x, map_y);
                if (switch_idx >= 0 && map_is_switch_activated(map, switch_idx)) {
                    // 활성화됨: 밝은 파란색
                    console_set_color(COLOR_BLUE, COLOR_BLUE);
                    printf("● ");
                } else {
                    // 비활성화: 어두운 파란색
                    console_set_color(COLOR_BLUE, COLOR_BLACK);
                    printf("○ ");
                }
            } else {
                // 기본 색상 (Map 정보 없을 때)
                console_set_color(COLOR_BLUE, COLOR_BLACK);
                printf("○ ");
            }
            break;
            
        case TILE_VERTICAL_WALL:
            console_set_color(COLOR_GREEN, COLOR_BLACK);
            printf("█ "); // 수직 벽 (초록색)
            break;
            
        case TILE_MOVING_PLATFORM:
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("▄▄"); // 이동 발판 (노란색, 세로)
            break;
            
        case TILE_HORIZONTAL_PLATFORM:
            console_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("▄▄"); // 이동 발판 (노란색, 가로)
            break;
            
        case TILE_FIRE_GEM:
            // Fireboy 전용 보석 (다이아몬드 모양, 빨간색 전경만)
            console_set_color(COLOR_RED, COLOR_BLACK);
            printf("◆ ");
            break;
            
        case TILE_WATER_GEM:
            // Watergirl 전용 보석 (다이아몬드 모양, 파란색 전경만)
            console_set_color(COLOR_CYAN, COLOR_BLACK);
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
            // 스위치/도어는 상태가 바뀔 수 있으므로 항상 체크
            bool needs_render = false;
            if (!prev_frame_buffer || prev_frame_buffer[buffer_idx] != current_tile) {
                needs_render = true;
            } else if (current_tile == TILE_SWITCH || current_tile == TILE_BOX_SWITCH) {
                // 스위치는 타일 타입이 같아도 상태가 바뀔 수 있음
                needs_render = true;
            }
            
            if (needs_render) {
                render_tile_with_map(current_tile, x, y, map, map_x, map_y);
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
            // 스위치/도어는 상태가 바뀔 수 있으므로 항상 체크
            bool needs_render = false;
            if (!prev_frame_buffer || prev_frame_buffer[buffer_idx] != current_tile) {
                needs_render = true;
            } else if (current_tile == TILE_SWITCH || current_tile == TILE_BOX_SWITCH) {
                // 스위치는 타일 타입이 같아도 상태가 바뀔 수 있음
                needs_render = true;
            }
            
            if (needs_render) {
                render_tile_with_map(current_tile, x, y, map, map_x, map_y);
                if (prev_frame_buffer) {
                    prev_frame_buffer[buffer_idx] = current_tile;
                }
            }
        }
    }

    // 토글 플랫폼 렌더링 (잔상 제거 포함)
    if (map) {
        // 1단계: 이전 위치 지우기
        for (int i = 0; i < map->toggle_platform_count; i++) {
            if (!prev_toggle_platform_valid[i]) continue;
            
            int px = map->toggle_platforms[i].x;
            int width = map->toggle_platforms[i].width;
            int old_py = prev_toggle_platform_y[i];
            int new_py = (int)roundf(map->toggle_platforms[i].y);
            
            // 위치가 바뀌었으면 이전 위치 지우기
            if (old_py != new_py) {
                for (int w = 0; w < width; w++) {
                    int sx = (px + w) - camera_x;
                    int sy = old_py - camera_y;
                    
                    if (sx >= 0 && sx < tiles_per_row && sy >= 0 && sy < screen_height - 1) {
                        TileType original_tile = map_get_tile(map, px + w, old_py);
                        render_tile_with_map(original_tile, sx, sy, map, px + w, old_py);
                    }
                }
            }
        }
        
        // 2단계: 현재 위치에 그리기
        for (int i = 0; i < map->toggle_platform_count; i++) {
            int px = map->toggle_platforms[i].x;
            int py = (int)roundf(map->toggle_platforms[i].y);
            int width = map->toggle_platforms[i].width;
            
            for (int w = 0; w < width; w++) {
                int sx = (px + w) - camera_x;
                int sy = py - camera_y;
                
                if (sx >= 0 && sx < tiles_per_row && sy >= 0 && sy < screen_height - 1) {
                    console_set_cursor_position(sx * 2, sy);
                    console_set_color(COLOR_MAGENTA, COLOR_BLACK);
                    printf("▄▄");
                    console_reset_color();
                }
            }
            
            // 현재 위치를 다음 프레임의 "이전 위치"로 저장
            prev_toggle_platform_y[i] = py;
            prev_toggle_platform_valid[i] = true;
        }
    }
    
    // 이동 발판 오버레이 렌더링 (잔상 제거 포함)
    if (map) {
        // 1단계: 이전 프레임의 발판 위치를 원래 타일로 복구 (잔상 제거)
        for (int i = 0; i < map->platform_count; i++) {
            if (!prev_platform_valid[i]) continue;
            
            int old_px = prev_platform_x[i];
            int old_py = prev_platform_y[i];
            int old_sx = old_px - camera_x;
            int old_sy = old_py - camera_y;
            
            // 현재 위치와 다른 경우에만 이전 위치를 지움
            int new_px = map->platforms[i].active ? (int)roundf(map->platforms[i].x) : -1;
            int new_py = map->platforms[i].active ? (int)roundf(map->platforms[i].y) : -1;
            
            if (old_px != new_px || old_py != new_py) {
                if (old_sx >= 0 && old_sx < tiles_per_row && old_sy >= 0 && old_sy < screen_height - 1) {
                    // 이전 발판 위치에 원래 타일(EMPTY)을 그려서 잔상 제거
                    TileType original_tile = map_get_tile(map, old_px, old_py);
                    render_tile_with_map(original_tile, old_sx, old_sy, map, old_px, old_py);
                }
            }
        }
        
        // 2단계: 현재 프레임의 발판 위치에 발판 그리기
        for (int i = 0; i < map->platform_count; i++) {
            if (!map->platforms[i].active) {
                prev_platform_valid[i] = false;
                continue;
            }
            
            int px = (int)roundf(map->platforms[i].x);
            int py = (int)roundf(map->platforms[i].y);
            int sx = px - camera_x;
            int sy = py - camera_y;
            
            if (sx >= 0 && sx < tiles_per_row && sy >= 0 && sy < screen_height - 1) {
                // vertical 플래그에 따라 다른 타일로 렌더링
                TileType platform_tile = map->platforms[i].vertical ? 
                    TILE_MOVING_PLATFORM : TILE_HORIZONTAL_PLATFORM;
                render_tile_with_map(platform_tile, sx, sy, map, px, py);
            }
            
            // 현재 위치를 다음 프레임의 "이전 위치"로 저장
            prev_platform_x[i] = px;
            prev_platform_y[i] = py;
            prev_platform_valid[i] = true;
        }
    }
    
    // 보석 오버레이 렌더링 (gems 배열 사용, 수집되지 않은 것만)
    if (map) {
        for (int i = 0; i < map->gem_count; i++) {
            if (map->gems[i].collected) continue;
            
            int gx = map->gems[i].x;
            int gy = map->gems[i].y;
            int sx = gx - camera_x;
            int sy = gy - camera_y;
            
            if (sx >= 0 && sx < tiles_per_row && sy >= 0 && sy < screen_height - 1) {
                // 보석을 오버레이로 그림
                render_tile_with_map(map->gems[i].type, sx, sy, map, gx, gy);
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
    
    // 플레이어 렌더링 (기본 심볼로 복구)
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
