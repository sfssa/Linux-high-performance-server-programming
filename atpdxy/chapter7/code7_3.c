#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
// 让一个进程以守护进程运行

bool daemonize(){
    pid_t pid = fork();
    if(pid == -1){
        return false;
    }else if(pid > 0){
        exit(0);
    }else{
        umask(0);

        // 创建新会话，当前进程为会话的首领进程
        pid_t sid = setsid();
        if(sid < 0){
            return false;
        }
        
        // 切换工作目录
        if((chdir("/")) < 0){
            return false;
        }

        // 关闭标准输入、标准输出、标准错误输出
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // 关闭其他打开的文件描述符
        // ...
        
        // 对标准输入、标准输出、标准错误输出重定向
        open("/dev/null", O_RDONLY);
        open("/dev/null", O_RDWR);
        open("/dev/null", O_RDWR);
        return true;
    }
}
