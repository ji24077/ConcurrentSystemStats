#ifndef STATS_FUNCTIONS_H
#define STATS_FUNCTIONS_H

#include <sys/resource.h>
#include <utmp.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <utmpx.h>
#include <sys/resource.h>
#include <utmp.h>
#include <sys/utsname.h>
#include <math.h>
#include <getopt.h>
// Struct for memory information (you might already have similar or need adjustments based on your implementation needs)
typedef struct {
    double total_memory;
    double used_memory;
    double total_swap;
    double used_swap;
} memory_info;

// Struct for CPU usage information
typedef struct {
    unsigned long user_time;
    unsigned long system_time;
    unsigned long idle_time;
} cpu_info;

// Function to initialize signal handling
void initializeSignalHandling(void);

// Functions related to collecting and displaying system statistics
void GetInfoTop(int samples, int tdelay, int sequential, int i);
void storeMemArr(char arr[][1024], int i, int memFD[2]);
void fcnForPrintMemoryArr(int sequential, int samples, char memArr[][1024], int i);
void memoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][1024], int i);
int printUserInfoThird(int userFD[2]);
void printCores(void);
// void storeCpuArr(unsigned long currCpuUsage[7], int cpuFD[2]);
void storeCpuArr(int cpuFD[2]);
double calculateCpuUsage(unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]);
void printSystemInfoLast(void);

#endif // STATS_FUNCTIONS_H
