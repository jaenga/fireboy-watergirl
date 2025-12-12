#ifndef RANKING_H
#define RANKING_H

#include <stdbool.h>
#include <time.h>

#define MAX_RANKING_ENTRIES 10
#define MAX_NAME_LENGTH 32

// 랭킹 엔트리 구조체
typedef struct {
    char name[MAX_NAME_LENGTH];
    float clear_time;  // 초 단위
    int deaths;
    float date;  // timestamp
} RankingEntry;

// 랭킹 시스템 구조체
typedef struct {
    RankingEntry entries[MAX_RANKING_ENTRIES];
    int count;
} RankingSystem;

// 랭킹 시스템 초기화
void ranking_init(RankingSystem* system);

// 랭킹 파일에서 로드
bool ranking_load(RankingSystem* system, const char* filename);

// 랭킹 파일에 저장
bool ranking_save(const RankingSystem* system, const char* filename);

// 새로운 랭킹 추가
void ranking_add_entry(RankingSystem* system, const char* name, float clear_time, int deaths);

// 랭킹 표시
void ranking_display(const RankingSystem* system);

#endif // RANKING_H
