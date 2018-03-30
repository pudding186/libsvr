#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_mem_unit*     HMEMORYUNIT;
typedef struct st_mem_pool*     HMEMORYPOOL;
typedef struct st_mem_mgr*      HMEMORYMANAGER;

extern HMEMORYUNIT (create_memory_unit)(size_t unit_size);

extern void (destroy_memory_unit)(HMEMORYUNIT unit);

extern void* (memory_unit_alloc)(HMEMORYUNIT unit, size_t grow_byte);

extern void* (memory_unit_alloc_ex)(HMEMORYUNIT unit, size_t grow_number);

extern void (memory_unit_free)(HMEMORYUNIT unit, void* mem);

extern void (memory_unit_quick_free)(HMEMORYUNIT unit, void* mem);

extern size_t (memory_unit_use_memory_size)(HMEMORYUNIT unit);

extern HMEMORYPOOL (create_memory_pool)(size_t align, size_t min_mem_size, size_t max_mem_size, size_t grow_size);

extern void (destroy_memory_pool)(HMEMORYPOOL pool);

extern void* (memory_pool_alloc)(HMEMORYPOOL pool, size_t mem_size);

extern void* (memory_pool_realloc)(HMEMORYPOOL pool, void* old_mem, size_t mem_size);

extern void (memory_pool_free)(HMEMORYPOOL pool, void* mem);

extern void (memory_pool_set_grow)(HMEMORYPOOL pool, size_t grow_size);

extern size_t (memory_pool_use_memory_size)(HMEMORYPOOL pool);

extern void* (memory_check_data)(void* mem);

extern size_t (memory_unit_size)(HMEMORYUNIT unit);

extern HMEMORYMANAGER (create_memory_manager)(size_t align, size_t start_size, size_t max_size, size_t grow_size, size_t grow_power);

extern void (destroy_memory_manager)(HMEMORYMANAGER mgr);

extern void* (memory_manager_alloc)(HMEMORYMANAGER mgr, size_t mem_size);

extern void* (memory_manager_realloc)(HMEMORYMANAGER mgr, void* old_mem, size_t mem_size);

extern void (memory_manager_free)(HMEMORYMANAGER mgr, void* mem);

extern bool (memory_unit_check)(HMEMORYUNIT unit, void* mem);

extern bool (memory_pool_check)(HMEMORYPOOL pool, void* mem);

extern bool (memory_manager_check)(HMEMORYMANAGER mgr, void* mem);

#ifdef  __cplusplus
}
#endif