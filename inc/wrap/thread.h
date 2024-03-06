#pragma once

#ifdef _WIN32 // Windows 32bit + 64 bit

#include <windows.h>

typedef HANDLE thread_t;

#else

#include <pthread.h>

typedef pthread_t thread_t;
typedef void* (*start_routine)(void*) thread_start_routine;

#endif

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg);