#include <stdlib.h>
#include <memory.h>
#include "../include/type_def.h"
#include "../include/loop_cache.h"
#include "./data_def.h"
#include "../include/memory_pool.h"

__declspec(thread) HMEMORYUNIT def_loop_cache_unit = 0;

inline HMEMORYUNIT _default_loop_cache_unit(void)
{
    return def_loop_cache_unit;
}

HLOOPCACHE create_loop_cache(size_t size, void* data)
{
    HLOOPCACHE cache = 0;

    if (!size)
    {
        return 0;
    }

    cache = (HLOOPCACHE)memory_unit_alloc(_default_loop_cache_unit(), 256);

    if (data)
    {
        cache->cache_begin = (char*)data;
        cache->alloc_cache = 0;
    }
    else
    {
        cache->alloc_cache = (char*)libsvr_memory_manager_alloc(size);
        cache->cache_begin = cache->alloc_cache;
    }

    cache->head = cache->cache_begin;
    cache->tail = cache->cache_begin;
    cache->cache_end = cache->cache_begin + size;
    cache->size = size;

    return cache;
}

void destroy_loop_cache(HLOOPCACHE cache)
{
    if (cache)
    {
        if (cache->alloc_cache)
        {
            libsvr_memory_manager_free(cache->alloc_cache);
            cache->alloc_cache = 0;
        }

        memory_unit_free(_default_loop_cache_unit(), cache);
    }
}

HLOOPCACHE create_loop_cache_ex(size_t size, void* data)
{
    HLOOPCACHE cache = 0;

    if (!size)
    {
        return 0;
    }

    cache = (HLOOPCACHE)malloc(sizeof(loop_cache));

    if (data)
    {
        cache->cache_begin = (char*)data;
        cache->alloc_cache = 0;
    }
    else
    {
        cache->alloc_cache = (char*)malloc(size);
        cache->cache_begin = cache->alloc_cache;
    }

    cache->head = cache->cache_begin;
    cache->tail = cache->cache_begin;
    cache->cache_end = cache->cache_begin + size;
    cache->size = size;

    return cache;
}

void destroy_loop_cache_ex(HLOOPCACHE cache)
{
    if (cache)
    {
        if (cache->alloc_cache)
        {
            free(cache->alloc_cache);
            cache->alloc_cache = 0;
        }

        free(cache);
    }
}

bool loop_cache_push_data(HLOOPCACHE cache, const void* data, size_t size)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (size + used + 1 > cache->size)
        return false;

    if (cache->tail + size >= cache->cache_end)
    {
        size_t seg_1 = cache->cache_end - cache->tail;
        size_t seg_2 = size - seg_1;

        memcpy(cache->tail, data, seg_1);
        memcpy(cache->cache_begin, (char*)data+seg_1, seg_2);
        cache->tail = cache->cache_begin + seg_2;
    }
    else
    {
        memcpy(cache->tail, data, size);
        cache->tail += size;
    }

    return true;
}

bool loop_cache_pop_data(HLOOPCACHE cache, void* data, size_t size)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (size > used)
        return false;

    if (cache->head + size >= cache->cache_end)
    {
        size_t seg_1 = cache->cache_end - cache->head;
        size_t seg_2 = size - seg_1;

        memcpy(data, cache->head, seg_1);
        memcpy((char*)data+seg_1, cache->cache_begin, seg_2);
        cache->head = cache->cache_begin + seg_2;
    }
    else
    {
        memcpy(data, cache->head, size);
        cache->head += size;
    }

    return true;
}

bool loop_cache_copy_data(HLOOPCACHE cache, void* data, size_t size)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (size > used)
        return false;

    if (cache->head + size >= cache->cache_end)
    {
        size_t seg_1 = cache->cache_end - cache->head;
        size_t seg_2 = size - seg_1;

        memcpy(data, cache->head, seg_1);
        memcpy((char*)data+seg_1, cache->cache_begin, seg_2);
    }
    else
    {
        memcpy(data, cache->head, size);
    }

    return true;
}

bool loop_cache_push(HLOOPCACHE cache, size_t size)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (size + used + 1 > cache->size)
    {
        return false;
    }

    if (cache->tail + size >= cache->cache_end)
    {
        size_t seg_1 = cache->cache_end - cache->tail;
        size_t seg_2 = size - seg_1;
        cache->tail = cache->cache_begin + seg_2;
    }
    else
    {
        cache->tail += size;
    }

    return true;
}

bool loop_cache_pop(HLOOPCACHE cache, size_t size)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (size > used)
        return false;

    if (cache->head + size >= cache->cache_end)
    {
        size_t seg_1 = cache->cache_end - cache->head;
        size_t seg_2 = size - seg_1;
        cache->head = cache->cache_begin + seg_2;
    }
    else
    {
        cache->head += size;
    }

    return true;
}

void loop_cache_get_free(HLOOPCACHE cache, void** cache_ptr, size_t* cache_len)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (*cache_len)
    {
        if ((*cache_len) > (cache->size - used -1))
        {
            *cache_len = cache->size - used -1;
        }
    }
    else
    {
        *cache_len = cache->size - used -1;
    }

    if (cache->tail + (*cache_len) >= cache->cache_end)
    {
        *cache_len = cache->cache_end - cache->tail;
    }

    *cache_ptr = cache->tail;
}

void loop_cache_get_data(HLOOPCACHE cache, void** cache_ptr, size_t* cache_len)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    if (*cache_len)
    {
        if ((*cache_len) > used)
        {
            *cache_len = used;
        }
    }
    else
    {
        *cache_len = used;
    }

    if (cache->head + (*cache_len) >= cache->cache_end)
    {
        *cache_len = cache->cache_end - cache->head;
    }

    *cache_ptr = cache->head;
}

size_t loop_cache_free_size(HLOOPCACHE cache)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    return cache->size - used -1;
}

size_t loop_cache_data_size(HLOOPCACHE cache)
{
    size_t dist = cache->tail + cache->size - cache->head;
    size_t used = dist >= cache->size ? (dist - cache->size) : dist;

    return used;
}

size_t loop_cache_size(HLOOPCACHE cache)
{
    return cache->size;
}

void* loop_cache_get_cache(HLOOPCACHE cache)
{
    return cache->cache_begin;
}

void loop_cache_reset(HLOOPCACHE cache, size_t size, void* data)
{
    cache->cache_begin = (char*)data;

    if (cache->alloc_cache)
    {
        free(cache->alloc_cache);
        cache->alloc_cache = 0;
    }
    

    cache->head = cache->cache_begin;
    cache->tail = cache->cache_begin;
    cache->cache_end = cache->cache_begin + size;
    cache->size = size;
}

void loop_cache_reinit(HLOOPCACHE cache)
{
    cache->head = cache->cache_begin;
    cache->tail = cache->cache_begin;
}
