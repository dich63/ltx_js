#pragma once

#include <windows.h>

typedef struct
{
  int waiters_count_;
  // Number of waiting threads.

  CRITICAL_SECTION waiters_count_lock_;
  // Serialize access to <waiters_count_>.

  HANDLE sema_;
  // Semaphore used to queue up threads waiting for the condition to
  // become signaled. 

  HANDLE waiters_done_;
  // An auto-reset event used by the broadcast/signal thread to wait
  // for all the waiting thread(s) to wake up and be released from the
  // semaphore. 

  size_t was_broadcast_;
  // Keeps track of whether we were broadcasting or signaling.  This
  // allows us to optimize the code if we're just signaling.
} pthread_cond_t;

typedef HANDLE pthread_mutex_t;
typedef void pthread_condattr_t;
typedef void pthread_mutexattr_t;

inline 
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr=0)
{
 return (*mutex=CreateMutex(0,0,0))!=0;
};

inline 
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return CloseHandle(*mutex);
};

inline 
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	WaitForSingleObject(*mutex,INFINITE);
	return 1;
};

inline 
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	ReleaseMutex(*mutex);
	return 1;
};


inline 
int 
pthread_cond_destroy(pthread_cond_t *cv)
{

 CloseHandle(cv->waiters_done_);
 DeleteCriticalSection(&cv->waiters_count_lock_);
 CloseHandle(cv->sema_);
 return 1;
};

inline   int pthread_cond_init (pthread_cond_t *cv, const pthread_condattr_t * p=0)
{
  cv->waiters_count_ = 0;
  cv->was_broadcast_ = 0;
  cv->sema_ = CreateSemaphore (NULL,       // no security
                                0,          // initially 0
                                0x7fffffff, // max count
                                NULL);      // unnamed 
  InitializeCriticalSection (&cv->waiters_count_lock_);
  cv->waiters_done_ = CreateEvent (NULL,  // no security
                                   FALSE, // auto-reset
                                   FALSE, // non-signaled initially
                                   NULL); // unnamed
  return 1;
};

inline 
void
pthread_cond_wait (pthread_cond_t *cv, 
                   pthread_mutex_t *external_mutex)
{
  // Avoid race conditions.
  EnterCriticalSection (&cv->waiters_count_lock_);
  cv->waiters_count_++;
  LeaveCriticalSection (&cv->waiters_count_lock_);

  // This call atomically releases the mutex and waits on the
  // semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
  // are called by another thread.
  SignalObjectAndWait (*external_mutex, cv->sema_, INFINITE, FALSE);

  // Reacquire lock to avoid race conditions.
  EnterCriticalSection (&cv->waiters_count_lock_);

  // We're no longer waiting...
  cv->waiters_count_--;

  // Check to see if we're the last waiter after <pthread_cond_broadcast>.
  int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

  LeaveCriticalSection (&cv->waiters_count_lock_);

  // If we're the last waiter thread during this particular broadcast
  // then let all the other threads proceed.
  if (last_waiter)
    // This call atomically signals the <waiters_done_> event and waits until
    // it can acquire the <external_mutex>.  This is required to ensure fairness. 
    SignalObjectAndWait (cv->waiters_done_, *external_mutex, INFINITE, FALSE);
  else
    // Always regain the external mutex since that's the guarantee we
    // give to our callers. 
    WaitForSingleObject (*external_mutex,INFINITE);
}

inline 
void
pthread_cond_signal (pthread_cond_t *cv)
{
  EnterCriticalSection (&cv->waiters_count_lock_);
  int have_waiters = cv->waiters_count_ > 0;
  LeaveCriticalSection (&cv->waiters_count_lock_);

  // If there aren't any waiters, then this is a no-op.  
  if (have_waiters)
    ReleaseSemaphore (cv->sema_, 1, 0);
};

inline 
void
pthread_cond_broadcast (pthread_cond_t *cv)
{
  // This is needed to ensure that <waiters_count_> and <was_broadcast_> are
  // consistent relative to each other.
  EnterCriticalSection (&cv->waiters_count_lock_);
  int have_waiters = 0;

  if (cv->waiters_count_ > 0) {
    // We are broadcasting, even if there is just one waiter...
    // Record that we are broadcasting, which helps optimize
    // <pthread_cond_wait> for the non-broadcast case.
    cv->was_broadcast_ = 1;
    have_waiters = 1;
  }

  if (have_waiters) {
    // Wake up all the waiters atomically.
    ReleaseSemaphore (cv->sema_, cv->waiters_count_, 0);

    LeaveCriticalSection (&cv->waiters_count_lock_);

    // Wait for all the awakened threads to acquire the counting
    // semaphore. 
    WaitForSingleObject (cv->waiters_done_, INFINITE);
    // This assignment is okay, even without the <waiters_count_lock_> held 
    // because no other waiter threads can wake up to access it.
    cv->was_broadcast_ = 0;
  }
  else
    LeaveCriticalSection (&cv->waiters_count_lock_);
}


#define PTHREAD_BARRIER_SERIAL_THREAD 1 


typedef struct 
{ 
	union{
	struct{
	HANDLE hEvt; 
	HANDLE hEvtAbort; 
	HANDLE hEvtSync; 
	};
    struct
	{
     HANDLE hwait[3];   
	};
	};
	volatile LONG lCount; 
	LONG lOrgCount; 
	void (*pabort_proc)(void);
} pthread_barrier_t; 


inline int pthread_barrier_abort(pthread_barrier_t* barrier)
{
   if(!barrier) return EINVAL;
   if(barrier->pabort_proc)
   {
     SetEvent(barrier->hEvtAbort);
   }
   return 0;
}

inline int pthread_barrier_init(pthread_barrier_t* barrier, unsigned count,void (*abort_proc)(void)=NULL) 
{ 
	if(!barrier) return EINVAL;;
	if(count)
	{
    	barrier->hEvt = CreateEvent(NULL, FALSE, FALSE, NULL); 
    	barrier->hEvtAbort = CreateEvent(NULL, TRUE, FALSE, NULL); 
    	barrier->hEvtSync = CreateEvent(NULL, TRUE, FALSE, NULL); 
    	barrier->lCount = count; 
    	barrier->lOrgCount = count; 
        barrier->pabort_proc=abort_proc;
	}
	else memset(barrier,0,sizeof(pthread_barrier_t));
	return 0; 
} 


inline  int pthread_barrier_wait(pthread_barrier_t *barrier) 
{ 
	if (barrier == NULL) 
		return EINVAL; 
	LONG lLast = InterlockedDecrement(&barrier->lCount); 
	if (lLast == 0) 
	{ 
		// Stelle sicher, dass nacher alle wieder anhalten... 
		ResetEvent(barrier->hEvtSync); 
		SetEvent(barrier->hEvt); 
	} 

	//Hier kommt immer nur *einer* raus... 
	//WaitForSingleObject(barrier->hEvt, INFINITE); 

	 if(WAIT_OBJECT_0!=WaitForMultipleObjects(2,barrier->hwait,false,INFINITE))
		 if(barrier->pabort_proc) barrier->pabort_proc();
		  

	// So, gehe jetzt wieder anders rum durch, bis ich bei lOrgCount bin 
	LONG lLastNext = InterlockedIncrement(&barrier->lCount); 
	if (lLastNext == barrier->lOrgCount) 
	{ 
		// jetzt sind alle wieder drausen, also bin ich wieder synchron... 
		SetEvent(barrier->hEvtSync); 
	} 
	else 
	{ 
		// Hab den Zähler noch nicht erreicht, lasse somit den nächsten raus... 
		SetEvent(barrier->hEvt); 
		// ... und warte bis alle drausen sind 
	//	WaitForSingleObject(barrier->hEvtSync, INFINITE); 
		if(WAIT_OBJECT_0==WaitForMultipleObjects(2,barrier->hwait+1,false,INFINITE))
			if(barrier->pabort_proc) barrier->pabort_proc();

	} 

	if (lLast == 0) 
	{ 
		return PTHREAD_BARRIER_SERIAL_THREAD; 
	} 
	return 0; 
} 

inline int pthread_barrier_destroy(pthread_barrier_t *barrier) 
{ 
	if(barrier->lOrgCount)
	{
	CloseHandle(barrier->hEvt); 
	CloseHandle(barrier->hEvtAbort); 
	CloseHandle(barrier->hEvtSync); 
	};
	return 0; 
} 
