#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/utility.hpp"
#include "../include/share_memory.h"
#include "../include/file_log.h"

#include <intrin.h>
class RandomNumGenarator
{
public:
    RandomNumGenarator(long long lSeed1 = 0, long long lSeed2 = 0)
    {
        if (!lSeed1)
        {
            m_lSeed1 = __rdtsc();
        }
        else
            m_lSeed1 = lSeed1;

        if (!lSeed2)
        {
            m_lSeed2 = __rdtsc();
        }
        else
            m_lSeed2 = lSeed2;
    }

    int GetOneRandNum(long _tModNum)
    {
        int _ResultNum = 0;

        long long lTempNum = 0;
        lTempNum = (a1 * m_lSeed1 + c1) % m1 - (a2 * m_lSeed2 + c2) % m2;
        if (lTempNum < 0) {
            //lTempNum += m - 1;
        }

        _ResultNum = static_cast<int>(lTempNum);
        m_lSeed1 = (a1 * m_lSeed1 + c1) % m1;
        m_lSeed2 = (a2 * m_lSeed2 + c2) % m2;

        if (0 == _tModNum) {
            return _ResultNum;
        }
        else
        {
            return _ResultNum % _tModNum;
        }
    }

    long long GetOneRandNum64(long long _tModNum)
    {
        long long _ResultNum = 0;

        long long lTempNum = 0;
        lTempNum = (a1 * m_lSeed1 + c1) % m1 - (a2 * m_lSeed2 + c2) % m2;
        if (lTempNum < 0) {
            //lTempNum += m - 1;
        }

        _ResultNum = lTempNum;
        m_lSeed1 = (a1 * m_lSeed1 + c1) % m1;
        m_lSeed2 = (a2 * m_lSeed2 + c2) % m2;

        if (0 == _tModNum) {
            return _ResultNum;
        }
        else
        {
            return _ResultNum % _tModNum;
        }
    }
protected:
private:
    long long m_lSeed1;
    long long m_lSeed2;

    static const long long m1 = 2147483563;
    static const long long m2 = 2147483399;
    static const long long m = m2;
    static const long long a1 = 40014;
    static const long long a2 = 40692;
    static const long long c1 = 12211;
    static const long long c2 = 3791;
};

int rand_int(int min, int max)
{
    static RandomNumGenarator rnd_gen;

    int range = max - min;
    if (range <= 0)
        return min;

    int rnd = rnd_gen.GetOneRandNum(range + 1);
    rnd = abs(rnd);

    return min + rnd;
}

long long rand_int64(long long min, long long max)
{
    static RandomNumGenarator rnd_gen64;

    long long range = max - min;
    if (range <= 0)
        return min;

    long long rnd = rnd_gen64.GetOneRandNum64(range + 1);
    rnd = _abs64(rnd);

    return min + rnd;
}

unsigned int BKDRHash(const char* str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash = hash * 131 + (*str++);
    }

    return hash;
}

unsigned long long BKDRHash64(const char* str)
{
    unsigned long long hash = 0;
    while (*str)
    {
        hash = hash * 131 + (*str++);
    }

    return hash;
}

//////////////////////////////////////////////////////////////////////////

#define MAX_STACK_CAPACITY	1000

class CFuncPerformanceMgr
{
    friend class CFuncPerformanceCheck;
public:
    CFuncPerformanceMgr(void)
    {
        m_func_list = 0;
        m_shm_mgr = 0;
        m_func_stack = 0;
        m_shm_key = 0;
        m_cur_func_perf_info = 0;
    }
    ~CFuncPerformanceMgr(void)
    {

    }

    inline void RecordFunc(CFuncPerformanceInfo* func_perf_info)
    {
        func_perf_info->next = m_func_list;
        m_func_list = func_perf_info;
    }

    CFuncPerformanceInfo* FuncPerfFirst(void)
    {
        return m_func_list;
    }

    int StackTop(void)
    {
        return m_func_stack->m_top;
    }

    CFuncPerformanceInfo* StackFuncPerfInfo(int idx)
    {
        if (idx >= 0 && idx < m_func_stack->m_top)
        {
            return m_func_stack->m_stack[idx];
        }

        return 0;
    }

    const char* UpdateStackInfo(void)
    {
        size_t stack_info_len = 0;

        for (int i = 0; i < m_func_stack->m_top; i++)
        {
#ifdef WIN32
            _snprintf_s(m_stack_info + stack_info_len, sizeof(m_stack_info) - stack_info_len, _TRUNCATE, "call %s()\n", m_func_stack->m_stack[i]->func_name);
#else
            snprintf(m_stack_info + stack_info_len, sizeof(m_stack_info) - stack_info_len, "call %s()\n", m_func_stack->m_stack[i]->func_name);
#endif
            stack_info_len = strnlen(m_stack_info, sizeof(m_stack_info));
        }

        m_stack_info[stack_info_len - 1] = '\0';

        return m_stack_info;
    }

    struct func_stack
    {
        int						m_top;
        CFuncPerformanceInfo*	m_stack[MAX_STACK_CAPACITY];

        inline void Push(CFuncPerformanceInfo* func_perf_info)
        {
            if (m_top < MAX_STACK_CAPACITY)
            {
                m_stack[m_top] = func_perf_info;
            }
            ++m_top;
        }

        inline void Pop(void)
        {
            if (m_top > 0)
            {
                --m_top;
            }
            else
            {
                char*a = 0;
                *a = 'a';
            }
        }
    };

    bool Init(int shm_key)
    {
        if (m_shm_mgr)
        {
            return true;
        }

        if (!shm_key)
        {
            m_shm_key = rand_int(1, INT_MAX);
        }
        else
            m_shm_key = shm_key;

        m_shm_mgr = create_shm_mgr();

        size_t shm_size = sizeof(struct func_stack) + sizeof(size_t);
        m_func_stack = (struct func_stack*)shm_alloc(m_shm_mgr, shm_key, (unsigned int)shm_size);
        if (!m_func_stack)
        {
            return false;
        }

        m_func_stack->m_top = 0;
        m_cur_func_perf_info = 0;

        return true;
    }

    void UnInit(void)
    {
        if (m_shm_mgr)
        {
            if (m_shm_key)
            {
                shm_free(m_shm_mgr, m_shm_key);
                m_shm_key = 0;
                m_func_stack = 0;
            }

            destroy_shm_mgr(m_shm_mgr);
            m_shm_mgr = 0;
        }

        m_func_list = 0;
    }

protected:
    CFuncPerformanceInfo*	m_func_list;
    HSHMMGR					m_shm_mgr;
    int						m_shm_key;
    struct func_stack*		m_func_stack;
    CFuncPerformanceInfo*	m_cur_func_perf_info;
    char					m_stack_info[1024 * 512];
private:
};

CFuncPerformanceInfo::CFuncPerformanceInfo(const char* func_name, HFUNCPERFMGR mgr)
    :func_name(func_name)
    , elapse_cycles(0)
    , hit_count(0)
{
    mgr->RecordFunc(this);
}

CFuncPerformanceCheck::CFuncPerformanceCheck(CFuncPerformanceInfo* info, HFUNCPERFMGR mgr)
{
    m_mgr = mgr;
    m_func_perf_info = info;
    m_parent_func_perf_info = mgr->m_cur_func_perf_info;
    mgr->m_cur_func_perf_info = info;
    mgr->m_func_stack->Push(info);
    m_cycles = __rdtsc();
}

CFuncPerformanceCheck::~CFuncPerformanceCheck(void)
{
    m_cycles = __rdtsc() - m_cycles;
    m_mgr->m_func_stack->Pop();
    m_func_perf_info->elapse_cycles += m_cycles;
    if (m_parent_func_perf_info)
    {
        m_parent_func_perf_info->elapse_cycles -= m_cycles;
    }
    m_mgr->m_cur_func_perf_info = m_parent_func_perf_info;
}

HFUNCPERFMGR CreateFuncPerfMgr(int shm_key)
{
    HFUNCPERFMGR mgr = new CFuncPerformanceMgr;

    if (mgr->Init(shm_key))
    {
        return mgr;
    }
    else
    {
        mgr->UnInit();
        delete mgr;

        return 0;
    }
}

void DestroyFuncPerfMgr(HFUNCPERFMGR mgr)
{
    mgr->UnInit();
    delete mgr;
}

const char* GetFuncStackInfo(HFUNCPERFMGR mgr)
{
    return mgr->UpdateStackInfo();
}

CFuncPerformanceInfo* FuncPerfFirst(HFUNCPERFMGR mgr)
{
    return mgr->FuncPerfFirst();
}

void FuncStackToFile(HFUNCPERFMGR mgr, const char* path)
{
    HFILELOG log = create_file_log(path, "call_stack");

    file_log_write(log, log_sys, "%s", mgr->UpdateStackInfo());

    destroy_file_log(log);
}

__declspec(thread) CFuncPerformanceMgr* def_func_perf_mgr = 0;

HFUNCPERFMGR DefFuncPerfMgr(void)
{
    return def_func_perf_mgr;
}

int GetFuncStackTop(HFUNCPERFMGR mgr)
{
    return mgr->StackTop();
}

CFuncPerformanceInfo* GetStackFuncPerfInfo(HFUNCPERFMGR mgr, int idx)
{
    return mgr->StackFuncPerfInfo(idx);
}

