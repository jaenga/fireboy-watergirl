#include "ranking.h"
#include "console.h"
#include "input.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 랭킹 시스템 초기화
void ranking_init(RankingSystem* system) {
    if (!system) return;
    system->count = 0;
    for (int i = 0; i < MAX_RANKING_ENTRIES; i++) {
        memset(system->entries[i].name, 0, MAX_NAME_LENGTH);
        system->entries[i].clear_time = 0.0f;
        system->entries[i].deaths = 0;
        system->entries[i].date = 0.0f;
    }
}

// 랭킹 파일에서 로드
bool ranking_load(RankingSystem* system, const char* filename) {
    if (!system || !filename) return false;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        // 파일이 없으면 새로 시작
        ranking_init(system);
        return true;
    }
    
    size_t read = fread(system, sizeof(RankingSystem), 1, file);
    fclose(file);
    
    return (read == 1);
}

// 랭킹 파일에 저장
bool ranking_save(const RankingSystem* system, const char* filename) {
    if (!system || !filename) return false;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    
    size_t written = fwrite(system, sizeof(RankingSystem), 1, file);
    fclose(file);
    
    return (written == 1);
}

// 랭킹 정렬 비교 함수 (빠른 시간 순)
static int compare_entries(const void* a, const void* b) {
    const RankingEntry* entry_a = (const RankingEntry*)a;
    const RankingEntry* entry_b = (const RankingEntry*)b;
    
    if (entry_a->clear_time < entry_b->clear_time) return -1;
    if (entry_a->clear_time > entry_b->clear_time) return 1;
    return 0;
}

// 새로운 랭킹 추가
void ranking_add_entry(RankingSystem* system, const char* name, float clear_time, int deaths) {
    if (!system || !name) return;
    
    // 새 엔트리 생성
    RankingEntry new_entry;
    strncpy(new_entry.name, name, MAX_NAME_LENGTH - 1);
    new_entry.name[MAX_NAME_LENGTH - 1] = '\0';
    new_entry.clear_time = clear_time;
    new_entry.deaths = deaths;
    new_entry.date = (float)time(NULL);
    
    // 랭킹 추가
    if (system->count < MAX_RANKING_ENTRIES) {
        system->entries[system->count] = new_entry;
        system->count++;
    } else {
        // 꽉 찼으면 마지막 엔트리 교체
        system->entries[MAX_RANKING_ENTRIES - 1] = new_entry;
    }
    
    // 정렬 (빠른 시간 순)
    qsort(system->entries, system->count, sizeof(RankingEntry), compare_entries);
    
    // 파일에 저장
    ranking_save(system, "rankings.dat");
}

// 랭킹 표시
void ranking_display(const RankingSystem* system) {
    if (!system) return;
    
    console_clear();
    console_set_cursor_position(0, 2);
    
    // 제목
    console_set_color(COLOR_YELLOW, COLOR_BLACK);
    console_set_attribute(ATTR_BOLD);
    printf("                      🏆 TOP 10 랭킹 🏆\n\n");
    console_reset_color();
    
    // 헤더
    console_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("     순위   이름              시간      사망   \n");
    printf("     ────────────────────────────────────────\n");
    console_reset_color();
    
    // 랭킹 리스트
    if (system->count == 0) {
        printf("\n           아직 기록이 없습니다!\n");
    } else {
        for (int i = 0; i < system->count; i++) {
            int minutes = (int)(system->entries[i].clear_time / 60.0f);
            int seconds = (int)(system->entries[i].clear_time) % 60;
            
            // 순위에 따라 색상 다르게
            if (i == 0) {
                console_set_color(COLOR_YELLOW, COLOR_BLACK);
                console_set_attribute(ATTR_BOLD);
                printf("     🥇 ");
            } else if (i == 1) {
                console_set_color(COLOR_WHITE, COLOR_BLACK);
                printf("     🥈 ");
            } else if (i == 2) {
                console_set_color(COLOR_YELLOW, COLOR_BLACK);
                printf("     🥉 ");
            } else {
                console_reset_color();
                printf("     %2d ", i + 1);
            }
            
            printf("  %-16s  %2d:%02d    %3d회\n", 
                   system->entries[i].name,
                   minutes, seconds,
                   system->entries[i].deaths);
            console_reset_color();
        }
    }
    
    printf("\n\n");
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
