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

// 构造函数
tcp_server::tcp_server(const char *ip, uint16_t port) {
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
    inet_aton(ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // 2-1 可以多次监听, 设置REUSE属性
    int op = 1;
    if(setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof (op)) < 0){
        fprintf(stderr, "setsocket opt SO_REUSERADDR\n");
    }

    if(bind(_sockfd, (const struct sockaddr*)&server_addr, sizeof (server_addr))){
        fprintf(stderr, "bind error\n");
        exit(1);
    }

    // 4. 开始监听ip端口
    if(listen(_sockfd, 500) == -1 ){
        fprintf(stderr, "listen error\n");
        exit(1);
    }
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
            int writed;
            char *data = "hello Lars\n";
            do{
                writed = write(connfd, data, strlen(data) + 1);
            } while (writed == -1 && errno == EINTR);

            if(writed > 0) {
                printf("succeed write\n");
            }
            if (writed == -1 && errno == EAGAIN){
                writed = 0;// 非错误, 返回0
            }
        }
    }
}

tcp_server::~tcp_server() {
    close(_sockfd);
}
