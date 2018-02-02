#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

typedef bool (*pfn_file)(const char* path, const char* file, void* user_data);
typedef bool (*pfn_dir)(const char* dir, void* user_data);

extern bool (for_each_file)(const char* dir, pfn_file do_file, pfn_dir do_dir, void* user_data);

#ifdef  __cplusplus
}
#endif