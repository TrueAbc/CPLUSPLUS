//
// Created by TrueAbc on 2021/9/1.
//

#include "event_loop.h"
#include "string.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/fcntl.h>


void callback(event_loop *loop, int fd, void *args){
    printf("test");

    int buf;
    void *item = shmat(fd, 0, 0);
    memcpy(&buf, item, sizeof (int));
    printf("read shared mem %d\n", buf);
}

int test_shared_mem(){

    void *shmadd = nullptr;

    int data = 1024;

    int shmid;

    shmid = shmget((key_t)0, sizeof (int), 666 | IPC_CREAT | IPC_NOWAIT);

    shmadd= shmat(shmid, 0, 0); // 挂载到进程

    event_loop loop;
    loop.add_io_event(shmid, callback, kReadEvent, nullptr);

    // 写内存
    memcpy(shmadd, &data, sizeof (int));
}

int testMSG();
int testPipe();

int main(){
//    testMSG();
    testPipe();
}

void readback(event_loop* loop, int fd, void *args){
    int buf;
    write(fd,&buf, sizeof (buf));
    printf("read data:%d\n", buf);
}

void writeback(event_loop* loop, int fd, void *args){
    printf("write back\n");
    int data = 1;
    write(fd,& data, sizeof (data));
    // 写事件需要被删除

}

int testPipe(){
    int _pipe[2] = {0, 0};
    int ret = pipe(_pipe);
    if(ret == -1){
        exit(1);
    }

    printf("pipe fd:%d  %d\n", _pipe[0], _pipe[1]);
    event_loop loop;
//    loop.add_io_event(_pipe[0], readback, kReadEvent, nullptr);
//    loop.add_io_event(_pipe[1], writeback, kWriteEvent, nullptr);
//
//
//    loop.event_process();
    int flags = fcntl(_pipe[0], F_GETFL);
    fcntl(_pipe[0], F_SETFL, flags | O_NONBLOCK);
    int data;
    int re = write(_pipe[1], &data, sizeof (data));
    printf("%d \n", re);

}

void MSGCallback(event_loop *loop, int fd, void *args){
    printf("test");

    int buf;
    if(msgrcv(fd, &buf, sizeof (int ), 0, IPC_NOWAIT) != -1){

    }
    printf("read data %d\n", buf);
}


int testMSG(){
    int id = msgget(0, IPC_CREAT | IPC_NOWAIT | 0666);
    event_loop loop;

    loop.add_io_event(id, nullptr, kReadEvent, nullptr);
    int data=1;
    msgsnd(id, &data, sizeof (data), 0);
}