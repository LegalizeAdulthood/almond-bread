/* 
 * thread.h --
 *
 *	"Portable" Multithreading support.
 *
 * Copyright (c) 1996-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef THREAD_H
#define THREAD_H

#include "conf.h"

#ifdef MULTITHREADING
#ifdef POSIX_THREADS
#include <pthread.h>
#define Thread_t pthread_t
typedef struct Condition_t
{
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  int value;
} Condition_t;
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
#define Thread_t int
#define Condition_t int
#endif /* MULTITHREADING */

#define START_THREAD 1
#define STOP_THREAD 2

static inline int CreateThread(Thread_t thread,
			       void * (*start_routine)(void *),
			       void * arg)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  pthread_attr_t attr;
  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  return pthread_create(&thread,&attr,start_routine,arg);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  start_routine(arg);
  return 0;
#endif /* MULTITHREADING */
}

static inline int CreateThreadJoinable(Thread_t thread,
				       void * (*start_routine)(void *),
				       void * arg)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  return pthread_create(&thread,NULL,start_routine,arg);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  start_routine(arg);
  return 0;
#endif /* MULTITHREADING */
}

static inline int CancelThread(Thread_t thread)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  return pthread_cancel(thread);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}  

static inline int SetThreadAsynchronous(void)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  int ret;
  if((ret=pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,(int *)NULL))!=0)
    return ret;
  else
    return pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,(int *)NULL);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

static inline int JoinThread(Thread_t thread, void **return_value)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  return pthread_join(thread,return_value);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

static inline Thread_t DefaultThread(void)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  return pthread_self();
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

static inline int InitCondition(Condition_t *cond)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  cond->value=0;
  pthread_cond_init(&(cond->cond),NULL);
  return pthread_mutex_init(&(cond->mutex),NULL);
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

static inline int SignalCondition(Condition_t *cond, int value)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  pthread_mutex_lock(&(cond->mutex));
  cond->value=value;
  pthread_cond_signal(&(cond->cond));
  return pthread_mutex_unlock(&(cond->mutex));
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

static inline int WaitCondition(Condition_t *cond, int value)
{
#ifdef MULTITHREADING
#ifdef POSIX_THREADS
  pthread_mutex_lock(&(cond->mutex));
  while(cond->value!=value)
    pthread_cond_wait(&(cond->cond),&(cond->mutex));
  return pthread_mutex_unlock(&(cond->mutex));
#endif /* POSIX_THREADS */
#else /* MULTITHREADING */
  return 0;
#endif /* MULTITHREADING */
}

#endif /* THREAD_H */
