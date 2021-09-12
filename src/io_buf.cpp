//
// Created by TrueAbc on 2021/8/11.
//

#include <cassert>
#include <cstring>
#include "io_buf.h"

io_buf::io_buf(int size): capacity(size), length(0),
head(0), next(nullptr){
    // 最后一个位置填充\0 方便打印
    data = new char [size+1];
    data[size] = '\0';
    assert(data);
}

void io_buf::clear() {
    length = head = 0;
    memset(data, '\0', capacity);
}

// 将已经处理的数据清空, 将没有处理的数据提前到数据首地址
void io_buf::adjust() {
    if(head != 0){
        if(length !=0){
            memmove(data, data+head, length);
            // 0-lenght-1是原始数据, 剩下的是被覆盖的字符
            memset(data+length, '\0', capacity-length);
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