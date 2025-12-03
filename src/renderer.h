#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "map.h"
#include "console.h"

// 렌더러 함수 선언
void renderer_init(int screen_width, int screen_height);
void renderer_cleanup(void);
void render_map(const Map* map, int camera_x, int camera_y);
void render_tile(TileType tile, int x, int y);
void render_map_no_flicker(const Map* map, int camera_x, int camera_y);

#endif // RENDERER_H

