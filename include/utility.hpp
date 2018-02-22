#pragma once

template <typename T>
inline void StrSafeCopy(T& dst, const char* src) throw()
{
    (static_cast<char[sizeof(dst)]>(dst));

    if (!src)
    {
        dst[0] = '\0';
        return;
    }

    if (!lstrcpyn(dst, src, sizeof(dst)))
    {
        dst[sizeof(dst) - 1] = '\0';
    }
}

//////////////////////////////////////////////////////////////////////////
#define DECLARE_SINGLETON(cls)\
private:\
    static cls* sm_poInstance;\
public:\
    static bool CreateInstance()\
    {\
        if(NULL == sm_poInstance)sm_poInstance = new cls;\
        return sm_poInstance != NULL;\
    }\
    static bool ReplaceInstance(cls* instance)\
    {\
        if (sm_poInstance)\
        {\
            delete sm_poInstance;\
            sm_poInstance = instance;\
        }\
        return sm_poInstance != NULL;\
    }\
    static cls* Instance(){ return sm_poInstance; }\
    static void DestoryInstance()\
    {\
        if(sm_poInstance != NULL)\
        {\
            delete sm_poInstance;\
            sm_poInstance = NULL;\
        }\
    }

#define INSTANCE_SINGLETON(cls) \
    cls* cls::sm_poInstance = NULL;

//////////////////////////////////////////////////////////////////////////

extern int RandInt(int min, int max);
extern long long RandInt64(long long min, long long max);

extern unsigned int BKDRHash(const char* str);
extern unsigned long long BKDRHash64(const char* str);

//////////////////////////////////////////////////////////////////////////

typedef class CFuncPerformanceMgr* HFUNCPERFMGR;

class CFuncPerformanceInfo
{
    friend class CFuncPerformanceMgr;
protected:
    CFuncPerformanceInfo* next;
public:
    const char* func_name;
    unsigned long long elapse_cycles;
    unsigned long long hit_count;
    CFuncPerformanceInfo(const char* func_name, HFUNCPERFMGR mgr);
    inline CFuncPerformanceInfo* NextInfo(void)
    {
        return next;
    }
};

class CFuncPerformanceCheck
{
public:
    CFuncPerformanceCheck(CFuncPerformanceInfo* info, HFUNCPERFMGR mgr);
    ~CFuncPerformanceCheck(void);
protected:
    unsigned long long m_cycles;
    CFuncPerformanceInfo* m_parent_func_perf_info;
    CFuncPerformanceInfo* m_func_perf_info;
    HFUNCPERFMGR	m_mgr;
private:
};

extern HFUNCPERFMGR(CreateFuncPerfMgr)(int shm_key);
extern void(DestroyFuncPerfMgr)(HFUNCPERFMGR mgr);
extern const char* (GetFuncStackInfo)(HFUNCPERFMGR mgr);
extern CFuncPerformanceInfo* (FuncPerfFirst)(HFUNCPERFMGR mgr);

extern void (FuncStackToFile)(HFUNCPERFMGR mgr, const char* file_path);

#define FUNC_PERFORMANCE_CHECK(FUNC_PERF_MGR) \
	__declspec(thread) static CFuncPerformanceInfo s_func_perf_info(__FUNCTION__, FUNC_PERF_MGR);\
	++ s_func_perf_info.hit_count;\
	CFuncPerformanceCheck func_perf_check(&s_func_perf_info, FUNC_PERF_MGR);