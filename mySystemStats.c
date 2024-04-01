// mySystemStats.c
#include "stats_functions.h"


void signalHandler(int sig) {
    // 신호 처리 로직 구현
}

int main(int argc, char *argv[]) {
    // 명령줄 인자 처리 로직 구현

    // 시스템 상태를 출력하는 함수 호출
    displaySystemStats(samples, tdelay, systemFlag, userFlag, graphicsFlag, sequentialFlag);

    return 0;
}
