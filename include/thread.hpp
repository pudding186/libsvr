#pragma once
#include "./type_def.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE  THREAD_HANDLE;

class CThreadLock
{
public:
    CThreadLock(void)
    {
        InitializeCriticalSection(&m_stCritical_section);
    }
public:
    ~CThreadLock(void)
    {
        DeleteCriticalSection(&m_stCritical_section);
    }

    inline void Lock(void)
    {
        EnterCriticalSection(&m_stCritical_section);
    }

    inline void UnLock(void)
    {
        LeaveCriticalSection(&m_stCritical_section);
    }
protected:
    CRITICAL_SECTION    m_stCritical_section;
};

class CThread
{
public:
    CThread(void);
public:
    virtual ~CThread(void);

    virtual void ThreadProc(void) = 0;

    bool Start(bool bSuspend, unsigned stack_size = 0);

    void WaitFor(UINT32 dwWaitTime = INFINITE);

    inline THREAD_HANDLE GetThreadHandle(void) { return m_hThread; }
    inline UINT32 GetThreadId(void) { return m_dwThreadId; }

private:
    static unsigned int _stdcall StaticThreadFunc(void *arg);
protected:
    HANDLE  m_hThread;
    UINT32  m_dwThreadId;
};
#else
#include <pthread.h>
#include <signal.h>
#define INFINITE    0xffffffff
typedef pthread_t   THREAD_HANDLE;

#include <pthread.h>
class CThreadLock
{
public:
    CThreadLock(void)
    {
        pthread_mutex_init(&m_stpthread_mutex, NULL);
    }
    ~CThreadLock(void)
    {
        pthread_mutex_destroy(&m_stpthread_mutex);
    }

    inline void Lock(void)
    {
        pthread_mutex_lock(&m_stpthread_mutex);
    }

    inline void UnLock(void)
    {
        pthread_mutex_unlock(&m_stpthread_mutex);
    }
protected:
    pthread_mutex_t m_stpthread_mutex;
};

class CThread
{
public:
    CThread(void);
public:
    virtual ~CThread(void);

    virtual void ThreadProc(void) = 0;

    bool Start(bool bSuspend);

    void WaitFor(UINT32 dwWaitTime = INFINITE);

    inline THREAD_HANDLE GetThreadHandle(void) { return m_stThread; }
    inline pthread_t GetThreadId(void) { return m_stThread; }
private:
    static void *StaticThreadFunc(void* arg);
protected:
    pthread_t   m_stThread;

};

extern pthread_t GetCurrentThreadId(void);
#endif

