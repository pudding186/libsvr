#pragma once
#include "./type_def.h"
#ifdef  __cplusplus
extern "C" {
#endif

    enum log_level
    {
        log_none = 0x0000,
        log_dbg = 0x0001,
        log_inf = (0x0001 << 1),
        log_wrn = (0x0001 << 2),
        log_cri = (0x0001 << 3),
        log_sys = (0x0001 << 4),
    };

    typedef struct st_log_file* HFILELOG;

    extern HFILELOG(create_file_log)(const char* path, const char* name);
    extern void (destroy_file_log)(HFILELOG log);
    extern bool (file_log_write)(HFILELOG log, enum log_level lv, const char* format, ...);
    extern void (file_log_option)(HFILELOG log, enum log_level lv, bool open_or_not);
    extern bool (file_log_open_or_not)(HFILELOG log, enum log_level lv);
    extern bool (file_log_flush)(HFILELOG log);


#ifdef  __cplusplus
}
#endif