#pragma once
#include "ctpl_stl.h"

class ThreadPool {
public:
    static ctpl::thread_pool& GetThreadPool();

protected:
    static std::unique_ptr<ctpl::thread_pool> thread_pool;
};

