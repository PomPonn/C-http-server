#include "thread.h"

#ifdef WIN32

struct _win_thread_start_t {
  void* (*start_routine)(void*);
  void* start_arg;
};

static DWORD WINAPI _win_thread_start(void* arg) {
  struct _win_thread_start_t* data = arg;
  void* (*start_routine)(void*) = data->start_routine;
  void* start_arg = data->start_arg;

  HeapFree(GetProcessHeap(), 0, data);

  start_routine(start_arg);

  return 0;
}

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
  if (!thread || !start_routine)
    return NULL;

  struct _win_thread_start_t* data =
    HeapAlloc(GetProcessHeap(), 0, sizeof(*data));
  data->start_routine = start_routine;
  data->start_arg = arg;

  *thread = CreateThread(NULL, 0, _win_thread_start, data, 0, NULL);
  if (*thread == NULL)
    return 1;
  return 0;
}

#else

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
  if (pthread_create(thread, NULL, start_routine, arg) != 0)
    return 1;
  return 0;
}

#endif