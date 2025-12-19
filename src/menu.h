#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

#define MAX_NAME_LENGTH 32

// 메뉴 결과 구조체
typedef struct {
    bool start_game;
    bool exit_game;
    char player_name[MAX_NAME_LENGTH];
} MenuResult;

// 메인 메뉴 표시 및 선택 처리
MenuResult menu_show_main(void);

// 게임 설명 화면
void menu_show_instructions(void);

// 랭킹 화면
void menu_show_ranking(void);

// 플레이어 이름 입력
bool menu_get_player_name(char* name, int max_length);

// 최종 결과 화면 (모든 스테이지 클리어시)
// stage_times: 각 스테이지별 클리어 시간 배열 
void menu_show_final_result(float stage_times[], float total_time, int total_deaths, int total_fire_gems, int total_water_gems);

#endif 
