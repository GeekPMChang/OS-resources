#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <fcntl.h>
pid_t parentpid, childpid;
typedef void (*sighandler_t) (int);

//信号处理功能：如果是父进程收到了一个SIGINT，就直接发送一个SIGINT给子进程，从而关闭掉这个执行命令的进程
void sigcat(){
    if(getpid() == parentpid)
        kill(childpid, SIGINT);    
}

//获取输入的信息放入到str字符串中
void getstr(char *str){
    char ch;
    int i = 0;
    while(scanf("%c", &ch) != EOF){
        if(ch != '\0' && ch != '\n')
            str[i++] = ch;
        else
            break;
    }
}

//解析命令
int parse(char *cmd, char **args){
    //分割字符串的初始化
    memset(args, 0, sizeof(char*)*64);//清除掉命令行的数据
    int cnt = 0;//记录切割后的参数数目个数
    char *str = cmd;//保存输入的命令
    char *tmp_str = NULL;//保存每次分割后的剩余字符串
    
    //对输入命令字符串按照中间的空格进行切割，切割结果放在args数组里
    while((args[cnt] = strtok_r(str, " ", &tmp_str)) != NULL){
        cnt++;
        str = NULL;
    }

    //返回一个切割的参数数量
    return cnt;
}

//执行无管道的指令
void exec_cmd(char *cmd){
    
    //进程使用
    size_t pid;
    int status;

    char *in = NULL;
    char *out = NULL;
    //在重定向系统中输入和输出文件的名称
    int in_num = 0;
    int out_num = 0;
    //重定向输入和输出的个数，方便判断使用者的指令是否存在错误
    
    char temp[256];
    strcpy(temp, cmd);
	char *args[256];
    //存储命令行的参数
    int argc = parse(temp, args);
    //argc分割后得到的字符串的数目

    int eidx = argc;//对于重定向操作的终止下标
    
    for(int i = 0; i < argc; i++){
        //确定重定向符“>”和“<”的数目
        if(!strcmp(args[i], "<")){
            if(i + 1 >= argc){
                printf("File name dose not exist.\n");
                exit(1);
            }
            else
                in = args[++i];//存储重定向输入文件名
            
            in_num++;
            if(eidx == argc) eidx = i - 1;
        }

        if(!strcmp(args[i], ">")){
            if(i + 1 >= argc){
                printf("File name dose not exist.\n");
                exit(1);
            }
            else
                out = args[++i];//存储重定向输出文件名
            
            out_num++;

            //对于重定向操作,重定向符所在的位置在文件名前一个字符上，最后一个字符是文件名
            if(eidx == argc) eidx = i - 1;
        }
    }
    
    //重定向输入输出文件数量过多，提示
    if(in_num > 1){
        printf("Too many redirection input files\n");
    }
    else if(out_num > 1){
        printf("Too many redirection output files.\n");
    }
    
    //如果是输入操作，需要考虑文件不存在的情况
    else if(in_num == 1){
        FILE *fp = fopen(in, "r");
        if(fp == NULL)
            printf("The input file dose not exist.\n");
        fclose(fp);
    }
    
    //处理指令
    childpid = fork();
    if(childpid < 0){
        printf("Create Process fail!\n");
        exit(1);
    }
    //子进程
    else if(childpid == 0){
        //有重定向输入
        if(in_num == 1){
            close(0);//如果有重定向输入，需要关掉0，open打开的新文件描述符会负责输入
            if(open(in, O_RDONLY) != 0){
                fprintf(stderr, "testsh: open != 0\n");
                exit(-1);
		    }

        }
        
        //有重定向输出
        if(out_num == 1){
            close(1);//如果有重定向输出，需要关掉1，open打开的新文件描述符会负责输出
            if(open(out, O_CREAT|O_WRONLY|O_TRUNC, 0644) != 1){
                fprintf(stderr, "testsh: open != 1\n");
                exit(-1);
		    }
        }
        
        char *tmp[256];
        for(int i = 0; i < eidx; i++)
            tmp[i] = args[i];
        tmp[eidx] = NULL;
        //如果是有重定向的这里就是将重定向符置为NULL
        //否则就是把argc位置置为NULL
        
        if(execvp(tmp[0], tmp) < 0)//执行操作
            printf("ERROR: invalid command.\n");
        exit(1);
    }
    //父进程
    else{
        waitpid(childpid, &status, 0);
    }
}

//处理管道指令
void pipe_cmd(char *cmd){

    size_t pid;

    char *tmp_str;
    //存放多级管道指令中当前需要执行的指令的字符串
    char *next = NULL;
    //存储分割后的剩余指令字符串
    char *args[256];
    
    //无管道指令，直接调用exec执行
    if(strstr(cmd, "|") == NULL){
        exec_cmd(cmd);
        return;
    }
    
    //有管道指令,对指令进行一次切割
    tmp_str = strtok_r(cmd, "|", &next);
    if(*next == 0){
        printf("There are no follow-up commands after |.\n");
        exit(1);
    }
    
    int pipe1[2];
    if(pipe(pipe1) < 0){
        printf("create pipe failed.\n");
    }
    //利用进程的并发和管道来实现管道的数据传输
    childpid = fork();
    if(childpid < 0){
        printf("create fork failed.\n");
        exit(1);
    }
    //子进程实现左边部分，并且将结果输出到管道的一端
    else if(childpid == 0){
        //关掉管道的读
        close(pipe1[0]);
        //将标准输出重定向到管道的输出
        dup2(pipe1[1], 1);
        //执行前半部分指令
        exec_cmd(tmp_str);
    }
    //父进程实现右边部分，将子进程左边传入的结果读取并作为右边指令的输入
    else{
        int status;
        waitpid(childpid, &status, 0);//父进程需要等子进程执行完成
        //关掉管道的写
        close(pipe1[1]);
        //将标准输入重定向到管道的输入
        dup2(pipe1[0], 0);
        
        pipe_cmd(next);//处理后半部分指令指令
        
        close(pipe1[0]);
    }
}

int main(){
    parentpid = getpid();
    signal(SIGINT, (sighandler_t)sigcat);
    char cmd[256];
    char *argclist[256];
    
    while(1){
        memset(cmd, 0, sizeof(cmd));
        //获取输入的命令并且刷洗缓冲区
        getstr(cmd);
        fflush(stdin);

        if(*cmd == '\n') continue;
        if(*cmd == '\0') exit(1);

        char tep_cmd[256];
        strcpy(tep_cmd, cmd);
        parse(tep_cmd, argclist);//对输入的指令进行解析
        
        //备份原标准输入输出标识符
        int stdin_FLAG = dup(0);
        int stdout_FLAG = dup(1);
        
        pipe_cmd(cmd);
        
        dup2(stdin_FLAG, 0);  //恢复标准输入输出标识符
        dup2(stdout_FLAG, 1);
    }
    return 0;
}

