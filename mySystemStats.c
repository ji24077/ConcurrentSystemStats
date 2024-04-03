#define _POSIX_C_SOURCE 200809L
#include "stats_functions.h"
#include <signal.h>

void signal_handler(int sig) {
    char ans;

    // If Ctrl-Z detected it will proceed as normal
    if (sig == SIGTSTP) { return; }

    // If Ctrl-C detected it will ask user if they want to proceed
    if (sig == SIGINT) {
        printf("\nCtrl-C detected: ");
        printf("Do you want to quit? (press 'y' if yes) ");

        // Wait for user input
        int ret = scanf(" %c", &ans);
        if (ret == EOF) {
            if (errno == EINTR) {
                printf("\nSignal detected during scanf, resuming...\n");
                return;
            } 
            else {
                perror("scanf error");
                exit(EXIT_FAILURE);
            }
        }

        if (ans == 'y' || ans == 'Y') {
            exit(EXIT_SUCCESS);
        } else {
            printf("Resuming...\n");
        }
    }
}


int main(int argc,char *argv[]){
    //  struct utmp utmp_user=malloc(sizeof(struct utmp));
    //  free(utmp_user);
    int samples = 10,tdelay = 1;
    int user=0,system=0,sequential=0,graphics=0;
    struct option long_options[] = {
        {"system", no_argument, 0, 's'}, 
        {"user", no_argument, 0, 'u'}, 
        {"graphics", no_argument, 0, 'g'},        
        {"sequential", no_argument, 0, 'a'}, 
        {"samples", optional_argument, 0, 'b'}, // both samples and tdelay may change after. so we use optinal_argument. 
        {"tdelay", optional_argument, 0, 'c'},
        {0,0,0,0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "sugab::c::", long_options, NULL)) != -1){
        //so samples or -a maybe has arg or not so we use :: to tell.
        switch (opt) {
            case 's':
                system =1;
                break;
            case 'u':
                user = 1;
                break;
            case 'g':
                graphics =1;
                break;
            case 'a':
                sequential = 1;
                break;
            case 'b':
                if(optarg)samples = atoi(optarg); //so this line will store /update the value of samples.
            case 'c':
                if(optarg)tdelay = atoi(optarg);
        }
    }

    // if (argc > 1) {
    //     samples = atoi(argv[1]);
    // }

    // if (argc > 2) {
    //     tdelay = atoi(argv[2]);
    // }
    for(int ind = optind, i=0; ind < argc; ind++, i++) { // optind is global var.현재 처리 중인 인자의 위치(인덱스)현재 처리 중인 인자의 위치(인덱스)
        switch(i){
            case 0: 
                samples = atoi(argv[ind]); 
                break;
            case 1:
                tdelay = atoi(argv[ind]); 
                break;
        }
    }
    //initialize signal , may modulize after.
    struct sigaction act;
    act.sa_handler = signal_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction error for SIGINT");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTSTP, &act, NULL) == -1) {
        perror("sigaction error for SIGTSTP");
        exit(EXIT_FAILURE);
    }
    unsigned long prevCpuUsage[7];
    unsigned long currCpuUsage[7];
    double virtual_used_gb=0.0;
    double  prev_used_gb=0.0;
    float cur_cpuUsage=0.00,prevCpuUsageUsage = 0.00;
    char memArr[samples][1024];
    char cpuArr[samples][200];
    
    int i =0;
    int default_num = 3;
    int CPU_GRAPH_START_LINE;
    int MEM_GRAPH_START_LINE;
    int userLine_count;
    // // char cpu_arr[samples][200];
    // if(user && !system &&!sequential)
    // {
    //     printTopInfo(samples,tdelay,sequential,i);
    //     printf("-------------------------\n");
    //     printUserInfoThird();
    //     printf("-------------------------\n");
    //     printSystemInfoLast();
    //     printf("-------------------------\n");
    //     exit(1);   

    // }    
    
    int memFD[2],userFD[2],cpuFD[2];
    if(pipe(memFD)==-1 || pipe(userFD)==-1 || pipe(cpuFD)==-1){ //create/call pipe
        fprintf(stderr,"pipe failed",stderr(errno));
    }
        
    if(sequential){   //--seuential and/or --user --system etc.. 
        for (i = 0; i < samples; i++) { 
            storeCpuArr(prevCpuUsage); 
            sleep(tdelay); 
            int memPID = fork();
            if(memPID ==-1){
                fprintf(stderr,"fork error");
            }
            else if(memPID==0){ 
                close(memFD[0]);//close for reading.
                close(userFD[0]);
                close(cpuFD[0]);
                close(cpuFD[1]);
                close(userFD[1]);
                dup2(memFD[1],STDERR_FILENO); //redirect writing to print.
            }
            printTopInfo(sequential, samples, tdelay,i); 
            if(!user || (user && system)){ 
                printf("---------------------------------------\n");
                storeMemArr(memArr,i,&memFD); 
                virtual_used_gb = calculateVirtUsed();
                if(graphics) 
                    memoryGraphics(virtual_used_gb,&prev_used_gb,memArr,i); 
                
                fcnForPrintMemoryArr(sequential,samples,memArr,i); 
                if((user && system)||!system){ 
                    
                    printf("---------------------------------------\n");
                    printUserInfoThird(&userFD); 
                    printf("---------------------------------------\n");
                }

                printCores();
                
                storeCpuArr(currCpuUsage); 

                cur_cpuUsage = calculateCpuUsage(prevCpuUsage, currCpuUsage); 
                printf(" total cpu use: %.2f%%\n", cur_cpuUsage); 

                if(graphics)
                    setCpuGraphics(sequential,cpuArr,&default_num,cur_cpuUsage,&prevCpuUsageUsage,i); 
            }else{ 
                printf("---------------------------------------\n");
                printUserInfoThird(&cpuFD);
                printf("---------------------------------------\n");
            }
            }
        }
        else{
            for( i = 0; i < samples; i++) {
                storeCpuArr(prevCpuUsage);
                sleep(tdelay);
                printTopInfo(samples, tdelay,sequential,i);
                if((system &&  !user) &&!(sequential &&system)){ // no user in arg then we have to care about space of printing. 
                //--system. and/or --graphics, but no user. yet and/or --sequential.
                    storeMemArr(memArr,i);
                    //printf("no stop");
                    //printf("should not running here");
                    virtual_used_gb = calculateVirtUsed();
                    if(graphics){
                        memoryGraphics(virtual_used_gb,&prev_used_gb,memArr,i);
                    }
                    //fcnForPrintMemoryArr(sequential,samples,memArr,i);
                    printf("------------------------------------------------\n");  
                    reserve_space(samples);
                    printf("----------------------------------------------------\n"); 
                    printCores();
                    storeCpuArr(currCpuUsage);
                    cur_cpuUsage = calculateCpuUsage(prevCpuUsage,currCpuUsage);
                    printf("total cpu use:%.2f%%\n",cur_cpuUsage );
                    

                    for (int j=0;j<samples+4;j++){
                        printf("\033[1A"); //move cursor up one line.
                    } 
                    fcnForPrintMemoryArr(sequential,samples,memArr,i);
                    
                    if(graphics && system){

                        printf("\033[%d;1H", CPU_GRAPH_START_LINE=18);
                        setCpuGraphics(sequential,cpuArr,&default_num,cur_cpuUsage,&prevCpuUsageUsage,i); //if graphics option is given, display cpu graphics
                        
                    
                    }else{
                        printf("\033[3B"); //move cursor three line down.

                    }
                    

                    }
                    else if((user &&system)||!(user && system)&& !sequential){ //display w --user --system and/or --graphics, and/or sequential.
                        storeMemArr(memArr,i, );
                        //printf("runnign here\n");
                        virtual_used_gb = calculateVirtUsed();
                        if(graphics){
                            memoryGraphics(virtual_used_gb,&prev_used_gb,memArr,i);
                        }
                        //fcnForPrintMemoryArr(sequential,samples,memArr,i);
                        printf("------------------------------------------------\n");  
                        reserve_space(samples);
                        printf("----------------------------------------------------\n"); 
                        //printf("running here");
                        userLine_count= printUserInfoThird();
                        printf("---------------------------------------\n");
                        printCores();
                        storeCpuArr(currCpuUsage);
                        cur_cpuUsage= calculateCpuUsage(prevCpuUsage,currCpuUsage);
                        printf("total cpu use:%.2f%%\n",cur_cpuUsage );

                        printf("\033[%d;1H", MEM_GRAPH_START_LINE=3); //,may chage after .
                        fcnForPrintMemoryArr(sequential,samples,memArr,i);
                        if(graphics){
                            CPU_GRAPH_START_LINE = userLine_count+20;
                            
                            //printf("userline is:%d\n",userLine_count);
                            printf("\033[%d;1H", CPU_GRAPH_START_LINE); //maybe
                            setCpuGraphics(sequential,cpuArr,&default_num,cur_cpuUsage,&prevCpuUsageUsage,i);
                        }else{
                            for(int j=0; j<userLine_count+6; j++){
                                printf("\033[1B");
                            }
                            
                        }

                    }
                }
    }

    printf("------------------------------------\n");
    printSystemInfoLast();
    printf("----------------------------------\n");
    return 0;
    

    
    
}

void reserve_space(int samples){
    
    for(int i=0;i<samples+1; i++){
        printf("\n");
    }
}
