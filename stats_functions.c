#include "stats_functions.h"
void GetInfoTop(int samples, int tdelay,int sequential,int i) 
{
    /*Print the Nbr of samples: 10 --  every 1 secs.
     and Memory usage:4092 kilobytes. this is just a example of Top info.
    */
    
    
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
void storeMemArr(char arr[][1024], int i, int memFD[2]) {
    struct sysinfo sys_info;

    // Collect system information
    if (sysinfo(&sys_info) != 0) {
        fprintf(stderr, "Error: sysinfo failed, %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE); // Exit if sysinfo call fails
    }

    double phys_total_gb = (double)sys_info.totalram / (1024 * 1024 * 1024);
    double phys_free_gb = (double)sys_info.freeram / (1024 * 1024 * 1024);
    double swap_total_gb = (double)sys_info.totalswap / (1024 * 1024 * 1024);
    double swap_free_gb = (double)sys_info.freeswap / (1024 * 1024 * 1024);
    double phys_used_gb = phys_total_gb - phys_free_gb;
    double swap_used_gb = swap_total_gb - swap_free_gb;
    double virtual_used_gb = phys_used_gb + swap_used_gb;

    // Format memory information
    snprintf(arr[i], 1024, "%.2f GB / %.2f GB -- %.2f GB / %.2f GB",
             phys_used_gb, phys_total_gb, virtual_used_gb, phys_total_gb + swap_total_gb);

    // Write the formatted string to the pipe
    ssize_t bytes_written = write(memFD[1], arr[i], strlen(arr[i]) + 1); // +1 for NULL terminator
    if (bytes_written == -1) {
        perror("Error writing to pipe");
        exit(EXIT_FAILURE);
    }
}


void fcnForPrintMemoryArr(int sequential,int samples,char memArr[][1024],int i){
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    if(sequential){
        for(int k=0; k<samples; k++){
            if(k==i){
                
                printf("%s\n",memArr[k]);
                
            }
            else{
                printf("\n");
            }
        }
    }
    else{
        for(int j=0; j<=i; j++){
            printf("%s\n",memArr[j]);
        }
        
    }
}
void memoryGraphics(double virtual_used_gb, double *prev_used_gb, char memArr[][1024], int i) {
    //for second information stil.
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
int printUserInfoThird(int userFD[2]) {
    struct utmp *utmp_info;

    // Open the UTMP file
    setutent();

    int userLine_count = 0;

    printf("### Sessions/users ###\n");

    while ((utmp_info = getutent()) != NULL) { // Read until null
        if (utmp_info->ut_type == USER_PROCESS) { // Check ut_type is normal process
            char userInfo[1024]; // Buffer to store user info string
            
            // Format user information string
            snprintf(userInfo, sizeof(userInfo), "%s\t%s\t%s\n",
                     utmp_info->ut_user,
                     utmp_info->ut_line,
                     utmp_info->ut_host[0] ? utmp_info->ut_host : "");

            // Write the formatted string to the pipe
            ssize_t bytes_written = write(userFD[1], userInfo, strlen(userInfo));
            if (bytes_written == -1) {
                perror("Error writing to pipe");
                close(userFD[1]); // Close the pipe on error
                exit(EXIT_FAILURE);
            }

            userLine_count++;
        }
    }

    endutent(); // Closes the internal stream of the UTMP database

    // It's important to close the write end of the pipe to signal EOF to the reader
    close(userFD[1]);

    return userLine_count; // Return the count of user lines processed
}

void printCores(){
    //fourth information.
    int num_cpu = sysconf(_SC_NPROCESSORS_ONLN); //sysconf returns the number of processors in <unistd.h>
    printf("Number of cores: %d\n",num_cpu);
     //print num_cores
}
void storeCpuArr(int cpuFD[2]) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    unsigned long cpuUsage[7];
    if (fscanf(fp, "cpu %lu %lu %lu %lu %lu %lu %lu", 
               &cpuUsage[0], &cpuUsage[1], &cpuUsage[2], &cpuUsage[3], 
               &cpuUsage[4], &cpuUsage[5], &cpuUsage[6]) != 7) {
        fprintf(stderr, "Error reading CPU values\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    // Format the CPU usage data into a string to write to the pipe
    char cpuInfo[256]; // Ensure the buffer is large enough
    snprintf(cpuInfo, sizeof(cpuInfo), "%lu %lu %lu %lu %lu %lu %lu\n",
             cpuUsage[0], cpuUsage[1], cpuUsage[2], cpuUsage[3],
             cpuUsage[4], cpuUsage[5], cpuUsage[6]);

    // Write the formatted string to the pipe
    ssize_t bytes_written = write(cpuFD[1], cpuInfo, strlen(cpuInfo));
    if (bytes_written == -1) {
        perror("Error writing to pipe");
        close(cpuFD[1]); // Close the pipe on error
        exit(EXIT_FAILURE);
    }

    // It's important to close the write end of the pipe to signal EOF to the reader
    close(cpuFD[1]);
}

double calculateCpuUsage(unsigned long prevCpuUsage[7], unsigned long currCpuUsage[7]) {
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
    
