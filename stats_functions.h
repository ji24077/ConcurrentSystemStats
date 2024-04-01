// stats_functions.h
#ifndef STATS_FUNCTIONS_H
#define STATS_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <utmp.h>
#include <errno.h>

// 구조체 정의
typedef struct {
    double total_memory;
    double used_memory;
    double total_virtual;
    double used_virtual;
} memory_info;

typedef struct {
    long int user;
    long int nice;
    long int system;
    long int idle;
    long int iowait;
    long int irq;
    long int softirq;
} cpu_info;

// 함수 프로토타입
void signalHandler(int sig);
void displaySystemStats(int samples, int tdelay, bool systemFlag, bool userFlag, bool graphicsFlag, bool sequentialFlag);
void collectMemoryStats(int pipefd[2]);
void collectCPUStats(int pipefd[2]);
void collectUserStats(int pipefd[2]);
void printMemoryStats(const memory_info *memInfo, bool graphics, int iteration);
void printCPUStats(const cpu_info *cpuInfo, bool graphics, int iteration);
void printUserStats(int pipefd[2]);

#endif // STATS_FUNCTIONS_H
