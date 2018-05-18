#pragma once
#include <string>

//////////////////////////////////////////////////////////////////////////

extern int rand_int(int min, int max);
extern long long rand_int64(long long min, long long max);

extern unsigned int BKDRHash(const char* str);
extern unsigned long long BKDRHash64(const char* str);

typedef struct st_str_fragment
{
    char*       frag_str;
    int         frag_str_len;
}str_fragment;

typedef struct st_str_fragment_array
{
    char*           copy_string;
    str_fragment*   fragments;
    int             fragments_count;
}str_fragment_array;

extern str_fragment_array* (split_str_to_fragment)(const char* str, int str_len, const char* spliter, int spliter_len);
extern void (free_str_fragment_array)(str_fragment_array* array);

#ifdef __cplusplus

template <size_t N>
inline void StrSafeCopy(char(&Destination)[N], const char* Source) throw() {
	static_assert(N > 0, "StrSafeCopy dst size == 0");

	// initialize for check below
	if (NULL == Source) {
		Destination[0] = '\0';
		return;
	}

	size_t nSrcLen = strnlen(Source, N - 1);
	memcpy(Destination, Source, nSrcLen + 1);
	Destination[N - 1] = '\0';
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

#define DEFMEMBER(type,member,name)								\
    public:														\
    inline void set_##name(type name){member=name;}					\
    inline type get_##name() const {return member;}					\
protected:														\
    type member;												\
public:

#define DEFREFMEMBER(type,member,name)							\
	public:														\
	inline void set_##name(const type& name){member=name;}			\
	inline const type& get_##name() const {return member;}			\
protected:														\
	type member;												\
public:

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

extern HFUNCPERFMGR (CreateFuncPerfMgr)(int shm_key);
extern void (DestroyFuncPerfMgr)(HFUNCPERFMGR mgr);
extern CFuncPerformanceInfo* (FuncPerfFirst)(HFUNCPERFMGR mgr);
extern int (GetFuncStackTop)(HFUNCPERFMGR mgr);
extern CFuncPerformanceInfo* (GetStackFuncPerfInfo)(HFUNCPERFMGR mgr, int idx);

extern HFUNCPERFMGR (DefFuncPerfMgr)(void);

extern void (FuncStackToFile)(HFUNCPERFMGR mgr, const char* file_path);

#define FUNC_PERFORMANCE_CHECK \
	__declspec(thread) static CFuncPerformanceInfo s_func_perf_info(__FUNCTION__, DefFuncPerfMgr());\
	++ s_func_perf_info.hit_count;\
	CFuncPerformanceCheck func_perf_check(&s_func_perf_info, DefFuncPerfMgr());

#endif
