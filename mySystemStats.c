#define _POSIX_C_SOURCE 200809L
#include "stats_functions.h"
#include <signal.h>



pid_t memPID, userPID, cpuPID; // 전역 변수로 선언

void handle_ctrlZ() {
 struct sigaction action;
 memset(&action, 0, sizeof(action));
 action.sa_handler = SIG_IGN; // Ignore SIGTSTP (Ctrl-Z)

 // Register signal handler for SIGTSTP (Ctrl-Z)
 if (sigaction(SIGTSTP, &action, NULL) == -1) {
 perror("Unable to set up signal handler for SIGTSTP");
 exit(1);
 }}
void childProcessFunction() {
    // SIGINT 신호 무시
    signal(SIGINT, SIG_IGN);

    // 자식 프로세스의 나머지 작업 수행
}
void signal_handler(int signal) {
    char userInput[10];

    if (signal == SIGINT) {
        printf("\nCtrl-C detected: terminate? (press 'y' or 'yes', 'n' or 'no' to continue) ");

        scanf(" %9s", userInput); // 사용자 입력 받음

        // 프로그램 종료를 원할 경우
        if (strcmp(userInput, "y") == 0 || strcmp(userInput, "Y") == 0 || 
            strcmp(userInput, "yes") == 0 || strcmp(userInput, "YES") == 0) {
            
            kill(memPID, SIGTERM);
            kill(userPID, SIGTERM);
            kill(cpuPID, SIGTERM);

            waitpid(memPID, NULL, 0);
            waitpid(userPID, NULL, 0);
            waitpid(cpuPID, NULL, 0);

            exit(EXIT_SUCCESS); // 프로그램 종료
        }
        // 프로그램 계속 진행을 원할 경우
        // else if (strcmp(userInput, "n") == 0 || strcmp(userInput, "N") == 0 || 
        //          strcmp(userInput, "no") == 0 || strcmp(userInput, "NO") == 0) {
        //     //signal(SIGINT,signal_handler);
        //     return;
        // }
        else {
            // 'y', 'Y', 'yes', 'YES', 'n', 'N', 'no', 'NO' 외의 입력을 받았을 경우
            printf("continuing...\n");
            
        }
    }
}



int main(int argc,char *argv[]){
    //  struct utmp utmp_user=malloc(sizeof(struct utmp));
    //  free(utmp_user);
    // signal(SIGSTOP,signal_handler);
    // signal(SIGTSTP, signal_handler);
    handle_ctrlZ();
    //signal(SIGINT,signal_handler);
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
    //intialize signal
    
    unsigned long prevCpuUsage[7];
    unsigned long currCpuUsage[7];
    double virtual_used_gb=0.0;
    double  prev_used_gb=0.0;
    float cur_cpuUsage=0.00;
    //float prevCpuUsage = 0.00;
    char memArr[samples][1024];
    char cpuArr[samples][200];

    
    int i =0;
    // int default_num = 3;
    int CPU_GRAPH_START_LINE;
    // int MEM_GRAPH_START_LINE;
    int userLine_count;
    // // char cpu_arr[samples][200];
    // if(user && !system &&!sequential)
    // {
    //     GetInfoTop(samples,tdelay,sequential,i);
    //     printf("-------------------------\n");
    //     storeUserInfoThird();
    //     printf("-------------------------\n");
    //     printSystemInfoLast();
    //     printf("-------------------------\n");
    //     exit(1);   

    // }    
    
    int memFD[2],userFD[2],cpuPFD[2],cpuCFD[2],ucountFD[2];
    // for (i = 0; i < samples; i++){
        if(pipe(memFD)==-1 || pipe(userFD)==-1 || pipe(cpuPFD)==-1 || pipe(cpuCFD)==-1 || pipe(ucountFD)==-1){ //create/call pipe
            fprintf(stderr,"pipe failed");
        }
        // pid_t memPID,userPID,cpuPID; // for forking.
        memPID=fork();
        if(memPID==-1){
            fprintf(stderr,"forkMemPID failed");
            exit(EXIT_FAILURE);
        }
        else if(memPID==0){// create child process to write the stat by using fcn from stat_function.c
                //dup2(memFD[1],STDOUT_FILENO);
                childProcessFunction();
                 close(cpuPFD[0]);
            close(cpuCFD[0]);
            close(userFD[0]);
            close(ucountFD[0]);
            close(memFD[0]);
                //dup2(memFD[1],STDOUT_FILENO);
                 close(cpuPFD[1]);
            close(cpuCFD[1]);
            close(userFD[1]);
            close(ucountFD[1]);

                //sleep(tdelay);
             storeMemArr(samples,memFD,tdelay);
             
                     close(memFD[1]);
            exit(0);
        }
        //create child process for user info write.
        userPID = fork();
        if(userPID == -1){
            fprintf(stderr,"fork userPID failed");
            exit(EXIT_FAILURE);
        }
        else if(userPID ==0){
                            childProcessFunction();

            //create child process to write user stat.
             close(cpuCFD[0]); //generate other close fds.
                //dup2(memFD[1],STDOUT_FILENO);
                 close(cpuPFD[0]);
            close(userFD[0]);
            close(ucountFD[0]);
            close(memFD[0]);
            close(cpuCFD[1]); //generate other close fds.
                //dup2(memFD[1],STDOUT_FILENO);
                 close(cpuPFD[1]);
            close(memFD[1]);
            storeUserInfoThird(userFD,ucountFD);
             close(userFD[1]);
             close(ucountFD[1]);
             

            exit(0); //very important

        }
    
        cpuPID = fork();
        if(cpuPID ==-1){
            fprintf(stderr,"fork for cpu failed");
            exit(EXIT_FAILURE);
        }
        else if(cpuPID == 0){
                            childProcessFunction();

            close(userFD[0]); //close others too.
            close(cpuCFD[0]); //generate other close fds.
                //dup2(memFD[1],STDOUT_FILENO);
                 close(cpuPFD[0]);
            close(ucountFD[0]);
            close(memFD[0]);
            
            close(ucountFD[1]);
            close(userFD[1]);
            //dup2(cpuFD[1],STDOUT_FILENO);s
            for(int i=0; i<samples; i++){
                storeCpuArr(cpuPFD);
                sleep(tdelay);
                storeCpuArr(cpuCFD);
            }
            close(cpuCFD[1]); //generate other close fds.
                //dup2(memFD[1],STDOUT_FILENO);
                 close(cpuPFD[1]);
                 
            exit(0);
            // calculateCpuUsage(prevCpuUsage, currCpuUsage); 
            // printf(" total cpu use: %.2f%%\n", cur_cpuUsage); //this part will be on parent process. on A3. read from child.
        //  setCpuGraphics(sequential,cpuArr,&d_num,cur_cpuUsage,&prevCpuUsageUsage,i); //if graphics option is given, display cpu graphics

        }
        else
        {
               // setup_signal_handling();

            //parent process
            
            close(memFD[1]);
            close(userFD[1]);
            close(cpuPFD[1]);
            close(cpuCFD[1]);
            close(ucountFD[1]);
            

            struct sigaction act;
            
            act.sa_handler = signal_handler;
            act.sa_flags = 0;
            sigemptyset(&act.sa_mask);
            sigaction(SIGINT, &act, NULL);
            sigaction(SIGTSTP, &act, NULL);
            sigaction(SIGSTOP,&act,NULL);
            if (sigaction(SIGINT, &act, NULL) == -1) {
                perror("sigaction error for SIGINT");
                exit(EXIT_FAILURE);
            }

            if (sigaction(SIGTSTP, &act, NULL) == -1) {
                perror("sigaction error for SIGTSTP");
                exit(EXIT_FAILURE);
            }
            //signal(SIGINT,signal_handler);
            
            read(ucountFD[0],&userLine_count,sizeof(userLine_count));
            // waitpid(memPID, NULL, 0);
            // waitpid(userPID, NULL, 0);
            // waitpid(cpuPID, NULL, 0);

            if(sequential){
                for (i = 0; i < samples; i++) { 
                    
                        //storeCpuArr(prevCpuUsage); 
                        cpuArr[i][0] = '\0';
                        sleep(tdelay); 
                        GetInfoTop(samples,tdelay,sequential,i);
                        if(!user || (user && system)){ 
                            printf("---------------------------------------\n");
                            //storeMemArr(memArr,i); 
                            if(read(memFD[0], memArr[i], sizeof(memArr[i]))>0){
                                if(graphics){
                                    virtual_used_gb = calculateVirtUsed();

                                    memoryGraphics(virtual_used_gb, &prev_used_gb, memArr, i);
                                }
                                fcnForPrintMemoryArr(sequential, samples, memArr, i, memFD);
                            }
                            if((user && system)||!system){ 
                                printf("---------------------------------------\n");
                                    printUserInfoThird(userFD);
                                printf("---------------------------------------\n");
                            }

                            printCores();
                            
                            //storeCpuArr(currCpuUsage); 
                            read(cpuPFD[0],&prevCpuUsage,sizeof(prevCpuUsage));
                            read(cpuCFD[0],&currCpuUsage,sizeof(currCpuUsage));
                            cur_cpuUsage= calculateCpuUsage(prevCpuUsage,currCpuUsage);
                            printf("total cpu use:%.2f%%\n",cur_cpuUsage );
                            if(graphics){
                                
                                setCpuGraphics(sequential,cpuArr,cur_cpuUsage,&prevCpuUsage,i); 
                            }

                        }
                        else{ 
                            printf("---------------------------------------\n");
                            printUserInfoThird(userFD);
                            printf("---------------------------------------\n");
                        }
                }
            }
            else
            {
                int systemStartGraphics=0;
                int memStartCursor=0;
                for (i = 0; i < samples; i++) { 
                    
                    sleep(tdelay);
                    //read(ucountFD[0],&userLine_count,sizeof(userLine_count));
                    GetInfoTop(samples,tdelay,sequential,i);
                                     //   printf("ucount:%d\n",userLine_count);

                    if(!user || (user && system)){ //base case
                        //printf("this is non seq\n");
                        printf("------------------------------------------------\n");  
                        reserve_space(samples);
                        if((user && system)||!system){ 
                                //printf("not passing?");
                                printf("---------------------------------------\n");
                                   printUserInfoThird(userFD);
                                printf("---------------------------------------\n");
                        }
                        printCores();
                        read(cpuPFD[0],&prevCpuUsage,sizeof(prevCpuUsage));
                        read(cpuCFD[0],&currCpuUsage,sizeof(currCpuUsage));
                        cur_cpuUsage = calculateCpuUsage(prevCpuUsage,currCpuUsage);
                        printf("total cpu use:%.2f%%\n",cur_cpuUsage );
                        if(read(memFD[0], memArr[i], sizeof(memArr[i]))>0){
                            if(graphics){
                                virtual_used_gb = calculateVirtUsed();

                                memoryGraphics(virtual_used_gb, &prev_used_gb, memArr, i);
                            }
                            
                            if(system && !user){ // for system
                                memStartCursor = samples+3;
                                printf("\033[%dA",memStartCursor); //cursor up 
                            }
                            else if((system && user)){
                                    memStartCursor = samples + userLine_count +4;
                                printf("\033[%dA",memStartCursor); //cursor up 
                            }
                            else if(user && !(system && user) ){
                                memStartCursor = samples + userLine_count +6;
                                printf("\033[%dA",memStartCursor); //cursor up 
                            }
                            else{
                                //default
                               // printf("default pass here");
                                memStartCursor = samples + userLine_count +4;
                                printf("\033[%dA",memStartCursor); //cursor up 

                            }
                            fcnForPrintMemoryArr(sequential, samples, memArr, i, memFD);
                            if(graphics)
                            {
                                systemStartGraphics =userLine_count;
                                CPU_GRAPH_START_LINE= 4+userLine_count+samples-i;
                                printf("\033[%d;1B", CPU_GRAPH_START_LINE); //cusor down
                                setCpuGraphics(sequential,cpuArr,cur_cpuUsage,&prevCpuUsage,i); //if graphics option is given, display cpu graphics
                            }
                        }
                        int systemStart = userLine_count+6;
                        if(graphics){
                            systemStart = systemStartGraphics;
                        }else if(system &&!user){
                                // printf("usercount:%d",userLine_count);
                            systemStart = userLine_count +3;
                            printf("\033[%dB",systemStart); //move cursor three line down
                        }
                        else{
                            systemStart = userLine_count +6;
                            printf("\033[%dB",systemStart); //move cursor three line down
                        }
                           

                    }
                    else{
                            //printf("this is user flag\n");
                            printf("---------------------------------------\n");
                            printUserInfoThird(userFD);
                            printf("---------------------------------------\n");
                            int systemStart = userLine_count;
                           // printf("usercount:%d",userLine_count);
                            printf("\033[%dB",systemStart); //move cursor three line down.


                    }       
                }     
            }
            close(cpuPFD[0]);
            close(cpuCFD[0]);
            close(userFD[0]);
            close(memFD[0]);
            close(ucountFD[0]);
            close(userFD[0]);
            printf("------------------------------------\n");
            printSystemInfoLast();
            printf("----------------------------------\n");
        }
        return 0;
}