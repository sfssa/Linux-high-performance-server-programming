# define TIMEOUT 5000

int timeout = TIMEOUT;
time_t start = time(NULL);
time_t end = time(NULL);

while(1)
{
    printf("the timeout is now %d mil_seconds\n", timeout);
    start = time(NULL);
    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, timeout);
    if(number == 0 && errno != EINTR)
    {
        printf("epoll failure\n");
        break;
    }
    if(number == 0)
    {
        // 返回0-超时时间到达，处理定时任务，重新设置定时时间
        timeout = TIMEOUT;
        continue;
    }
    // 返回值大于0，使用的时间是end - start
    end = time(NULL);
    timeout -= (end - start) * 1000;
    if(timeout <= 0) 
    {
        timeout = TIMEOUT;
    }
    // 处理非活动连接
}
