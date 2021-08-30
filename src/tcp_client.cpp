//
// Created by TrueAbc on 2021/8/30.
//
#include "assert.h"
#include "sys/ioctl.h"
#include "tcp_client.h"

static void write_callback(event_loop* loop, int fd, void *args){
    tcp_client *cli = (tcp_client *)args;
    cli->do_write();
}

static void read_callback(event_loop* loop, int fd, void *args){
    tcp_client *cli = (tcp_client *)args;
    cli->do_read();
}


static void connection_delay(event_loop *loop, int fd, void *args){
    tcp_client *cli = (tcp_client*)args;
    loop->del_io_event(fd);

    int result = 0;
    socklen_t result_len = sizeof (result);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &result_len);
    if(result == 0){
        cli->connected = true;
        // 链接是成功的
        printf("connect %s:%d succ!\n", inet_ntoa(cli->_server_addr.sin_addr), ntohs(cli->_server_addr.sin_port));

        const char *msg = "hello lars!";
        int msgid = 1;
        cli->send_message(msg, strlen(msg), msgid);
        loop->add_io_event(fd, read_callback, kReadEvent, cli);
        if(cli->_obuf.length != 0){
            loop->add_io_event(fd, write_callback, kWriteEvent,cli);
        }
    }else{
        fprintf(stderr, "connection %s:%d error\n", inet_ntoa(cli->_server_addr.sin_addr), ntohs(cli->_server_addr.sin_port));

    }
}

tcp_client::tcp_client(event_loop *loop, const char *ip, unsigned short port, const char *name) :
_ibuf(m1M),
_obuf(m1M){
    _sockfd = -1;
    _msg_callback = nullptr;
    _name = name;
    _loop = loop;

    bzero(&_server_addr, sizeof (_server_addr));
    _server_addr.sin_family = AF_INET;
    inet_aton(ip, &_server_addr.sin_addr);
    _server_addr.sin_port = htons(port);

    _addrlen = sizeof (_server_addr);

    this->do_connect();
}

void tcp_client::do_connect() {
    if(_sockfd != -1){
        close(_sockfd);
    }

    // sock_cloexec是close on exec, not on-fork, 使用execl执行的程序会被关闭, 而使用fork的子进程不关闭
    _sockfd = socket(AF_INET, SOCK_STREAM  , IPPROTO_TCP);

    ioctl(_sockfd, FIOCLEX, 1);
    ioctl(_sockfd, FIONBIO, 1);
    if(_sockfd == -1){
        fprintf(stderr, "create tcp client socket error\n");
        exit(1);
    }

    int ret = connect(_sockfd, (const struct sockaddr*)&_server_addr, _addrlen);
    if(ret == 0){
        connected = true;

        _loop->add_io_event(_sockfd, read_callback, kReadEvent, this);

        // 如果写缓冲区有数据, 需要触发写回调
        if(this->_obuf.length != 0){
            _loop->add_io_event(_sockfd, write_callback, kWriteEvent, this);
        }
        printf("connect %s:%d succ!\n", inet_ntoa(_server_addr.sin_addr), ntohs(_server_addr.sin_port));

        const char *msg = "hello lars!";
        int msgid = 1;
        this->send_message(msg, strlen(msg), msgid);
    } else{
        if(errno == EINPROGRESS){
            // fd是非阻塞的, 可能出现, 但是并不表示失败的创建
            fprintf(stderr, "do_connect EINPROGRESS\n");
            // 进行链接成功的测试
            _loop->add_io_event(_sockfd, connection_delay, kWriteEvent, this);
        }else{
            fprintf(stderr, "connection error\n");
            exit(1);
        }
    }
}

int tcp_client::do_read() {
    //确定已经成功链接
    assert(connected == true);

    int need_read = 0;
    if(ioctl(_sockfd, FIONREAD, &need_read) == -1){
        fprintf(stderr, "ioctl fionread error");
        return -1;
    }

    assert(need_read <= _ibuf.capacity - _ibuf.length);

    int ret;
    do{
        ret = read(_sockfd, _ibuf.data + _ibuf.length, need_read);
    } while (ret == -1 && errno == EINTR);

    if(ret == 0){
        if(_name != nullptr){
            printf("%s client:connection close by peer!\n", _name);
        } else{
            printf("client: connection close by peer\n");
        }

        clean_conn();
        return -1;
    } else if(ret == -1){
        fprintf(stderr, "client: do_read(), error\n");
        clean_conn();
        return -1;
    }

    assert(ret == need_read);
    _ibuf.length += ret;

    // 2.  解包
    msg_head head;
    int msgid, length;
    while (_ibuf.length >= MESSAGE_HEAD_LEN){
        memcpy(&head, _ibuf.data + _ibuf.head, MESSAGE_HEAD_LEN);
        msgid = head.msgid;
        length = head.msglen;

        _ibuf.pop(MESSAGE_HEAD_LEN);

        // 3. 交给业务处理函数
        if(_msg_callback != nullptr){
            this->_msg_callback(_ibuf.data + _ibuf.head, length, msgid, this, nullptr);
        }
        _ibuf.pop(length);
    }

    _ibuf.adjust(); // 重置head
    return 0;
}

int tcp_client::do_write() {
    assert(_obuf.head == 0 && _obuf.length);

    int ret;
    while (_obuf.length){
        do{
            ret = write(_sockfd, _obuf.data, _obuf.length);
        } while (ret == -1 && errno == EINTR); // 非阻塞下的异常继续重写

        if(ret > 0){
            _obuf.pop(ret);
            _obuf.adjust();
        } else if(ret == -1 && errno != EAGAIN){
            fprintf(stderr, "tcp client write\n");
            this->clean_conn();
        } else{
            break;
        }

    }

    if(_obuf.length == 0){
        printf("do write over,del write event\n");
        this->_loop->del_io_event(_sockfd, kWriteEvent);
    }

    return 0;
}

// 释放并重新链接
void tcp_client::clean_conn() {
    if(_sockfd != -1){
        printf("clean conn, del socket!\n");
        _loop->del_io_event(_sockfd);
        close(_sockfd);
    }

    connected = false;
    this->do_connect();
}

tcp_client::~tcp_client() {
    close(_sockfd);
}

int tcp_client::send_message(const char *data, int msglen, int msgid) {
    if (connected == false){
        fprintf(stderr, "no connected, send message stop!\n");
        return -1;
    }

    //  如果obuf有数据, 不需要添加, 没有的话需要添加写事件, 因为在读事件中也会进行判断
    bool need_add_event = (_obuf.length == 0)? true: false;
    if(msglen + MESSAGE_HEAD_LEN > this->_obuf.capacity - _obuf.length){
        fprintf(stderr, "no more space to write socket!\n");
        return -1;
    }

    msg_head head;
    head.msglen = msglen;
    head.msgid = msgid;

    memcpy(_obuf.data + _obuf.length, &head, MESSAGE_HEAD_LEN);
    _obuf.length += MESSAGE_HEAD_LEN;

    memcpy(_obuf.data +_obuf.length, data, msglen);
    _obuf.length += msglen;
    if(need_add_event){
        _loop->add_io_event(_sockfd, write_callback, kWriteEvent, this);
    }
    return 0;
}