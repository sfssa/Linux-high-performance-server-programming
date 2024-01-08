#include <sys/signal.h>
#include <event.h>

// 定时器只执行一次，而信号会执行多次
void signal_cb(int fd, short event, void* argc)
{
    // 创建事件的基础结构
    struct event_base* base = (event_base*)argc;
    // 延迟时间
    struct timeval delay = {2, 0};
    printf("Caught an interrupt signal; exiting cleanly in two seconds...\n");
    // 通知事件循环退出，告诉事件循环，在两秒后退出
    event_base_loopexit(base, &delay);
}

void timeout_cb(int fd, short event, void* argc)
{
    printf("timeout\n");
}

int main(int argc, char* argv[])
{
    // 初始化事件库，代表一个事件循环的实例
    struct event_base* base = event_init();

    // 创建信号事件
    struct event* signal_event = evsignal_new(base, SIGINT, signal_cb, base);
    event_add(signal_event, NULL);

    // 创建定时事件
    struct timeval tv = {1, 0};
    struct event* timeout_event = evtimer_new(base, timeout_cb, NULL);
    event_add(timeout_event, &tv);

    // 开始循环
    event_base_dispatch(base);
    // 释放资源
    event_free(timeout_event);
    event_free(signal_event);
    event_base_free(base);
    return 0;
}
