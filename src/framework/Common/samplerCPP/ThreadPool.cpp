#include "ThreadPool.h"
ctpl::thread_pool& ThreadPool::GetThreadPool() {
    if (thread_pool == nullptr) {
        thread_pool = std::make_unique<ctpl::thread_pool>(std::thread::hardware_concurrency());
    }
    return *thread_pool;
}

std::unique_ptr<ctpl::thread_pool> ThreadPool::thread_pool = nullptr;
