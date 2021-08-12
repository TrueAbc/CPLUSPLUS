//
// Created by TrueAbc on 2021/8/11.
//


#include "buf_pool.h"

void pointer_test(void *ptr, const char *state){
    if(ptr == nullptr){
        fprintf(stderr, state);
        exit(1);
    }
}

buf_pool * buf_pool::_instance = NULL;

pthread_once_t buf_pool::_once = PTHREAD_ONCE_INIT;

pthread_mutex_t buf_pool::_mutex = PTHREAD_MUTEX_INITIALIZER;

buf_pool::buf_pool() : _total_mem(0){
    io_buf *prev;
    // 4K 内存池
    _pool[m4K] = new io_buf(m4K);
    pointer_test(_pool[m4K], "new io_buf m4k error");

    prev = _pool[m4K];
    for (int i = 0; i < 5000; ++i) {
        prev->next = new io_buf(m4K);
        pointer_test(prev->next, "new io_buf m4k error");
        prev = prev->next;
    }

    _total_mem += 4 * 5001;

    // 16k  内存池
    _pool[m16K] = new io_buf(m16K);
    pointer_test(_pool[m16K], "new io_buf m16k error");
    prev = _pool[m16K];
    for (int i = 1; i < 1000; ++i) {
        prev->next = new io_buf(m16K);
        pointer_test(prev->next, "mem io_buf m16k error");
        prev = prev->next;
    }
    _total_mem += 16 * 1000;

    //64K buf
    _pool[m64K] = new io_buf(m64K);
    pointer_test(_pool[m64K], "new io_buf m64k error");
    prev = _pool[m64K];
    for (int i = 0; i < 499; ++i) {
        prev->next = new io_buf(m64K);
        pointer_test(prev->next, "mem io_buf m64k error");
        prev = prev->next;
    }
    _total_mem += 64*500;

    //256K buf
    _pool[m256K] = new io_buf(m256K);
    pointer_test(_pool[m256K], "new io_buf m256k error");
    prev = _pool[m256K];
    for (int i = 0; i < 199; ++i) {
        prev->next = new io_buf(m256K);
        pointer_test(prev->next, "mem io_buf m256k error");
        prev = prev->next;
    }
    _total_mem += 256 * 200;

    // 1M
    _pool[m1M] = new io_buf(m1M);
    pointer_test(_pool[m1M], "new io_buf m1M error");
    prev = _pool[m1M];
    for (int i = 0; i < 49; ++i) {
        prev->next = new io_buf(m1M);
        pointer_test(prev->next, "mem io_buf m1M error");
        prev = prev->next;
    }
    _total_mem += 1024 * 50;

    // 4M
    _pool[m4M] = new io_buf(m4M);
    pointer_test(_pool[m4M], "new io_buf m4M error");
    prev = _pool[m4M];
    for (int i = 0; i < 19; ++i) {
        prev->next = new io_buf(m4M);
        pointer_test(prev->next, "mem io_buf m4M error");
        prev = prev->next;
    }
    _total_mem += 4096 * 20;

    // 4M
    _pool[m8M] = new io_buf(m8M);
    pointer_test(_pool[m8M], "new io_buf m8M error");
    prev = _pool[m8M];
    for (int i = 0; i < 9; ++i) {
        prev->next = new io_buf(m8M);
        pointer_test(prev->next, "mem io_buf m8M error");
        prev = prev->next;
    }
    _total_mem += 8192 * 10;
}


// 开辟一个io_buf
// 1. 上层需要N个字节, 找到与N最接近的buf组
// 2. 如果该组节点没有了, 额外申请
// 3. 总申请长度不能超过 limit
// 4. 如果有该节点需要的内存块, 直接取出, 将pool摘除
io_buf *buf_pool::alloc_buf(int N) {
    int index;
    if(N<=m4K){
        index = m4K;
    } else if( N <= m16K){
        index = m16K;
    } else if (N <= m64K){
        index = m64K;
    } else if (N <= m256K){
        index = m256K;
    } else if (N <= m1M){
        index = m1M;
    } else if (N <= m4M){
        index = m4M;
    } else if (N <= m8M){
        index = m8M;
    } else {
        fprintf(stderr, "too much memory asked");
        exit(1);
    }

    pthread_mutex_lock(&_mutex);
    if (_pool[index] == nullptr){
        if(_total_mem + index / 1024 >= EXTRA_MEM_LIMIT){
            //  超过了限制
            fprintf(stderr, "already use too many memory!\n");
            exit(1);
        }

        io_buf *new_buf = new io_buf(index);
        if (new_buf == nullptr){
            fprintf(stderr, "new io_buf error\n");
            exit(1);
        }
        _total_mem += index / 1024;
        pthread_mutex_unlock(&_mutex);
        return new_buf;
    }

    // 3. 使用本地的pool的内存
    io_buf* target = _pool[index];
    _pool[index] = target->next;
    pthread_mutex_unlock(&_mutex);
    target->next = nullptr;

    return target;
}

// 重置一个io_buf, 返回到pool中
void buf_pool::revert(io_buf *buffer) {
    int index = buffer->capacity;

    buffer->length = buffer->head = 0;

    pthread_mutex_lock(&_mutex);
    assert(_pool.find(index) != _pool.end());

    buffer->next = _pool[index];
    _pool[index] = buffer;

    pthread_mutex_unlock(&_mutex);
}