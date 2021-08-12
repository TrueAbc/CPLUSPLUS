//
// Created by TrueAbc on 2021/8/11.
//

#include "reactor_buf.h"
#include <sys/ioctl.h>

reactor_buf::reactor_buf() {
    _buf = nullptr;
}

reactor_buf::~reactor_buf() {
    clear();
}

void reactor_buf::clear() {
    if(_buf != nullptr){
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

const int reactor_buf::length() const {
    return _buf != nullptr ? _buf->length : 0;
}

void reactor_buf::pop(int len) {
    assert(_buf != nullptr && len <= _buf->length);

    _buf->pop(len);

    if(_buf->length == 0){
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

int input_buf::read_data(int fd) {
    int need_read;// 硬件可以读取的数据量

    if(ioctl(fd, FIONREAD, &need_read) == -1){
        fprintf(stderr, "ioctl fionread\n");
        return -1;
    }

    if(_buf == nullptr){
        _buf = buf_pool::instance()->alloc_buf(need_read);
        if(_buf == nullptr){
            fprintf(stderr, "no idle buf for alloc\n");
            return -1;
        }
    } else{
        // io_buf可用, 判断大小是否满足
        assert(_buf->head == 0);
        if(_buf->capacity - _buf->length <(int)need_read){
            io_buf* new_buf = buf_pool::instance()->alloc_buf(need_read);
            if(new_buf == nullptr){
                fprintf(stderr, "no idle buf for alloc\n");
                return -1;
            }

            new_buf->copy(_buf);
            buf_pool::instance()->revert(_buf);
            _buf = new_buf;
        }
    }

    int already_read = 0;
    // todo 这部分的循环不是很理解
    do{
        if(need_read ==0){
            // 可能是read阻塞了读数据模式, 对方没有写数据
            already_read = read(fd, _buf->data + _buf->length, m4K);
        }else {
            already_read = read(fd, _buf->data + _buf->length, need_read);
        }
    } while (already_read == -1 && errno == EINTR);// systemCall 的中断

    if(already_read >0 ){
        if(need_read != 0){
            assert(need_read == already_read);
        }
        _buf->length += already_read;
    }

    return already_read;
}

const char *input_buf::data() const {
    return _buf!= nullptr ? _buf->data + _buf->head : nullptr;
}

void input_buf::adjust() {
    if(_buf != nullptr){
        _buf->adjust();
    }
}

int out_buf::send_data(const char *data, int datalen) {
    if(_buf == nullptr){
        _buf = buf_pool::instance()->alloc_buf(datalen);
        if (_buf == nullptr){
            fprintf(stderr, "no idle buf for alloc\n");
            return -1;
        }
    } else {
        assert(_buf->head == 0);
        if(_buf->capacity - _buf->length < datalen){
            io_buf *new_buf = buf_pool::instance()->alloc_buf(datalen+_buf->length);
            if(new_buf == nullptr){
                fprintf(stderr, "no idle buf for alloc\n");
                return -1;
            }

            new_buf->copy(_buf);
            buf_pool::instance()->revert(_buf);
            _buf = new_buf;
        }
    }
    memcpy(_buf->data + _buf->length, data, datalen);
    _buf->length += datalen;

    return 0;
}

int out_buf::write2fd(int fd) {
    assert(_buf != nullptr && _buf->head == 0);
    int already_write = 0;

    do{
        already_write = write(fd, _buf->data, _buf->length);
    } while (already_write == -1 && errno == EINTR);

    if(already_write > 0){
        _buf ->pop(already_write);
        _buf->adjust();
    }

    //fd 为非阻塞模式, 可能会得到EAGAIN错误
    if (already_write == -1 && errno == EAGAIN){
        already_write = 0; // 表示当前是不可以继续写的
    }

    return already_write;
}