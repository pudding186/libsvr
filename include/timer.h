#pragma once
#include <time.h>
#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_timer_manager* HTIMERMANAGER;
typedef struct st_timer_info* HTIMERINFO;
typedef void (*pfn_on_timer)(HTIMERINFO timer);

extern HTIMERMANAGER (create_timer_manager)(pfn_on_timer func_on_timer);

extern void (destroy_timer_manager)(HTIMERMANAGER mgr);

extern HTIMERINFO (timer_add)(HTIMERMANAGER mgr, unsigned elapse, int count, void* data);

extern void (timer_mod)(HTIMERINFO timer, unsigned elapse, int count, void* data);

extern void (timer_del)(HTIMERINFO timer);

extern bool (timer_update)(HTIMERMANAGER mgr, unsigned elapse);

extern void* (timer_get_data)(HTIMERINFO timer);

extern int (timer_remain_count)(HTIMERINFO timer);

//////////////////////////////////////////////////////////////////////////
extern unsigned int (get_tick)(void);
extern time_t (get_time)(void);

//////////////////////////////////////////////////////////////////////////
// yyyy-mm-dd hh:mm:ss
extern bool (time_to_string)(time_t time, char* str, size_t len);

extern time_t (string_to_time)(const char* time_string);


//返回从开机到现在的毫秒数
#define TIME_ZONE    g_time_zone

//返回从1970年1月1日0时0分0到现在经过的秒数(UTC 时间)
#define NOW_TIME    g_local_time

//返回从1970年1月1日0时0分0到现在经过的小时数(UTC 时间)
extern time_t now_hour(void);

//从1970年1月1日0时0分0到现在经过的天数(考虑本地时区)
extern time_t now_day(void);

//从UTC 1970年1月1日0时0分0到现在经过的星期数(考虑本地时区)
extern time_t now_week(void);

//从UTC 1970年1月1日0时0分0到现在经过的月数
extern unsigned now_month(void);

//从UTC 1970年1月1日0时0分0到现在经过的年数
extern unsigned now_year(void);

//获取本周指定星期日子到1970年1月1日8时0分0经过的秒数(UTC时间),取值区间[1, 7]其中7代表星期天，1代表星期一
extern time_t week_day_to_time(time_t week_day);

//将从1970年1月1日0时0分0经过的秒数(UTC时间)转换星期,取值区间[1, 7]其中7代表星期天，1代表星期一
extern time_t time_to_week_day(time_t tm);

// 获取第N天的开始时的time
extern time_t day_begin_time(time_t day);

// 获取第N周的开始时的time  按中国习惯，这里用星期一00:00:00(当地时间)作为每周的开始
extern time_t week_begin_time(time_t week);

// 获取第N月的开始时的time  这里用每月1号00:00:00(当地时间)作为每月的开始
extern time_t month_begin_time(time_t month);

// 获取第N年的开始时的time  这里用每年1月1号00:00:00(当地时间)作为每月的开始
extern time_t year_begin_time(time_t year);


#ifdef  __cplusplus
}
#endif