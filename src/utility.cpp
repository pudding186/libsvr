#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/smemory.hpp"
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

//const char* memnmem(const char* mem1, int len1, const char* mem2, int len2)
//{
//    int n;
//
//    if (!len2)
//    {
//        return 0;
//    }
//
//    if (len2 > len1)
//    {
//        return 0;
//    }
//
//    while (len1)
//    {
//        for (n = 0; *(mem1 + n) == *(mem2 + n); n++)
//        {
//            if ((n + 1) == len2)
//            {
//                return mem1;
//            }
//        }
//
//        mem1++;
//        len1--;
//    }
//
//    return 0;
//}

//data_fragment_array split_data_to_fragment(const char* data, int len, const char* spliter, int spliter_len)
//{
//    const char* pStart = data;
//    const char* pPos;
//    const char* end = data + len;
//
//    int alloc_frg_size = 256;
//    data_fragment_array frag_array;
//
//    frag_array.count = 0;
//    frag_array.fragments = (data_fragment*)default_memory_manager_alloc(sizeof(data_fragment)*alloc_frg_size);
//
//
//    pPos = memnmem(pStart, len, spliter, spliter_len);
//
//    while (pPos)
//    {
//        if (pPos >= pStart)
//        {
//            frag_array.fragments[frag_array.count].str = pStart;
//            frag_array.fragments[frag_array.count].length = (int)(pPos - pStart);
//            frag_array.count++;
//
//            if (frag_array.count > alloc_frg_size)
//            {
//                alloc_frg_size += 256;
//                frag_array.fragments = (data_fragment*)default_memory_manager_realloc(frag_array.fragments, sizeof(data_fragment)*alloc_frg_size);
//            }
//        }
//
//        pStart = pPos + spliter_len;
//        pPos = memnmem(pStart, (int)(end - pStart), spliter, spliter_len);
//    }
//
//    if (pStart < end)
//    {
//        frag_array.fragments[frag_array.count].str = pStart;
//        frag_array.fragments[frag_array.count].length = (int)(pPos - pStart);
//        frag_array.count++;
//
//        if (frag_array.count > alloc_frg_size)
//        {
//            alloc_frg_size += 256;
//            frag_array.fragments = (data_fragment*)default_memory_manager_realloc(frag_array.fragments, sizeof(data_fragment)*alloc_frg_size);
//        }
//    }
//
//    return frag_array;
//}

//data_fragment_array split_str_to_fragment(const char* data, int len, const char* spliter, int spliter_len)
//{
//    char* new_str = (char*)default_memory_manager_alloc(len + 1);
//    memcpy(new_str, data, len);
//    new_str[len] = 0;
//
//    char* pStart = new_str;
//    char* pPos;
//    char* end = new_str + len;
//
//    int alloc_frg_size = 256;
//    data_fragment_array frag_array;
//
//    frag_array.count = 0;
//    frag_array.fragments = (data_fragment*)default_memory_manager_alloc(sizeof(data_fragment)*alloc_frg_size);
//
//    pPos = (char*)memnmem(pStart, len, spliter, spliter_len);
//
//    while (pPos)
//    {
//        if (pPos >= pStart)
//        {
//            frag_array.fragments[frag_array.count].str = pStart;
//            frag_array.fragments[frag_array.count].length = (int)(pPos - pStart);
//            frag_array.count++;
//
//            if (frag_array.count > alloc_frg_size)
//            {
//                alloc_frg_size += 256;
//                frag_array.fragments = (data_fragment*)default_memory_manager_realloc(frag_array.fragments, sizeof(data_fragment)*alloc_frg_size);
//            }
//        }
//
//        *pPos = 0;
//
//        pStart = pPos + spliter_len;
//        pPos = (char*)memnmem(pStart, (int)(end - pStart), spliter, spliter_len);
//    }
//
//    if (pStart < end)
//    {
//        frag_array.fragments[frag_array.count].str = pStart;
//        frag_array.fragments[frag_array.count].length = (int)(pPos - pStart);
//        frag_array.count++;
//
//        if (frag_array.count > alloc_frg_size)
//        {
//            alloc_frg_size += 256;
//            frag_array.fragments = (data_fragment*)default_memory_manager_realloc(frag_array.fragments, sizeof(data_fragment)*alloc_frg_size);
//        }
//    }
//
//    return frag_array;
//}
//
//void free_data_fragment_array(data_fragment_array frag_array)
//{
//    if (frag_array.fragments)
//    {
//        default_memory_manager_free(frag_array.fragments);
//    }
//}

str_fragment_array* split_str_to_fragment(const char* str, int str_len, const char* spliter, int spliter_len)
{
    int alloc_str_fragment_count = 256;

    char* ptr = (char*)default_memory_manager_alloc(sizeof(str_fragment_array) + str_len + 1);
    str_fragment_array* frag_array = (str_fragment_array*)ptr;
    frag_array->copy_string = (char*)(ptr + sizeof(str_fragment_array));
    memcpy(frag_array->copy_string, str, str_len);
    frag_array->copy_string[str_len] = 0;

    frag_array->fragments_count = 0;
    frag_array->fragments = (str_fragment*)default_memory_manager_alloc(
        sizeof(str_fragment)*alloc_str_fragment_count);

    char* pStart = frag_array->copy_string;
    char* pPos;
    char* end = frag_array->copy_string + str_len;

    pPos = strstr(pStart, spliter);

    while (pPos)
    {
        if (pPos > pStart)
        {
            frag_array->fragments[frag_array->fragments_count].frag_str = pStart;
            frag_array->fragments[frag_array->fragments_count].frag_str_len = (int)(pPos - pStart);
            frag_array->fragments_count++;
        }
        else if (pPos == pStart)
        {
            frag_array->fragments[frag_array->fragments_count].frag_str = 0;
            frag_array->fragments[frag_array->fragments_count].frag_str_len = 0;
            frag_array->fragments_count++;
        }

        if (frag_array->fragments_count >= alloc_str_fragment_count)
        {
            alloc_str_fragment_count += 256;
            frag_array->fragments = (str_fragment*)default_memory_manager_realloc(
                frag_array->fragments, sizeof(str_fragment)*alloc_str_fragment_count);
        }

        *pPos = 0;
        pStart = pPos + spliter_len;
        pPos = strstr(pStart, spliter);
    }

    if (pStart < end)
    {
        frag_array->fragments[frag_array->fragments_count].frag_str = pStart;
        frag_array->fragments[frag_array->fragments_count].frag_str_len = (int)(end - pStart);
        frag_array->fragments_count++;
    }

    return frag_array;
}

void free_str_fragment_array(str_fragment_array* array)
{
    if (array->fragments)
    {
        default_memory_manager_free(array->fragments);
        array->fragments = 0;
    }

    default_memory_manager_free(array);
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

CFuncPerformanceInfo* FuncPerfFirst(HFUNCPERFMGR mgr)
{
    return mgr->FuncPerfFirst();
}

extern "C"
{
    extern bool _mk_dir(const char* dir);
}

void FuncStackToFile(HFUNCPERFMGR mgr, const char* path)
{
    struct tm st_cur_time;
    time_t cur_time = time(0);
    st_cur_time = *localtime(&cur_time);

    if (_mk_dir(path))
    {
        char stack_file_full_path[512];
        snprintf(stack_file_full_path, sizeof(stack_file_full_path),
            "%s/stack_%04d_%02d_%02d_%02d_%02d_%02d.log", path,
            st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1, st_cur_time.tm_mday,
            st_cur_time.tm_hour, st_cur_time.tm_min, st_cur_time.tm_sec);

        FILE* stack_file = fopen(stack_file_full_path, "a");

        if (stack_file)
        {
            fprintf(stack_file, "****** call stack ******\r\n");
            int stack_idx = mgr->StackTop();

            while (stack_idx > 0)
            {
                CFuncPerformanceInfo* info = mgr->StackFuncPerfInfo(stack_idx - 1);
                if (info)
                {
                    fprintf(stack_file, "[stack:%2d] call %s()\r\n", stack_idx, info->func_name);
                }
                else
                {
                    fprintf(stack_file, "[stack:%2d] call ?()\r\n", stack_idx);
                }

                --stack_idx;
            }

            fclose(stack_file);
        }
    }
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

