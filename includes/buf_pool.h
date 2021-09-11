//
// Created by TrueAbc on 2021/8/11.
//

#ifndef LARS_BUF_POOL_H
#define LARS_BUF_POOL_H

#include <ext/hash_map>
#include <cstring>
#include "io_buf.h"


typedef __gnu_cxx::hash_map<int, io_buf*> pool_t;

enum MEM_CAP{
    m4K = 2 << 12,
    m16K = 2 << 14,
    m64K = 2 << 16,
    m256K = 2 << 18,
    m1M = 2 << 20,
    m4M = 2 << 22,
    m8M = 2 << 24,
};

// 限制总的内存大小, 单位是5GB
#define EXTRA_MEM_LIMIT (5U * (2 << 20))

class buf_pool{
public:
    //  单例模式
    static void init(){
        _instance = new buf_pool();
    }

    // 获取单例的方法
    static buf_pool* instance(){
        pthread_once(&_once, init);
        return _instance;
    }

    io_buf *alloc_buf(int N);
    io_buf *alloc_buf(){return alloc_buf(m4K);};

    void revert(io_buf *buffer); // 重置一个io_buf

private:
    buf_pool();

    buf_pool(const buf_pool&);
    const buf_pool& operator=(const buf_pool&);

    pool_t _pool; // buffer 的map集合

    uint64_t _total_mem; // 总内存大小
    static buf_pool* _instance;

    static pthread_once_t  _once; // 保证init方法只执行一次

    static pthread_mutex_t _mutex;
};

#endif //LARS_BUF_POOL_H
