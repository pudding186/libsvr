#include <stdlib.h>
#include <stddef.h>
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include "../include/type_def.h"
#include "../include/timer.h"
#include "../include/memory_pool.h"

#pragma comment(lib, "winmm.lib")

bool                    g_local_time_thread_run = false;
bool                    g_local_time_run = false;
volatile unsigned int   g_local_tick = 0;
volatile time_t         g_local_time = 0;
HANDLE                  g_local_time_thread = 0;
long                    g_time_zone = 0;

unsigned _stdcall local_time_proc(void* param)
{
    unsigned last_tick = 0;

    bool* local_time_thread_run = (bool*)param;

    while (g_local_time_run)
    {
        g_local_tick = timeGetTime();

        if (g_local_tick - last_tick > 1000)
        {
            last_tick = g_local_tick;
            g_local_time = time(0);
        }

        timeBeginPeriod(1);
        Sleep(1);
        *local_time_thread_run = true;
        timeEndPeriod(1);
    }

    *local_time_thread_run = false;

    return 0;
}

bool init_local_time(void)
{
    unsigned thread_id = 0;

    if (g_local_time_thread)
    {
        return true;
    }

    g_local_tick = timeGetTime();
    g_local_time = time(0);
    g_local_time_run = true;
    _tzset();
    _get_timezone(&g_time_zone);
    g_local_time_thread = (HANDLE)_beginthreadex(0, 0, local_time_proc, &g_local_time_thread_run, 0, &thread_id);

    if (!g_local_time_thread)
    {
        return false;
    }

    Sleep(10);

    return true;
}

void uninit_local_time(void)
{
    if (g_local_time_thread)
    {
        g_local_time_run = false;

        WaitForSingleObject(g_local_time_thread, INFINITE);

        if (g_local_time_thread)
        {
            CloseHandle(g_local_time_thread);
            g_local_time_thread = 0;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define INDEX(N) ((mgr->last_tick >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)



struct list_head {
    struct list_head *next, *prev;
};

typedef struct st_timer_info
{
    struct list_head            entry;
    unsigned                    expires;
    unsigned                    elapse;
    int                         count;
    void*                       data;
    struct st_timer_manager*    manager;
}timer_info;

typedef struct st_timer_manager 
{
    struct list_head    tv1[TVR_SIZE];
    struct list_head    tv2[TVN_SIZE];
    struct list_head    tv3[TVN_SIZE];
    struct list_head    tv4[TVN_SIZE];
    struct list_head    tv5[TVN_SIZE];

    unsigned            last_tick;
    pfn_on_timer        func_on_timer;
    HMEMORYUNIT         timer_info_unit;
}timer_manager;

static __inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static __inline void LIST_ADD_TAIL(struct list_head *newone, struct list_head *head)
{
    newone->prev = head->prev;
    head->prev->next = newone;
    newone->next = head;
    head->prev = newone;

}

static __inline void LIST_DEL(struct list_head* prev, struct list_head* next)
{
    next->prev = prev;
    prev->next = next;
}

static __inline void LIST_REPLACE_INIT(struct list_head* old, struct list_head* newone)
{
    newone->next = old->next;
    newone->next->prev = newone;
    newone->prev = old->prev;
    newone->prev->next = newone;

    old->prev = old;
    old->next = old;
}

static __inline int LIST_EMPTY(const struct list_head* head)
{
    return head->next == head;
}

static __inline struct st_timer_info* _get_info_by_entry(struct list_head* entry_ptr)
{
    return (struct st_timer_info*)((char*)entry_ptr - offsetof(struct st_timer_info, entry));
}

static __inline int _is_timer_pend(struct st_timer_info* info)
{
    return info->entry.next != 0;
}

static __inline void _detach_timer(struct st_timer_info* info)
{
    LIST_DEL(info->entry.prev, info->entry.next);
    info->entry.next = 0;
}

void _add_timer(struct st_timer_info* info)
{
    unsigned expires = info->expires;

    unsigned idx = expires - info->manager->last_tick;

    struct list_head* vec;

    int i;

	if (idx < TVR_SIZE)
	{
		i = expires & TVR_MASK;
		vec = info->manager->tv1 + i;
	}
	else if (idx < 1 << (TVR_BITS + TVN_BITS))
	{
		i = (expires >> TVR_BITS) & TVN_MASK;
		vec = info->manager->tv2 + i;
	}
	else if (idx < 1 <<(TVR_BITS + 2*TVN_BITS))
	{
		i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		vec = info->manager->tv3 + i;
	}
	else if (idx < 1 <<(TVR_BITS + 3*TVN_BITS))
	{
		i = (expires >> (TVR_BITS + 2*TVN_BITS)) & TVN_MASK;
		vec = info->manager->tv4 + i;
	}
	else if ((INT32)idx < 0)
	{
		/*  
		 * Can happen if you add a timer with expires == jiffies,  
		 * or you set a timer to go off in the past  
		 */
		vec = info->manager->tv1 + (info->manager->last_tick & TVR_MASK);
	}
	else
	{
		/* If the timeout is larger than 0xffffffff on 64-bit  
		 * architectures then we use the maximum timeout:  
		 */
		if (idx > 0xffffffffUL)
		{
			idx = 0xffffffffUL;
			expires = idx + info->manager->last_tick;
		}

		i = (expires >> (TVR_BITS + 3*TVN_BITS)) & TVN_MASK;

		vec = info->manager->tv5 + i;
	}

    LIST_ADD_TAIL(&info->entry, vec);
}

unsigned _cascade(struct list_head* vec, unsigned index)
{
    struct st_timer_info* info;
    struct st_timer_info* tmp_info;
    struct list_head tv_list;

    LIST_REPLACE_INIT(vec + index, &tv_list);

    for (info = _get_info_by_entry(tv_list.next), tmp_info = _get_info_by_entry(info->entry.next);
        &info->entry != (&tv_list); info = tmp_info, tmp_info = _get_info_by_entry(tmp_info->entry.next))
    {
        _add_timer(info);
    }

    return index;
}

HTIMERMANAGER create_timer_manager(pfn_on_timer func_on_timer)
{
    int i = 0;

    if (init_local_time())
    {
        struct st_timer_manager* mgr = (struct st_timer_manager*)malloc(sizeof(struct st_timer_manager));
        mgr->timer_info_unit = create_memory_unit(sizeof(struct st_timer_info));

        for (i = 0; i < TVN_SIZE; i++)
        {
            INIT_LIST_HEAD(mgr->tv5 + i);
            INIT_LIST_HEAD(mgr->tv4 + i);
            INIT_LIST_HEAD(mgr->tv3 + i);
            INIT_LIST_HEAD(mgr->tv2 + i);
        }

        for (i = 0; i < TVR_SIZE; i++)
        {
            INIT_LIST_HEAD(mgr->tv1 + i);
        }

        mgr->func_on_timer = func_on_timer;
        mgr->last_tick = get_tick();

        return mgr;
    }

    return 0;
}

void destroy_timer_manager(HTIMERMANAGER mgr)
{
    if (mgr->timer_info_unit)
    {
        destroy_memory_unit(mgr->timer_info_unit);
    }

    free(mgr);

    uninit_local_time();
}

HTIMERINFO timer_add(HTIMERMANAGER mgr, unsigned elapse, int count, void* data)
{
    struct st_timer_info* info = (struct st_timer_info*)memory_unit_alloc(mgr->timer_info_unit, 4*1024);

    info->expires = mgr->last_tick + elapse;
    info->elapse = elapse;
    info->count = count;
    info->data = data;
    info->manager = mgr;

    _add_timer(info);

    return info;
}

void timer_mod(HTIMERINFO timer, unsigned elapse, int count, void* data)
{
    if (_is_timer_pend(timer))
    {
        _detach_timer(timer);

        timer->expires = timer->manager->last_tick + elapse;
        timer->elapse = elapse;
        timer->count = count;

        if (data)
        {
            timer->data = data;
        }

        _add_timer(timer);
    }
    else
    {
        timer->expires = timer->manager->last_tick + elapse;
        timer->elapse = elapse;
        timer->count = count;

        if (data)
        {
            timer->data = data;
        }
    }
}

void timer_del(HTIMERINFO timer)
{
    if (_is_timer_pend(timer))
    {
        _detach_timer(timer);
        timer->data = 0;
        //memory_unit_free(_get_timer_info_unit(), timer);
        memory_unit_free(timer->manager->timer_info_unit, timer);
    }
    else
        timer->count = 0;
}

void timer_update(HTIMERMANAGER mgr, unsigned run_time)
{
    struct st_timer_info* info;

    unsigned int tick = get_tick();

    while ((int)(tick) - (int)(mgr->last_tick) >= 0)
    {
        struct list_head work_list;
        struct list_head* head = &work_list;

        unsigned index = mgr->last_tick & TVR_MASK;

        if (!index &&
            (!_cascade(mgr->tv2, INDEX(0))) &&
            (!_cascade(mgr->tv3, INDEX(1))) &&
            (!_cascade(mgr->tv4, INDEX(2))))
            _cascade(mgr->tv5, INDEX(3));

        LIST_REPLACE_INIT(mgr->tv1 + index, &work_list);

        while (!LIST_EMPTY(head))
        {
            info = _get_info_by_entry(head->next);

            _detach_timer(info);

            if (info->manager != mgr)
            {
                char* a = NULL;
                *a = 'a';
            }

            if (info->count)
            {
                if (run_time)
                {
                    unsigned int cur_tick = get_tick();
                    if (cur_tick - tick > run_time)
                    {
                        info->expires = cur_tick;
                        _add_timer(info);
                        continue;
                    }
                }

                if (info->count > 0)
                {
                    --info->count;
                }

                mgr->func_on_timer(info);

                if (0 == info->count)
                {
                    info->data = 0;
                    memory_unit_free(mgr->timer_info_unit, info);
                }
                else
                {
                    info->expires = mgr->last_tick + info->elapse;

                    _add_timer(info);
                }
            }
            else
            {
                info->data = 0;
                memory_unit_free(mgr->timer_info_unit, info);
            }
        }

        ++mgr->last_tick;

        if (run_time)
        {
            if (get_tick() - tick > run_time)
            {
                return;
            }
        }
    }
}

void* timer_get_data(HTIMERINFO timer)
{
    return timer->data;
}

int timer_remain_count(HTIMERINFO timer)
{
    return timer->count;
}

const char* time_to_string(time_t time)
{
    static char szBuf[128];
    struct tm* pTm = localtime(&time);
    if (pTm)
    {
        strftime(szBuf, 128, "%Y-%m-%d %H:%M:%S", pTm);
        return szBuf;
    }
    return 0;
}

time_t string_to_time(const char* time_string)
{
    struct tm t_time;

    if (6 == sscanf(time_string, "%d-%d-%d %d:%d:%d", 
        &t_time.tm_year, &t_time.tm_mon, &t_time.tm_mday, &t_time.tm_hour, &t_time.tm_min, &t_time.tm_sec))
    {
        t_time.tm_year -= 1900;
        t_time.tm_mon -= 1;
        return mktime(&t_time);
    }

    return 0;
}



unsigned int get_tick(void)
{
    if (g_local_time_thread_run)
    {
        return g_local_tick;
    }

    return timeGetTime();
}

time_t get_time(void)
{
    if (g_local_time_thread_run)
    {
        return g_local_time;
    }

    return time(0);
}

//返回从1970年1月1日0时0分0到现在经过的小时数(UTC 时间)
time_t now_hour(void)
{
    return g_local_time/3600;
}
//从1970年1月1日0时0分0到现在经过的天数(考虑本地时区)
time_t now_day(void)
{
    return (g_local_time - g_time_zone)/86400;
}
//从UTC 1970年1月1日0时0分0到现在经过的星期数(考虑本地时区)
time_t now_week(void)
{
    //1970年1月1日是星期四

    return (now_day()+3)/7;
}
//从UTC 1970年1月1日0时0分0到现在经过的月数
unsigned now_month(void)
{
    time_t tt = g_local_time;
    struct tm* pTM = localtime(&tt);

    return (pTM->tm_year-70)*12 + pTM->tm_mon+1;
}
//从UTC 1970年1月1日0时0分0到现在经过的年数
unsigned now_year(void)
{
    time_t tt = g_local_time;
    struct tm* pTM = localtime(&tt);

    return pTM->tm_year-70;
}

//获取本周指定星期日子到1970年1月1日8时0分0经过的秒数(UTC时间),取值区间[1, 7]其中7代表星期天，1代表星期一
time_t week_day_to_time(time_t week_day)
{
    return week_begin_time(now_week()) + (week_day - 1) * 86400;
}

//将从1970年1月1日0时0分0经过的秒数(UTC时间)转换星期,取值区间[1, 7]其中7代表星期天，1代表星期一
time_t time_to_week_day(time_t tm)
{
    int week_day = localtime(&tm)->tm_wday;

    return week_day ? week_day : 7;
}

// 获取第N天的开始时的time
time_t day_begin_time(time_t day)
{
    return day * 86400 + g_time_zone;
}

// 获取第N周的开始时的time  按中国习惯，这里用星期一00:00:00(当地时间)作为每周的开始
time_t week_begin_time(time_t week)
{
    return (week - 1) * 86400 * 7 + 4 * 86400 + g_time_zone;
}

// 获取第N月的开始时的time  这里用每月1号00:00:00(当地时间)作为每月的开始
time_t month_begin_time(time_t month)
{
    int m = (int)(month - 1) % 12;
    int y = (int)(month - 1) / 12;
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = 1;
    t.tm_mon = m;
    t.tm_year = y + 70;
    return mktime(&t);
}

// 获取第N年的开始时的time  这里用每年1月1号00:00:00(当地时间)作为每月的开始
time_t year_begin_time(time_t year)
{
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = 1;
    t.tm_mon = 0;
    t.tm_year = (int)year + 70;
    return mktime(&t);
}