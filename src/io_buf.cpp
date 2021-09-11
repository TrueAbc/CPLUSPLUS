//
// Created by TrueAbc on 2021/8/11.
//

#include <cassert>
#include <cstring>
#include "io_buf.h"

io_buf::io_buf(int size): capacity(size), length(0),
head(0), next(nullptr){
    data = new char [size];
    assert(data);
}

void io_buf::clear() {
    length = head = 0;
}

// 将已经处理的数据清空, 将没有处理的数据提前到数据首地址
void io_buf::adjust() {
    if(head != 0){
        if(length !=0){
            memmove(data, data+head, length);
        }
        head = 0;
    }
}

void io_buf::copy(const io_buf *other) {
    memcpy(data, other->data + other->head, other->length);
    head = 0;
    length = other->length;
}

void io_buf::pop(int len) {
    length -= len;
    // 可能会存在覆盖问题, 这里删除数据
    memset(this->data + this->head, '\0', len);
    head += len;
}