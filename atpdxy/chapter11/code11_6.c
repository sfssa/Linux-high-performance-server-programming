#ifndef MIN_HEAP
#define MIN_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>

using std::exception;

#define BUFFER_SIZE 64

// 向前声明时间堆
class heap_timer;

// 存储客户端信息
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer* timer;
};

// 定时器封装
class heap_timer
{
public:
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }
public:
    // 超时时间
    time_t expire;
    // 定时器回调函数
    void (*cb_func)(client_data*);
    // 客户端信息
    client_data* user_data;
};

class time_heap
{
public:
    time_heap(int cap)
        : capacity(cap), cur_size(0)
    {
        array = new heap_timer* [capacity];
        if(!array)
        {
            throw std::exception();
        }
        for(int i = 0; i < capacity; ++i)
        {
            array[i] = NULL;
        }
    }

    // 用已有数组初始化堆
    time_heap(heap_timer** init_array, int size, int capacity)
        :cur_size(size), capacity(capacity)
    {
        if(capacity < size)
        {
            throw std::exception();
        }
        array = new heap_timer* [capacity];
        if(!array)
        {
            throw std::exception();
        }
        for(int i = 0; i < capacity; ++i)
        {
            array[i] = NULL;
        }
        if(size != 0)
        {
            for(int i = 0; i < size; ++i)
            {
                array[i] = init_array[i];
            }
            for(int i = (cur_size - 1) / 2; i >= 0; --i)
            {
                percolate_down(i);
            }
        }
    }
    
    ~time_heap()
    {
        for(int i = 0; i < cur_size; ++i)
        {
            delete array[i];
        }
        delete[] array;
    }

    // 添加定时器
    void add_timer(heap_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        // 堆的容量不足，扩大一倍
        if(cur_size >= capacity)
        {
            resize();
        }
        // 新插入一个元素，当前堆大小+1，hole是新建空穴的位置
        int hole = cur_size++;
        int parent = 0;
        for(; hole > 0; hole = parent)
        {
            parent = (hole - 1) / 2;
            if(array[parent]->expire <= timer->expire)
            {
                break;
            }
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }
    // 删除定时器
    void del_timer(heap_timer* timer)
    {
        if(!timer)
        {
            return;
        }

        // 仅将目标定时器的回调函数置空，也就是所谓的延迟销毁，节省真正删除该定时器造成的开销，但是容易使堆数组膨胀
        timer->cb_func = NULL;
    }

    // 获得堆顶的定时器
    heap_timer* top() const
    {
        if(empty())
        {
            return NULL;
        }
        return array[0];
    }

    // 删除堆顶的定时器
    void pop_timer()
    {
        if(empty())
        {
            return;
        }
        if(array[0])
        {
            delete array[0];
            // 将原来堆顶元素替换成最后一个元素
            array[0] = array[--cur_size];
            // 对新的堆顶，执行下滤操作
            percolate_down(0);
        }
    }

    // 心搏函数
    void tick()
    {
        heap_timer* tmp = array[0];
        time_t cur = time(NULL);
        while(!empty())
        {
            if(!tmp)
            {
                break;
            }
            // 堆顶没有到期
            if(tmp->expire > cur)
            {
                break;
            }
            // 执行堆顶定时器任务
            if(array[0]->cb_func)
            {
                array[0]->cb_func(array[0]->user_data);
            }
            // 将堆顶定时器删除
            pop_timer();
            tmp = array[0];
        }
    }

    // 判断堆数组是否为空
    bool empty() const { return cur_size == 0; }
private:
    // 最小堆的下滤工作
    void percolate_down(int hole)
    {
        heap_timer* temp = array[hole];
        int child = 0;
        for(; ((hole * 2 + 1) <= (cur_size - 1)); hole = child)
        {
            child = hole * 2 + 1;
            if((child < (cur_size - 1)) && (array[child + 1]->expire < array[child]->expire))
            {
                ++child;
            }
            if(array[child]->expire < temp->expire)
            {
                array[hole] = array[child];
            }
            else 
            {
                break;
            }
        }
        array[hole] = temp;
    }

    // 将堆数组容量阔1倍
    void resize()
    {
        heap_timer** temp = new heap_timer*[2 * capacity];
        for(int i = 0; i < 2 * capacity; ++i)
        {
            temp[i] = NULL;
        }
        if(!temp)
        {
            throw std::exception();
        }
        capacity = 2 * capacity;
        for(int i = 0; i < cur_size; ++i)
        {
            temp[i] = array[i];
        }
        delete[] array;
        array = temp;
    }
private:
    // 堆数组
    heap_timer** array;
    // 堆数组的最大容量
    int capacity;
    // 堆数组当前的元素个数
    int cur_size;
};
#endif
