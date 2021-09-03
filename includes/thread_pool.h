//
// Created by TrueAbc on 2021/9/2.
//

#ifndef LARS_THREAD_POOL_H
#define LARS_THREAD_POOL_H

#include "thread_queue.h"
#include "task_msg.h"
#include <pthread.h>

class thread_pool{
public:
    thread_pool(int thread_cnt);
    thread_queue<task_msg>* get_thread();
private:

    //  当前线程池中的全部消息任务队列的头指针
    thread_queue<task_msg> **_queues;

    int _thread_cnt;

    pthread_t  *_tids;// 线程编号

    int _index; // 当前选择的线程队列下标

};

#endif //LARS_THREAD_POOL_H
