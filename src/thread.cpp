#include "../include/thread.hpp"

#ifdef WIN32
#include <process.h>

CThread::CThread(void)
{
    m_hThread = NULL;
    m_dwThreadId = 0;
}

CThread::~CThread(void)
{
    if (m_hThread != NULL)
    {
        CloseHandle(m_hThread);

        m_hThread = NULL;
    }
}

bool CThread::Start(bool bSuspend, unsigned stack_size)
{
    if (bSuspend)
    {
        m_hThread = (HANDLE)_beginthreadex(0, stack_size, StaticThreadFunc, this, CREATE_SUSPENDED, &m_dwThreadId);
    }
    else
    {
        m_hThread = (HANDLE)_beginthreadex(0, stack_size, StaticThreadFunc, this, 0, &m_dwThreadId);
    }

    if (NULL == m_hThread)
    {
        return false;
    }

    return true;
}

void CThread::WaitFor(UINT32 dwWaitTime)
{
    if (NULL == m_hThread)
    {
        return;
    }

    WaitForSingleObject(m_hThread, dwWaitTime);
}

unsigned int _stdcall CThread::StaticThreadFunc(void *arg)
{
    unsigned dwRet = 0;

    CThread* pThread = (CThread*)arg;

    __try
    {
        pThread->ThreadProc();
    }
    __except (0)
    {
        dwRet = 1;
    }

    return dwRet;
}

#else

CThread::CThread(void)
{
    m_stThread = 0;
}

CThread::~CThread(void)
{
}

bool CThread::Start(bool bSuspend)
{
    if (0 != pthread_create(&m_stThread, NULL, CThread::StaticThreadFunc, this))
    {
        m_stThread = 0;
        return false;
    }
    return true;
}

void CThread::WaitFor(UINT32 dwWaitTime)
{
    if (0 == m_stThread)
    {
        return;
    }
    pthread_join(m_stThread, NULL);
}

void * CThread::StaticThreadFunc(void* arg)
{
    CThread* pThread = (CThread*)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    sigset_t new_set, old_set;
    sigemptyset(&new_set);
    sigemptyset(&old_set);
    sigaddset(&new_set, SIGHUP);
    sigaddset(&new_set, SIGINT);
    sigaddset(&new_set, SIGQUIT);
    sigaddset(&new_set, SIGTERM);
    sigaddset(&new_set, SIGUSR1);
    sigaddset(&new_set, SIGUSR2);
    sigaddset(&new_set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &new_set, &old_set);

    pThread->ThreadProc();

    return NULL;
}

pthread_t GetCurrentThreadId(void)
{
    return pthread_self();
}

#endif

