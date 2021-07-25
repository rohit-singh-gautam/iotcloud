////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <pthread.h>

namespace rohit {

template <bool multi_thread>
struct pthread_lock_c {
    inline void lock() {}
    inline void unlock() {}
};

template <>
struct pthread_lock_c<true> {
    pthread_mutex_t _lock;

    inline pthread_lock_c() {
        pthread_mutex_init(&_lock, nullptr);
    }

    inline ~pthread_lock_c() {
        pthread_mutex_destroy(&_lock);
    }

    inline void lock() {
        pthread_mutex_lock(&_lock);
    }
    inline void unlock() {
        pthread_mutex_unlock(&_lock);
    }
};

} // namespace rohit