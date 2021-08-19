//
// Created by TrueAbc on 2021/8/9.
//

#include <cstring>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include "tcp_server.h"


struct message{
    char data[m4K];
    char len;
};

struct message msg;

void server_rd_callback(event_loop* loop, int fd, void *args);
void server_wt_callback(event_loop* loop, int fd, void *args);

void accept_callback(event_loop *loop, int fd, void *args){
    tcp_server *server = (tcp_server*)args;
    server->do_accept();
}

// 构造函数
tcp_server::tcp_server(event_loop *loop ,const char *ip, uint16_t port) {
    bzero(&_connaddr, sizeof (_connaddr));

    // 忽略一些信号
    if(signal(SIGHUP, SIG_IGN) == SIG_ERR){
        fprintf(stderr, "signal ignore SIGUP\n");
    }
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        fprintf(stderr, "signal ignore SIGPIPE\n");
    }

    // 1. 创建socket
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(_sockfd == -1){
        fprintf(stderr, "tcp_server::socket()\n");
        exit(-1);
    }

    // 初始化地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
            //inet_addr(INADDR_ANY);

    // 2-1 可以多次监听, 设置REUSE属性
    int op = 1;
    if(setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof (op)) < 0){
        fprintf(stderr, "setsocket opt SO_REUSERADDR\n");
    }

    if(bind(_sockfd, (const struct sockaddr*)&server_addr, sizeof (server_addr)) < 0){
        fprintf(stderr, "bind error\n");
        if (errno == EADDRINUSE){
            fprintf(stderr, "addr in use\n");
        } else if(errno == EINVAL){
            fprintf(stderr, "port in use\n");
        } else{
            fprintf(stderr, "nothing in use\n");
        }
        exit(1);
    }

    // 4. 开始监听ip端口
    if(listen(_sockfd, 500) == -1 ){
        fprintf(stderr, "listen error\n");
        exit(1);
    }

    _loop = loop;
    _loop->add_io_event(_sockfd, accept_callback, kReadEvent, this);
}

// 开始提供创建链接的服务
void tcp_server::do_accept() {
    int connfd;
    while (true){
        printf("begin accept\n");
        connfd = accept(_sockfd, (struct sockaddr*)&_connaddr, &_addrlen);
        if(connfd == -1){
            if(errno == EINTR){
                fprintf(stderr, "accept errno=EINTR\n");
                continue;
            } else if(errno == EMFILE){
                fprintf(stderr, "accept errno=EMFILE\n");
            } else if(errno == EAGAIN){
                fprintf(stderr, "accept errno = EAGAIN\n");
                break;
            } else{
                fprintf(stderr, "accept error");
                exit(1);
            }
        } else{
            // TODO 添加心跳机制
            this->_loop->add_io_event(connfd, server_rd_callback, kReadEvent, &msg);
            break;
        }
    }
}

tcp_server::~tcp_server() {
    close(_sockfd);
}


void server_rd_callback(event_loop* loop, int fd, void *args){
    int ret = 0;
    struct message *msg = (struct message*)args;
    input_buf ibuf;
    ret = ibuf.read_data(fd);
    if(ret == -1){
        fprintf(stderr, "ibuf read data error\n");
        loop->del_io_event(fd, kReadEvent);
        close(fd);
        return;
    }

    if(ret == 0){
        loop->del_io_event(fd, kReadEvent);
        close(fd);
        return;
    }

    printf("ibuf.length()=%d\n", ibuf.length());

    msg->len = ibuf.length();
    bzero(msg->data, msg->len);
    memcpy(msg->data, ibuf.data(), msg->len);

    ibuf.pop(msg->len);
    ibuf.adjust();

    printf("recv data = %s\n", msg->data);

    // 删除读取事件, 添加写事件, 针对kqueue没有必要
    loop->del_io_event(fd, kReadEvent);
    loop->add_io_event(fd, server_wt_callback, kWriteEvent, msg);
}

void server_wt_callback(event_loop *loop, int fd, void *args){
    struct message *msg = (struct  message *)args;
    out_buf obuf;

    obuf.send_data(msg->data, msg->len);
    // 回显数据
    while(obuf.length()){
        int write_ret = obuf.write2fd(fd);
        if(write_ret == -1){
            fprintf(stderr, "write connfd error\n");
            return;
        } else if(write_ret == 0){
            break; // 不是错误, 表示此时不可写
        }
    }
    loop->del_io_event(fd, kWriteEvent);
    loop->add_io_event(fd, server_rd_callback, kReadEvent, msg);
}