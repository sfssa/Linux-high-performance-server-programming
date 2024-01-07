#ifndef TIME_WHEEL_TIMER
#define TIME_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64

class tw_timer;

// 客户端信息
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer* timer;
};

class tw_timer
{
public:
    tw_timer(int rot, int ts) :next(NULL), prev(NULL), rotation(rot), time_slot(ts) {}
public:
    // 定时器在时间轮多少个滴答后触发
    int rotation;
    // 定时器属于时间轮上哪个槽
    int time_slot;
    // 定时器回调函数
    void (*cb_func)(client_data*);
    // 客户数据
    client_data* user_data;
    tw_timer* next;
    tw_timer* prev;
};

class time_wheel
{
public:
    time_wheel() 
        : cur_slot(0)
    {
        for(int i = 0; i < N; ++i)
        {
            // 初始化每个头结点
            slots[i] = NULL;
        }
    }

    ~time_wheel()
    {
        for(int i = 0; i < N; ++i)
        {
            tw_timer* tmp = slots[i];
            while(tmp)
            {
                // 析构整条链表的结点
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }

    // 根据定时值创建定时器，插入到指定的槽中
    tw_timer* add_timer(int timeout)
    {
        if(timeout < 0)
        {
            return NULL;
        }
        int ticks = 0;
        // 计算需要转多少个滴答后触发，小于SI则上调为1，否则调整为timeout/si
        if(timeout < SI)
        {
            ticks = 1;
        }
        else 
        {
            ticks = timeout / SI;
        }

        // 计算待插入的定时器在时间轮转动多少圈后触发
        int rotation = ticks / N;
        // 应该插入到哪个槽中
        int ts = (cur_slot + (ticks % N)) % N;
        // 创建新的定时器，时间轮转动rotation圈后触发，插入到第ts槽上
        tw_timer* timer = new tw_timer(rotation, ts);
        // 如果该槽没有头结点则设置为该槽的头结点
        if(!slots[ts])
        {
            printf("add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else 
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    // 删除目标定时器
    void del_timer(tw_timer* timer)
    {
        if(!timer)
        {
            return;
        }

        int ts = timer->time_slot;
        // 是槽的头结点，重置头结点
        if(timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if(slots[ts])
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if(timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }

    // SI时间到达后，调用该函数，时间轮向前滚动一个槽的间隔
    void tick()
    {
        tw_timer* tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while(tmp)
        {
            // 循环处理整条链表上的结点
            printf("tick the timer once\n");
            // 如果rotation大于0，在这一轮不会触发
            if(tmp->rotation > 0)
            {
                --tmp->rotation;
                tmp = tmp->next;
            }
            else
            {
                // 否则说明已经到期，执行定时任务
                tmp->cb_func(tmp->user_data);
                // 处理的是头结点
                if(tmp == slots[cur_slot])
                {
                    printf("delete header in cur_slot\n");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if(slots[cur_slot])
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if(tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        // 更新时间轮
        cur_slot = ++cur_slot % N;
    }
private:
    // 时间轮上槽的数量
    static const int N = 60;
    // 槽间隔，1秒钟
    static const int SI = 1;
    // 时间轮的槽起始结点
    tw_timer* slots[N];
    // 当前槽
    int cur_slot;
};
#endif
