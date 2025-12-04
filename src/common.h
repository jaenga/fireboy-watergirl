#ifndef COMMON_H
#define COMMON_H

// 플랫폼 감지
#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <conio.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    #define PLATFORM_UNIX
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

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
#define MAX_DOORS 20      // 한 스테이지에서 관리할 수 있는 최대 도어 개수
#define MAX_PLATFORMS 20  // 한 스테이지에서 관리할 수 있는 최대 이동 발판 개수

#endif // COMMON_H