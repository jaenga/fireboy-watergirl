#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "map.h"
#include "console.h"
#include "player.h"

// 렌더러 함수 선언
void renderer_init(int screen_width, int screen_height);
void renderer_cleanup(void);
void render_map(const Map* map, int camera_x, int camera_y);
void render_tile(TileType tile, int x, int y);
void render_map_no_flicker(const Map* map, int camera_x, int camera_y);
void render_map_no_flicker_with_players(const Map* map, int camera_x, int camera_y, 
                                        int player1_x, int player1_y, 
                                        int player2_x, int player2_y);
void render_player(const Player* player, int camera_x, int camera_y);

#endif // RENDERER_H

