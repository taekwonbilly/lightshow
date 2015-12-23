#ifndef THREAD_H
#define THREAD_H

#if defined(LINUX)
#include <pthread.h>
typedef void* (* LINUX_THREAD_FUNC_PTR)(void*);
#define OS_THREAD_FUNC_PTR LINUX_THREAD_FUNC_PTR
#elif defined(WIN32)
#include <windows.h> // defines LPTHREAD_START_ROUTINE
#define OS_THREAD_FUNC_PTR LPTHREAD_START_ROUTINE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timeb.h>

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef THREAD_C
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct Thread {
#if defined(LINUX)
  pthread_t          info;
#elif defined(WIN32)
  HANDLE             handle;
#endif
  OS_THREAD_FUNC_PTR start_func;
  void* user_ptr;
}* ThreadPtr;

typedef double real;

EXTERN inline real seconds() {
  struct timeb t;
  ftime(&t);
  return (real)t.time + 0.001 * (real)t.millitm;
}

EXTERN inline void thread_sleep(double sleep_time) {
#ifdef LINUX
  double start_time = seconds();
  while(seconds() - start_time < sleep_time)
    sched_yield();
#else
#error thread_sleep not defined for this os.
#endif
}

EXTERN inline int thread_wait_int(int* integer, double max_time) {
#ifdef LINUX
  double start_time = seconds();
  while(seconds() - start_time < max_time) {
    if(*integer)
      return *integer;
    sched_yield();
  }
  return *integer;
#else
#error thread_wait_int not defined for this os.
#endif  
}

EXTERN inline void thread_exit() {
#if defined(LINUX)
  pthread_exit(0);
#elif defined(WIN32)
  ExitThread(0);
#endif
}

EXTERN inline void thread_run(ThreadPtr this) {
#if defined(LINUX)
  pthread_t info;
  if(pthread_create(&info,
		    NULL,
		    this->start_func,
		    this->user_ptr) != 0) {
    fprintf(stderr, "Thread ERROR: creating thread.\n");
    exit(-1);
  }
#elif defined(WIN32)
  
  fprintf(stderr, "creating thread.\n");
  handle = CreateThread(NULL,	          // no security attributes
			0,	          // use default stack size
			this->start_func, // thread function
			this->user_ptr,   // argument to thread function
			0,	          // use default creation flags
			NULL);	          // returns the thread identifier
  
  
  if(handle == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Could not create simulation thread");
    exit(-1);
  }
#endif
}

EXTERN inline void thread_init(ThreadPtr this,
			       void* (*start_func)(void*),
			       void* user_ptr) {
  this->start_func = (OS_THREAD_FUNC_PTR)start_func;
  this->user_ptr   = user_ptr;
}

EXTERN inline void thread_destroy(ThreadPtr this) {
  // we do nothing for two reasons:
  //   1. terminating threads is dangerous under windows.
  //   2. pthreads doesn't support thread termination.
}

#endif // THREAD_H

