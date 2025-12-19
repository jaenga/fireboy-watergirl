#ifndef COMMON_H
#define COMMON_H

// 플랫폼 감지 (Unix/macOS/Linux만 지원)
#define PLATFORM_UNIX
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// 기본 상수
#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 50
#define MAX_STAGE_NAME_LENGTH 50

// 오브젝트 관련 상수
#define MAX_BOXES 50      // 한 스테이지에서 관리할 수 있는 최대 상자 개수
#define MAX_SWITCHES 20   // 한 스테이지에서 관리할 수 있는 최대 스위치 개수
#define MAX_PLATFORMS 20  // 한 스테이지에서 관리할 수 있는 최대 이동 발판 개수

// 음악 재생 함수 선언
void music_play(const char* filename);
void music_stop(void);

#endif // COMMON_H