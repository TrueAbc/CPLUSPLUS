
#include "thread_pool.h"
#include "event_loop.h"
#include "tcp_conn.h"
#include <stdio.h>

/*
 * 一旦有task消息, 使用该业务处理消息服务
 *  只要有人调用task_queue::send()方法就会触发
 */
void deal_task_message(event_loop* loop, int fd, void *args){
   thread_queue<task_msg>* queue = (thread_queue<task_msg>*)args;

   // 取出所有的任务
   std::queue<task_msg> tasks;
   queue->recv(tasks);

   while(!tasks.empty()){
       task_msg task = tasks.front();

       tasks.pop();
       if(task.type == task_msg::NEW_CONN){
           // 新建链接的任务
           tcp_conn *conn = new tcp_conn(task.connfd, loop);
           if(conn == nullptr){
               fprintf(stderr, "in thread new tcp conn error\n");
               exit(1);
           }

           printf("[thread]: get new connection succ\n");
       } else if(task.type == task_msg::NEW_TASK){
           //todo 收发消息的任务
       } else{
           fprintf(stderr, "unknow task\n");
       }
   }

}

void* thread_main(void *args){
    thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;

    //每个线程对应一个event_loop监控客户端链接的读写事件
    event_loop* loop = new event_loop();
    if(loop == nullptr){
        fprintf(stderr, "new event _loop error\n");
        exit(1);
    }

    // 内部的事件只有读事件
    queue->set_loop(loop);
    queue->set_callback(deal_task_message, queue);

    loop->event_process();
    return nullptr;
}

thread_pool::thread_pool(int thread_cnt) {
    _index = 0;
    _queues = nullptr;
    _thread_cnt = thread_cnt;
    if(_thread_cnt <= 0){
        fprintf(stderr, "_thread_cnt <0\n");
        exit(1);
    }

    _queues = new thread_queue<task_msg>*[thread_cnt];
    _tids = new pthread_t [thread_cnt];

    for(int i=0;i<thread_cnt;i++){
        printf("creat thread %d\n", i);
        _queues[i] = new thread_queue<task_msg>();//给线程添加任务队列
        int ret = pthread_create(&_tids[i], nullptr, thread_main, _queues[i]);
        if(ret == -1){
            perror("thread pool, create thread error\n");
            exit(1);
        }

        pthread_detach(_tids[i]);
    }
}

thread_queue<task_msg> *thread_pool::get_thread() {
    if(_index == _thread_cnt){
        _index = 0;
    }
    return _queues[_index];
}