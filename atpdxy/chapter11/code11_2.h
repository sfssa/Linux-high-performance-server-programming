#ifndef LST_TIMER
#define LST_TINER

/*
 * Filename: code11_2.h
 * Author: atpdxy
 * Date: 2024-01-02
 * Description: Brief description of the header file.
 */

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
// 缓冲区大小
#define BUFFER_SIZE 64

// 向前声明
class util_timer;

// 客户端数据结构
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

// 定时器类
class util_timer
{
public:
    // 构造函数
    util_timer() :prev(nullptr),next(nullptr) {}
    // 定时时间，使用绝对时间
    time_t expire;
    // 回调函数
    void (*cb_func)(client_data*);
    // 客户数据
    client_data* user_data;
    // 指向前一个定时器
    util_timer* prev;
    // 指向后一个定时器
    util_timer* next;
};

// 定时器链表，升序双向链表，有头结点和尾结点
class sort_timer_lst
{
public:
    // 构造函数
    sort_timer_lst() :head(NULL), tail(NULL) {}

    // 析构函数
    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;;
            tmp = head;
        }
    }

    // 将目标定时器timer添加到列表中
    void add_timer(util_timer* timer)
    {
        // 如果定时器为空
        if(!timer)
        {
            return;
        }

        // 如果定时器链表为空
        if(!head)
        {
            head = tail = timer;
            return;
        }
        // 新插入的定时器定时时间最短，成为新的头结点
        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        // 根据时间插入到指定位置
        add_timer(timer, head);
    }

    // 当某个定时任务发生变化时，调整对应的定时器在链表中的位置，这个函数只考虑被调整定时器的超时时间演唱的情况
    void addjust_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        util_timer* tmp = timer->next;
        // 如果被调整目标超时时间在尾部或者小于其下一个定时器超时值，不用调整
        if(!tmp || timer->expire < tmp->expire)
        {
            return;
        }
        // 链表头结点，将头结点取出后重新插入到链表中
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        // 目标定时器不是头结点，将定时器取出，插入到原来的所在位置之后的部分链表中
        else 
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    // 将目标定时器timer从链表中删除
    void del_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }

        if(timer == head && timer == tail)
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        // 链表至少有两个定时器且目标定时器是链表头结点，将链表头结点重置为原头结点的下一个节点，删除目标定时器
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        // 链表至少有两个定时器且目标定时器是链表尾部，将链表尾节点重置为原尾节点的前一个节点，删除目标定时器
        if(timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        // 目标定时器位于链表中间，将前后链表串联起来，删除目标定时器
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    // SIGALRM信号每次触发的信号处理函数
    void tick()
    {
        if(!head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);
        // 从头结点开始处理每个定时器
        util_timer* tmp = head;
        while(tmp)
        {
            if(tmp->expire > cur)
            {
                // 链表升序，后面的都不用判断了
                break;
            }
            tmp->cb_func(tmp->user_data);
            // 执行完毕后删除并重置头结点
            head = tmp->next;
            if(head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
    }
private:
    // 私有添加定时器函数：将timer插入到list_head后面的部分
    void add_timer(util_timer* timer, util_timer* list_head)
    {
        util_timer* prev =list_head;
        util_timer* tmp = prev->next;
        while(tmp)
        {
            if(tmp->expire < prev->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        // 作为尾节点
        if(!tmp)
        {
            prev->next = timer;
            timer->next = nullptr;
            timer->prev = prev;
            tail = timer;
        }
    }
private:
    // 头结点
    util_timer* head;
    // 尾节点
    util_timer* tail;
};

#endif
