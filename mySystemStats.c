#include "stats_functions.h"

void GetInfoTop(int samples, int tdelay,int sequential,int i) 
{
    /*Print the Nbr of samples: 10 --  every 1 secs.
     and Memory usage:4092 kilobytes. this is just a example of Top info.
    */
    
    //printf("go to GEtinfotop");
    //for memory usage:
    struct rusage usage_info; // from <sys/resource.h>, to use mAgetrusage();
    
    int result = getrusage(RUSAGE_SELF,&usage_info);
    if(sequential){
        printf(">>> iteration %d\n",i);
    }
    else{
        printf("\033[H\033[2J");
        printf("Nbr of samples: %d-- every %d secs\n", samples,tdelay);
    }
    if(result ==0){
        printf("Memory usage: %ld kilobytes\n", usage_info.ru_maxrss);
    }
   else{
    printf("fail to get Resource usage info");
   }
}


void storeMemArr(int samples,int memFD[2],int tdelay){//memFD[1]
    /*display/print Memo*/
    //아마도 array에 저장해 sequentially 다룰 필요.
    //second information.
    char memArr[samples][1024];
    struct sysinfo sys_info;
    for(int i=0; i<samples;i++){
            //struct sysinfo sys_info;

        sysinfo(&sys_info); //get the information of system.(such as memory usage)
        double phys_total_gb = (float)sys_info.totalram/1024/1024/1024; 
        double phys_free_gb = (float)sys_info.freeram/1024/1024/1024;
        double phys_used_gb = (float)phys_total_gb-phys_free_gb;
        double swap_total_gb = (float)sys_info.totalswap/1024/1024/1024;
        double swap_free_gb = (float)sys_info.freeswap/1024/1024/1024;

        double virtual_used_gb = phys_used_gb+(swap_total_gb-swap_free_gb);
        double virtual_total_gb =(phys_total_gb+swap_total_gb);
        sprintf(memArr[i],"%.2f GB / %.2f GB  -- %.2f GB / %.2f GB",
        phys_used_gb,phys_total_gb,virtual_used_gb,virtual_total_gb);
        ssize_t bytes_written = write(memFD[1], &memArr[i], sizeof(memArr[i]));

        sleep(tdelay);
        // sleep(tdelays);
        //if i write it here, then when i read it is it also tdleay of speed to read it\?

        
    }
    // for (int i = 0; i < samples; i++) {
            
        
    //     //ssize_t bytes_written = write(memFD[1], &memArr[i], sizeof(memArr[i]));
    //     ssize_t bytes_written = write(memFD[1], &memArr[i], sizeof(memArr[i]));

    // if (bytes_written == -1) {
    //     perror("Failed to write memory info to pipe");
    // }
    // }
}

void fcnForPrintMemoryArr(int sequential,int samples,char memArr[][1024],int i,int memFD[2]){//fd[0]
    //printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

   // char buffer[1024];
   // while(read(memFD[0],memArr[i], sizeof(memArr[i])) > 0) {
       // printf("메모리 사용량: %s\n", buffer);
        // if (i == 0) {
        // //printf("i is for fcnprint: %d\n",i);
        //   printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
        // }
        if(sequential){
            for(int k=0; k<samples; k++){
                //ssize_t bytes_read = read(memFD,&memArr[k],1024); //may do this on main.
                if(k==i){
                    
                    printf("%s\n",memArr[k]);
                    //sleep(tdelay);
                    
                }
                else{
                    printf("\n");
                }
            }
        }
        else{
            
            for(int j=0; j<=i; j++){

                printf("%s\n",memArr[j]);
                //sleep(tdelay);
            }
            
        }
    //}
}


void memoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][1024], int i) {
    double difference = virtual_used_gb - *prev_used_gb;
    char graphicsStr[1024] = "\0 ";
    char diff_virtArr[1024]="\0"; // Initialize with a space for proper formatting
    char infoStr[100]; // Buffer for the formatted information
    // Base representation for the first sample or minimal change
    if (i == 0 || fabs(difference) < 0.01) {
        snprintf(graphicsStr, sizeof(graphicsStr), "|%s %.2f (%.2f)", difference >= 0 ? "o" : "@", difference, virtual_used_gb);
    } else {
        // Prepare graphics based on the magnitude and direction of change
        char changeSymbol = difference < 0 ? ':' : '#';
        int symbolsCount = fabs(difference) * 100; // Convert change to symbol count

        // Append the base bars
        strcat(graphicsStr, "|");
        // Append additional symbols based on change magnitude
        for (int j = 0; j < symbolsCount && j < (sizeof(graphicsStr) - strlen(graphicsStr) - 50); ++j) {
            strncat(graphicsStr, &changeSymbol, 1);
        }
        // Append closing symbol
        strcat(graphicsStr, difference < 0 ? "@" : "*");
        
        // Append the formatted difference and usage
        snprintf(infoStr, sizeof(infoStr), " %.2f (%.2f)", difference, virtual_used_gb); //this part seems wrong
        strncat(graphicsStr, infoStr, sizeof(graphicsStr) - strlen(graphicsStr) - 1);
    }

    // Ensure not to exceed buffer limit
    strcat(memArr[i], graphicsStr);
    memArr[i][1023] = '\0'; // Null-terminate to ensure string is properly closed

    // Update previous usage for next call
    *prev_used_gb = virtual_used_gb;
     
}

            
void storeUserInfoThird(int userFD[2]) {
    struct utmp *utmp_info;
    char buffer[1024]=""; // Ensure this is large enough for the data
    
    setutent(); // Open the utmp file

    while ((utmp_info = getutent()) != NULL) { // Read until null
        if (utmp_info->ut_type == USER_PROCESS) { // Check ut_type is normal process
            // Clear the buffer for each user session
            //memset(buffer, 0, sizeof(buffer));

            // Check if ut_host has a value and format the string accordingly
            if (strlen(utmp_info->ut_host) > 0) {
                snprintf(buffer, sizeof(buffer), "%s\t%s\t(%s)\n", utmp_info->ut_user, utmp_info->ut_line, utmp_info->ut_host);
            } 
            // else {
            //     snprintf(buffer, sizeof(buffer), "%s\t%s\n", utmp_info->ut_user, utmp_info->ut_line);
            // }

            // Write the formatted string to the pipe
            
        }
    }
    ssize_t bytes_written = write(userFD[1], buffer, sizeof(char)*1024);
            if (bytes_written == -1) {
                perror("Failed to write user session info to pipe");
                // Consider handling the error, e.g., by breaking out of the loop or other measures
            }

    endutent(); // Closes the internal stream of the utmp database
    close(userFD[1]); // Close the write-end of the pipe
}


int printUserInfoThird(int userFD[2]) {
    char buffer[1024]=""; // 데이터를 읽어올 버퍼
    ssize_t bytesRead; // read 함수로부터 읽은 바이트 수를 저장
    int userLine_count = 0; // Counter for user lines

    printf("### Sessions/users ###\n");
    // 파이프의 읽기 끝을 사용하여 데이터 읽기 시도
    if((bytesRead = read(userFD[0], buffer, sizeof(char)*1024)) > 0) {
        buffer[bytesRead] = '\0'; // 문자열의 끝을 나타내기 위해 널 종료자 추가
        //printf("for loop overf?\n");
        // printf("bytesread:%ld\n",bytesRead);
        printf("%s\n", buffer); // 읽어온 데이터(사용자 정보) 출력
        for (ssize_t i = 0; i < bytesRead; ++i) {
            //printf("for loop?\n");
            if (buffer[i] == '\n') {
                userLine_count++;
                //printf("userline =%d\n",userLine_count);
            }
        }
        // printf("for loop overf fr??\n");
        if (bytesRead == -1) {
        // 읽기 중 오류 발생
            perror("Failed to read from pipe");
            exit(EXIT_FAILURE);
        }
    }
    // printf("while loop over fr??\n");

    
    close(userFD[0]); // 파이프의 읽기 끝 닫기
    return userLine_count; // Return the count of user lines processed
}



void printCores(){
    //fourth information.
    int num_cpu = sysconf(_SC_NPROCESSORS_ONLN); //sysconf returns the number of processors in <unistd.h>
    printf("Number of cores: %d\n",num_cpu);
     //print num_cores
}
void storeCpuArr(int cpuFD[2]) {//fd[1]
    //fifth information, print with fourth information.
    unsigned long currCpuUsage[7];
    FILE *fp = fopen("/proc/stat", "r");

    if (!fp) {
        perror("Error for opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    
    if (fscanf(fp, "cpu %lu %lu %lu %lu %lu %lu %lu", 
    &currCpuUsage[0], &currCpuUsage[1], &currCpuUsage[2], &currCpuUsage[3], &currCpuUsage[4], &currCpuUsage[5], &currCpuUsage[6]) != 7){ 
        fprintf(stderr, "Error reat.ding CPU values\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(cpuFD[1], &currCpuUsage, sizeof(currCpuUsage));
    if (bytes_written == -1) {
        perror("Error writing to pipe from storeCpuArr");
        close(cpuFD[1]); // Close the pipe on error
        exit(EXIT_FAILURE);
    }

    // Close the write end of the pipe to signal EOF to the reader
    //close(cpuFD[1]);
    fclose(fp); //mendatory
}



double calculateCpuUsage(unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]) {
    // unsigned long prevCpuUsage[7];
    // unsigned long currCpuUsage[7];
    unsigned long idle_prev = prevCpuUsage[3] + prevCpuUsage[4]; // idle_prev = idle+iowait. i wrote clearly on notion.
    unsigned long idle_cur = currCpuUsage[3] + currCpuUsage[4]; //similarly,

    unsigned long total_prev = 0;
    unsigned long total_cur = 0;
    for (int i = 0; i < 7; ++i) {
        total_prev += prevCpuUsage[i];
        total_cur += currCpuUsage[i];
    }

    double total_diff = (double)(total_cur - total_prev);
    double idle_diff = (double)(idle_cur - idle_prev);

    return (total_diff - idle_diff) / total_diff * 100.0; // for persentage converted.// from the internet. need confirm.
}
void printCpuUsageAndGraphics(int cpuPFD[2], int cpuCFD[2], int sequential, int i,int graphics) {
    unsigned long prevCpuUsage[7], currCpuUsage[7];
    

    read(cpuPFD[0], prevCpuUsage, sizeof(prevCpuUsage));
    read(cpuCFD[0], currCpuUsage, sizeof(currCpuUsage));

    double cur_cpuUsage = calculateCpuUsage(prevCpuUsage, currCpuUsage);
    printf("Total CPU use: %.2f%%\n", cur_cpuUsage);

    // if (graphics) {
    //     // Use cur_cpuUsage to generate graphics
    //     setCpuGraphics(sequential, cpuArr, &default_num, cur_cpuUsage, &prevCpuUsageUsage, i);
    // }
    
}

void setCpuGraphics(int sequential,char cpuArr[][200],int *default_num,float curCpuUsage, float *prevCpuUsage,int sampleIndex){
    int baseBarCount = 3; // 기본 바 개수는 3으로 고정
    int additionalBars; // CPU 사용량에 따라 추가되는 바의 개수를 결정할 변수

    // 첫 번째 샘플인 경우, default_num을 초기화하고 기본 바를 설정합니다.
    if (sampleIndex == 0) {
        *default_num = baseBarCount; // 기본 바 개수로 초기화
        additionalBars = (int)curCpuUsage; // 현재 CPU 사용량에 따른 추가 바 개수 계산
    } else {
        // 이전 CPU 사용량과 현재 CPU 사용량의 차이를 계산하여 추가 바 개수를 결정합니다.
        additionalBars = (int)curCpuUsage - (int)(*prevCpuUsage);
    }

    // CPU 사용 정보를 담을 문자열을 초기화합니다.
    char cpuUsageStr[200] = "         "; // 시작 공백을 포함하여 초기화
    *default_num += additionalBars; // 추가 바 개수를 반영하여 전체 바 개수를 업데이트합니다.
    for (int i = 0; i < *default_num; ++i) {
        strcat(cpuUsageStr, "|"); // 각 바를 문자열에 추가합니다.
    }

    // CPU 사용량(%)을 문자열에 추가합니다.
    char usagePercent[50];
    // 완성된 문자열을 cpuArr에 저장합니다.
    strcpy(cpuArr[sampleIndex], cpuUsageStr);

    // sequential 모드에 따라 출력 방식을 조정합니다.
    if (sequential) {
        // sequential 모드가 활성화된 경우, 지금까지의 모든 CPU 사용 정보를 순차적으로 출력합니다.
        for (int j = 0; j <= sampleIndex; j++) {
            printf("%s\n", cpuArr[j]);
        }
    } else {
        for(int k=0; k<=sampleIndex; k++){ //iterate through cpuArr upto the current iteration index i from main
            printf("%s\n", cpuArr[k]); //prints the entire cpuArr upto and including the current iteration
        }
    }

    // 현재 CPU 사용량을 이전 사용량으로 업데이트합니다.
    *prevCpuUsage = curCpuUsage;
}

void printSystemInfoLast(){
    struct utsname sysinfo;
    FILE *uptime_file;
    double uptime_secs;
    uptime_file = fopen("/proc/uptime","r"); //read file uptime.
    fscanf(uptime_file, "%lf", &uptime_secs);
    fclose(uptime_file);
    
    // Convert to days, hours, minutes, and seconds
    int days = uptime_secs / (24 * 3600);
    uptime_secs = uptime_secs - (days * 24 * 3600);
    int hours = uptime_secs / 3600;
    uptime_secs = uptime_secs - (hours * 3600);
    int minutes = uptime_secs / 60;
    int seconds = (int)uptime_secs % 60;
    int days_to_hr = 24*days;
    int Total_hr = days_to_hr + hours; 
    if(uname(&sysinfo) == 0 && uptime_file != NULL){ // if 정보를 가져오는데 성공하면.
        printf("### System Information ###\n");
        printf("it goes here somehow\n");
        printf("System Name =%s\n",sysinfo.sysname );
        printf("Machine Name=%s\n", sysinfo.nodename);
        printf("Version=%s\n", sysinfo.version);
        printf("Release=%s\n", sysinfo.release);
        printf("Architecture=%s\n", sysinfo.machine);
        printf("System running since last reboot: %d days %02d:%02d:%02d (%02d:%02d:%02d)\n",
         days, hours, minutes, seconds,Total_hr,minutes,seconds);
        
    }
    else{
        //printf("damn really ");
        perror("Error opening /proc/uptime");
        return;
    }
}
    
double calculateVirtUsed(){
    struct sysinfo sys_info;

    sysinfo(&sys_info); //get the information of system.(such as memory usage)
    double phys_total_gb = (float)sys_info.totalram/1024/1024/1024; 
    double phys_free_gb = (float)sys_info.freeram/1024/1024/1024;
    double phys_used_gb = (float)phys_total_gb-phys_free_gb;
    double swap_total_gb = (float)sys_info.totalswap/1024/1024/1024;
    double swap_free_gb = (float)sys_info.freeswap/1024/1024/1024;

    double virtual_used_gb = phys_used_gb+(swap_total_gb-swap_free_gb);
    
    return virtual_used_gb; 
}
void reserve_space(int samples){
    
    for(int i=0;i<samples+1; i++){
        printf("\n");
    }
}