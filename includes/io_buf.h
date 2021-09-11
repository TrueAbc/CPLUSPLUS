//
// Created by TrueAbc on 2021/8/11.
//

#ifndef LARS_IO_BUF_H
#define LARS_IO_BUF_H

class io_buf{
public:
    io_buf(int size);

    void clear(); // 清除数据
    void adjust(); //  将已经处理的数据清空, 未处理的数据提前到数据首地址
    void copy(const io_buf* other);
    void pop(int len); // 处理长度为len的数据, 移动head和修正length

    io_buf* next; //

    int capacity;
    int length;
    int head; // 未处理数据的头部位置索引
    char *data; // 数据的真实存放地址
};
#endif //LARS_IO_BUF_H
