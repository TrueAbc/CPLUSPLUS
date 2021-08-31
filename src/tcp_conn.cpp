//
// Created by TrueAbc on 2021/8/19.
//

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "tcp_server.h"
#include "tcp_conn.h"
#include "message.h"

// 回显业务, 测试用
void callback_busi(const char*data, uint32_t len, int msgid, void *args, tcp_conn *conn){
    conn->send_message(data, len, msgid);
}


// 链接的读写事件回调
static void conn_rd_callback(event_loop* loop, int fd, void *args){
    tcp_conn* conn = (tcp_conn *)args;
    conn->do_read();
}

static void conn_wt_callback(event_loop* loop, int fd, void *args){
    tcp_conn* conn = (tcp_conn *)args;
    conn->do_write();
}

// 初始化tcp_conn
tcp_conn::tcp_conn(int connfd, event_loop *loop) {
    _connfd = connfd;
    _loop = loop;
    // 1. 将connfd设置为非阻塞状态
    int flag = fcntl(_connfd, F_GETFL, 0);
    fcntl(_connfd, F_SETFL, O_NONBLOCK | flag);

    // 2. 设置tcp_nodelay 禁止做读写缓存, 降低小包延迟
    int op;
    setsockopt(_connfd, F_SETFL, O_NONBLOCK | flag, &op, sizeof (op));

    if(tcp_server::conn_start_cb){
        tcp_server::conn_start_cb(this, tcp_server::conn_start_cb_args);
    }

    _loop->add_io_event(_connfd, conn_rd_callback, kReadEvent, this);
    tcp_server::increase_conn(_connfd, this);
}

void tcp_conn::do_read() {
    // 1. 从套接字读取数据
    int ret = ibuf.read_data(_connfd);
    if(ret == -1){
        fprintf(stderr, "read data from socket\n");
        this->clean_conn();
        return;
    } else if(ret == 0){
        printf("connection closed by peer\n");
        clean_conn();
        return;
    }

    // 解析msg_head
    msg_head head;
    while (ibuf.length() >= MESSAGE_HEAD_LEN){
        // 2.1 可以读取到一个消息头
        memcpy(&head, ibuf.data(), MESSAGE_HEAD_LEN);
        if (head.msglen > MESSAGE_LENGTH_LIMIT || head.msglen <0){
            fprintf(stderr, "data format error, need close, msglen=%d", head.msglen);
            clean_conn();
            break;
        }

        if(ibuf.length() < MESSAGE_HEAD_LEN + head.msglen){
            break;
        }

        // 头部已经处理完成
        ibuf.pop(MESSAGE_HEAD_LEN);
        printf("read data:%s\n", ibuf.data());

        tcp_server::router.call(head.msgid, head.msglen, ibuf.data(), this);
        ibuf.pop(head.msglen);
    }

    ibuf.adjust();
    return;
}


void tcp_conn::do_write() {
    // 触发event处理的事情
    while(obuf.length()){
        int ret = obuf.write2fd(_connfd);
        if(ret == -1){
            fprintf(stderr, "write2df error, close conn!\n");
            this->clean_conn();
            return;
        }
        if(ret == 0 ){
            break;
        }
    }
    if(obuf.length() == 0){
        // 数据写入完成, 写事件取消
        _loop->del_io_event(_connfd, kWriteEvent);
    }
    return;
}

int tcp_conn::send_message(const char *data, int msglen, int msgid) {
    printf("send message:%s:%d, msgid=%d\n", data, msglen, msgid);
    bool active_epollout = false;
    if(obuf.length() == 0){
        // 数据发送完成, 激活写事件
        active_epollout = true;
    }

    // 1. 消息头
    msg_head head;
    head.msglen = msglen;
    head.msgid = msgid;

    int ret = obuf.send_data((const char*)&head, MESSAGE_HEAD_LEN);
    if (ret != 0){
        fprintf(stderr, "send head error\n");
        return -1;
    }

    ret = obuf.send_data(data, msglen);
    if (ret != 0) {
        //如果写消息体失败，那就回滚将消息头的发送也取消
        obuf.pop(MESSAGE_HEAD_LEN);
        return -1;
    }

    if(active_epollout){
        // 激活epollout事件
        _loop->add_io_event(_connfd, conn_wt_callback, kWriteEvent, this);
    }
    return 0;
}

void tcp_conn::clean_conn() {

    if(tcp_server::conn_close_cb){
        tcp_server::conn_close_cb(this, tcp_server::conn_close_cb_args);
    }

    tcp_server::decrease_conn(_connfd);

    _loop->del_io_event(_connfd);

    ibuf.clear();
    obuf.clear();

    int fd = _connfd;
    _connfd = -1;
    close(fd);
}