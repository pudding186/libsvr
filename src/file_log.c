
#include "../include/file_log.h"
#include "../include/memory_pool.h"
#include "../include/loop_cache.h"
#include "../include/timer.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

#define MAX_LOGFILE_SIZE    (1024*1024*1024)

#define LOG_CMD_WRITE   1
#define LOG_CMD_FLUSH   2

#ifdef WIN32
#define PATH_MAX MAX_PATH
#include <windows.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif


typedef struct st_log_cmd 
{
    int     cmd;
    enum    log_level lv;
    char    data_org[512];
    char*   data_ext;
    int     data_len;
}log_cmd;

typedef struct st_log_unit
{
    struct st_log_unit* next;
    HLOOPCACHE          cmd_que;
    HLOOPCACHE          rcy_que;
    HMEMORYUNIT         log_cmd_unit;
    HMEMORYMANAGER      log_mem_pool_mgr;
}log_unit;

typedef struct st_log_file
{
    char        log_dir[PATH_MAX];
    char        log_name[PATH_MAX];
    struct tm   log_time;
    FILE*       log_file;
    size_t      log_size;
    int         log_flag;
    HANDLE      log_thread;
    log_unit*   log_unit_head;
    int         m_is_run;
}log_file;

extern HLOOPCACHE create_loop_cache_ex(size_t size, char* data);
extern void destroy_loop_cache_ex(HLOOPCACHE cache);

__declspec(thread) static log_unit* log_unit_thread = 0;

log_unit* _get_log_unit(log_file* log)
{
    if (!log_unit_thread)
    {
        HMEMORYMANAGER mem_pool_mgr = create_memory_manager(8, 128, 65536, 4 * 1024, 2);

        log_unit_thread = (log_unit*)memory_manager_alloc(mem_pool_mgr, sizeof(log_unit));
        log_unit_thread->cmd_que = create_loop_cache_ex(sizeof(log_unit*) * 1024, memory_manager_alloc(mem_pool_mgr, sizeof(log_unit*) * 1024));
        log_unit_thread->rcy_que = create_loop_cache_ex(sizeof(log_unit*) * 1024, memory_manager_alloc(mem_pool_mgr, sizeof(log_unit*) * 1024));
        log_unit_thread->log_cmd_unit = create_memory_unit(sizeof(log_cmd));
        log_unit_thread->log_mem_pool_mgr = mem_pool_mgr;

        log_unit_thread->next = log->log_unit_head;
        log->log_unit_head = log_unit_thread;

    }

    return log_unit_thread;
}

extern void _process_log_unit(log_file* log);

static unsigned int _stdcall _log_thread_func(void* arg)
{
    log_file* log = (log_file*)arg;

    while (log->m_is_run)
    {
        _process_log_unit(log);
    }

    _process_log_unit(log);

    return 0;
}

bool _mk_dir(const char* dir)
{
    size_t i;
    char* p1;
    char* p;
    char path[PATH_MAX] = { 0 };
    strncpy(path, dir, PATH_MAX);
    p1 = path + 1;

    p = path;

    for (i = 0; i < strlen(path); i++)
    {
        if (*(p + i) == '\\')
        {
            *(p + i) = '/';
        }
    }

    do
    {
        p1 = strchr(p1, '/');
        if (p1 != NULL)
        {
            *p1 = '\0';
        }
#ifdef _WIN32
        if (-1 == _mkdir(path))
#else
        if (-1 == mkdir(path, 0755))
#endif

        {
            if ((NULL == p1) && (errno != EEXIST))
            {
                return false;
            }
        }
        if (p1 != NULL)
        {
            *p1++ = '/';
        }

    } while (p1 != NULL);

    return true;
}

void _check_log(log_file* log)
{
    int index = 0;
    char file_full_path[PATH_MAX];
    struct tm st_cur_time;
    time_t cur_time = get_time();
    st_cur_time = *localtime(&cur_time);

    if (!log->log_file)
    {
        snprintf(file_full_path, sizeof(file_full_path), "%s/%s_%04d-%02d-%02d.txt",
            log->log_dir, log->log_name, st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1,
            st_cur_time.tm_mday);

        log->log_file = fopen(file_full_path, "a");
        log->log_size = ftell(log->log_file);
    }

    if (log->log_file)
    {
        while (log->log_size >= MAX_LOGFILE_SIZE)
        {
            ++index;
            snprintf(file_full_path, sizeof(file_full_path), "%s/%s_%04d-%02d-%02d-%d.txt",
                log->log_dir, log->log_name, st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1,
                st_cur_time.tm_mday, index);

            fclose(log->log_file);
            log->log_file = fopen(file_full_path, "a");
            log->log_size = ftell(log->log_file);
        }
    }

    if (log->log_time.tm_year != st_cur_time.tm_year
        || log->log_time.tm_mon != st_cur_time.tm_mon
        || log->log_time.tm_mday != st_cur_time.tm_mday)
    {
        snprintf(file_full_path, sizeof(file_full_path), "%s/%s_%04d-%02d-%02d.txt",
            log->log_dir, log->log_name, st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1,
            st_cur_time.tm_mday);

        if (log->log_file)
        {
            fclose(log->log_file);
        }

        log->log_file = fopen(file_full_path, "a");
        log->log_size = ftell(log->log_file);
    }

    log->log_time = st_cur_time;
}

const char* log_lv_to_str(enum log_level lv)
{
    switch (lv)
    {
    case log_dbg:
        return "[DBG]";
    	break;
    case log_inf:
        return "[INF]";
        break;
    case log_wrn:
        return "[WRN]";
        break;
    case log_cri:
        return "[CRI]";
        break;
    case log_sys:
        return "[SYS]";
        break;
    default:
        return "[UNKNOW]";
    }
}

void _begin_console(enum log_level lv)
{
#ifdef WIN32
    switch (lv)
    {
    case log_sys:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
        break;
    case log_cri:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
        break;
    case log_wrn:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
        break;
    case log_inf:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
        break;
    case log_dbg:
        break;
    }
#else
    switch (log_level)
    {
    case log_sys:
        printf("\033[32m");
        break;
    case log_cri:
        printf("\033[31m");
        break;
    case log_wrn:
        printf("\033[33m");
        break;
    case log_inf:
        printf("\033[32m");
        break;
    case log_dbg:
        break;
    }
#endif
}

void _end_console(void)
{
#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    printf("\033[0m");
#endif
}

void _process_log_unit(log_file* log)
{
    log_cmd* cmd;
    log_unit* unit = log->log_unit_head;
    bool is_busy = false;

    for (;;)
    {
        if (!unit)
        {
            if (is_busy)
            {
                unit = log->log_unit_head;
                is_busy = false;
            }
            else
            {
                Sleep(10);
                break;
            }
        }

        cmd = 0;

        if (loop_cache_pop_data(unit->cmd_que, (char*)&cmd, sizeof(log_cmd*)))
        {
            is_busy = true;

            if (LOG_CMD_WRITE == cmd->cmd)
            {
                int write_size;
                char* data;

                _check_log(log);

                data = cmd->data_org;
                if (cmd->data_ext)
                {
                    data = cmd->data_ext;
                }

                write_size = fprintf(log->log_file, "%04d-%02d-%02d %02d:%02d:%02d %s %s\r\n",
                    log->log_time.tm_year + 1900, log->log_time.tm_mon + 1, log->log_time.tm_mday,
                    log->log_time.tm_hour, log->log_time.tm_min, log->log_time.tm_sec,
                    log_lv_to_str(cmd->lv), data);

                _begin_console(cmd->lv);
                printf("%04d-%02d-%02d %02d:%02d:%02d %s %s\r\n", log->log_time.tm_year + 1900, log->log_time.tm_mon + 1, log->log_time.tm_mday,
                    log->log_time.tm_hour, log->log_time.tm_min, log->log_time.tm_sec,
                    log_lv_to_str(cmd->lv), data);
                _end_console();

                if (write_size > 0)
                {
                    log->log_size += write_size;
                }
            }
            else if (LOG_CMD_FLUSH == cmd->cmd)
            {
                fflush(log->log_file);
            }

            while (!loop_cache_push_data(unit->rcy_que, (char*)&cmd, sizeof(log_cmd*)))
            {
                Sleep(10);
            }
        }


        unit = unit->next;
    }
}

HFILELOG(create_file_log)(const char* path, const char* name)
{
    unsigned thread_id = 0;
    log_file* log = (log_file*)malloc(sizeof(log_file));
    
    if (!path)
    {
        return 0;
    }

    if (!name)
    {
        name = "log";
    }

    if (!_mk_dir(path))
    {
        free(log);
        return 0;
    }

    strncpy(log->log_dir, path, sizeof(log->log_dir));
    strncpy(log->log_name, name, sizeof(log->log_name));

    log->log_file = 0;
    log->log_size = 0;
    log->log_flag = log_none;

    _check_log(log);

    if (!log->log_file)
    {
        free(log);
        return 0;
    }
    log->log_unit_head = 0;
    log->m_is_run = true;
    log->log_thread = (HANDLE)_beginthreadex(NULL,
        0,
        _log_thread_func,
        log,
        0,
        &thread_id);

    if (!log->log_thread)
    {
        fclose(log->log_file);
        free(log);
        return 0;
    }

    return log;
}

void (destroy_file_log)(HFILELOG log)
{
    log_unit* unit;
    log->m_is_run = false;
    WaitForSingleObject(log->log_thread, INFINITE);

    if (log->log_file)
    {
        fflush(log->log_file);
        fclose(log->log_file);
    }

    unit = log->log_unit_head;

    while (unit)
    {
        log_unit* next_unit = unit->next;

        destroy_memory_unit(unit->log_cmd_unit);
        destroy_loop_cache_ex(unit->cmd_que);
        destroy_loop_cache_ex(unit->rcy_que);
        destroy_memory_manager(unit->log_mem_pool_mgr);

        unit = next_unit;
    }

    free(log);
}

//bool (file_log_write_now)(HFILELOG log, enum log_level lv, const char* format, ...)
//{
//    va_list args;
//
//    struct tm st_cur_time;
//    time_t cur_time;
//
//    if (!(log->log_flag & lv))
//    {
//        return false;
//    }
//
//    cur_time = get_time();
//    st_cur_time = *localtime(&cur_time);
//
//    fprintf(log->log_file, "%04d-%02d-%02d %02d:%02d:%02d %s ",
//        st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1, st_cur_time.tm_mday,
//        st_cur_time.tm_hour, st_cur_time.tm_min, st_cur_time.tm_sec,
//        log_lv_to_str(lv));
//
//    va_start(args, format);
//    vfprintf(log->log_file, format, args);
//    va_end(args);
//
//    fprintf(log->log_file, "\r\n");
//
//    _begin_console(lv);
//    printf("%04d-%02d-%02d %02d:%02d:%02d %s ",
//        st_cur_time.tm_year + 1900, st_cur_time.tm_mon + 1, st_cur_time.tm_mday,
//        st_cur_time.tm_hour, st_cur_time.tm_min, st_cur_time.tm_sec,
//        log_lv_to_str(lv));
//    va_start(args, format);
//    vprintf(format, args);
//    va_end(args);
//    _end_console(lv);
//    
//    return true;
//}

bool (file_log_write)(HFILELOG log, enum log_level lv, const char* format, ...)
{
    log_unit* unit = _get_log_unit(log);

    log_cmd* cmd = 0;

    va_list args;

    if (!(log->log_flag & lv))
    {
        return false;
    }

    if (loop_cache_pop_data(unit->rcy_que, (char*)&cmd, sizeof(log_cmd*)))
    {
        if (cmd->data_ext)
        {
            memory_manager_free(unit->log_mem_pool_mgr, cmd->data_ext);
            cmd->data_ext = 0;
        }
    }
    else
    {
        cmd = memory_unit_alloc(unit->log_cmd_unit, 4096);
        cmd->data_ext = 0;
    }

    cmd->data_len = 0;

    cmd->lv = lv;
    cmd->cmd = LOG_CMD_WRITE;

    va_start(args, format);
    cmd->data_len = vsnprintf(cmd->data_org, sizeof(cmd->data_org), format, args);
    va_end(args);

    if (cmd->data_len < 0)
    {
        memory_unit_free(unit->log_cmd_unit, cmd);
        return false;
    }
    else if (cmd->data_len >= sizeof(cmd->data_org))
    {
        va_list args_ex;

        int new_data_len = cmd->data_len + 1;
        cmd->data_ext = memory_manager_alloc(unit->log_mem_pool_mgr, new_data_len);

        
        va_start(args_ex, format);
        cmd->data_len = vsnprintf(cmd->data_ext, new_data_len, format, args_ex);
        va_end(args_ex);

        if (cmd->data_len != (new_data_len-1))
        {
            memory_manager_free(unit->log_mem_pool_mgr, cmd->data_ext);
            memory_unit_free(unit->log_cmd_unit, cmd);
            return false;
        }
    }

    if (loop_cache_push_data(unit->cmd_que, (char*)&cmd, sizeof(log_cmd*)))
    {
        return true;
    }
    else
    {
        if (cmd->data_ext)
        {
            memory_manager_free(unit->log_mem_pool_mgr, cmd->data_ext);
        }
        memory_unit_free(unit->log_cmd_unit, cmd);
        return false;
    }
}

bool (file_log_flush)(HFILELOG log)
{
    log_unit* unit = _get_log_unit(log);

    log_cmd* cmd = 0;

    if (loop_cache_pop_data(unit->rcy_que, (char*)&cmd, sizeof(log_cmd*)))
    {
        if (cmd->data_ext)
        {
            memory_manager_free(unit->log_mem_pool_mgr, cmd->data_ext);
            cmd->data_ext = 0;
        }
    }
    else
    {
        cmd = memory_unit_alloc(unit->log_cmd_unit, 4096);
        cmd->data_ext = 0;
    }

    cmd->data_len = 0;

    cmd->cmd = LOG_CMD_FLUSH;

    if (loop_cache_push_data(unit->cmd_que, (char*)&cmd, sizeof(log_cmd*)))
    {
        return true;
    }
    else
    {
        memory_unit_free(unit->log_cmd_unit, cmd);
        return false;
    }
}

void file_log_option(HFILELOG log, enum log_level lv, bool open_or_not)
{
    if (open_or_not)
    {
        log->log_flag = log->log_flag | lv;
    }
    else
    {
        log->log_flag &= ~lv;
    }
}

bool file_log_open_or_not(HFILELOG log, enum log_level lv)
{
    return ((log->log_flag & lv) != 0);
}
