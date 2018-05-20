#pragma once
#include "../include/type_def.h"
#include "../include/memory_pool.h"

#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct st_share_memory_info* HSHMINFO;
    typedef struct st_share_memory_info_mgr* HSHMMGR;

    extern HSHMMGR(create_shm_mgr)(void);

    extern void (destroy_shm_mgr)(HSHMMGR mgr);

    extern HSHMINFO(shm_get_info)(HSHMMGR mgr, int key);

    extern void* (shm_alloc)(HSHMMGR mgr, int key, unsigned int size);

    extern void (shm_free)(HSHMMGR mgr, int key);

#ifdef  __cplusplus
}
#endif