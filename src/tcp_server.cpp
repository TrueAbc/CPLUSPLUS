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
#include "tcp_conn.h"

// ===== 链接资源管理 =====
tcp_conn **tcp_server::conns = nullptr;

int tcp_server::_max_conns = 0;
int tcp_server::_curr_conns = 0;
pthread_mutex_t tcp_server::_conns_mutex = PTHREAD_MUTEX_INITIALIZER;

msg_router tcp_server::router;

conn_callback tcp_server::conn_start_cb = nullptr;
conn_callback tcp_server::conn_close_cb = nullptr;

void* tcp_server::conn_start_cb_args = nullptr;
void * tcp_server::conn_close_cb_args = nullptr;

void tcp_server::increase_conn(int connfd, tcp_conn *conn) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = conn;
    _curr_conns++;
    pthread_mutex_unlock(&_conns_mutex);
}

void tcp_server::decrease_conn(int connfd) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = nullptr;
    _curr_conns--;
    pthread_mutex_unlock(&_conns_mutex);
}

void tcp_server::get_conn_num(int *curr_conn) {
    pthread_mutex_lock(&_conns_mutex);
    *curr_conn = _curr_conns;
    pthread_mutex_unlock(&_conns_mutex);
}

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

    // 5. 将sockfd加入到event_loop中
    _loop = loop;
    _loop->add_io_event(_sockfd, accept_callback, kReadEvent, this);

    // 6. 链接管理
    _max_conns = MAX_CONNS;
    conns = new tcp_conn*[_max_conns+3]; // stdin, stdout, stderr已经占了前三个位置
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
            int cur_conns;
            get_conn_num(&cur_conns);

            if(cur_conns >= _max_conns){
                fprintf(stderr, "too many connections, max=%d\n", _max_conns);
            } else{
                tcp_conn *conn = new tcp_conn(connfd, _loop);
                if(conn == nullptr){
                    fprintf(stderr, "new tcp_conn error\n");
                    exit(1);
                }
                printf("get new connection succ!\n");

            }
            break;
        }
    }
}

tcp_server::~tcp_server() {
    close(_sockfd);
}
