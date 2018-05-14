#include <stdlib.h>
#include <memory.h>

#include "../include/type_def.h"
#include "../include/avl_tree.h"
#include "../include/memory_pool.h"
#include "./data_def.h"

#define CRUSH_CODE char* p = 0;*p = 'a';

__declspec(thread) HMEMORYMANAGER lib_svr_mem_mgr = 0;

mem_block* _create_memory_block(mem_unit* unit, size_t unit_count)
{
    unsigned char* ptr;
    size_t i;
    mem_block* block;
    size_t block_size = sizeof(mem_block) + unit_count*(sizeof(void*)+unit->unit_size);

    block = (mem_block*)malloc(block_size);

    if (block)
    {
        block->next = unit->block_head;
        unit->block_head = block;

        ptr = (unsigned char*)block + sizeof(mem_block);

        for (i = 0; i < unit_count-1; i++)
        {
            *((void**)ptr) = ptr + sizeof(void*)+unit->unit_size;
            ptr += sizeof(void*)+unit->unit_size;
        }

        *((void**)ptr) = unit->unit_free_head;
        unit->unit_free_head = (unsigned char*)block + sizeof(mem_block);
        unit->use_mem_size += block_size;
    }

    return block;
}

HMEMORYUNIT create_memory_unit(size_t unit_size)
{
    HMEMORYUNIT unit = (HMEMORYUNIT)malloc(sizeof(mem_unit));
    if (unit)
    {
        unit->unit_size = unit_size;
        unit->block_head = 0;
        unit->unit_free_head = 0;
        unit->use_mem_size = 0;
    }

    return unit;
}

void destroy_memory_unit(HMEMORYUNIT unit)
{
    while (unit->block_head)
    {
        mem_block* block = unit->block_head;
        unit->block_head = block->next;

        free(block);
    }

    free(unit);
}

void* memory_unit_alloc(HMEMORYUNIT unit, size_t grow_size)
{
    void* alloc_mem;

    if (!unit->unit_free_head)
    {
        size_t unit_count;

        if (!grow_size)
        {
            return 0;
        }

        if (grow_size <= sizeof(mem_block))
        {
            grow_size = sizeof(mem_block);
        }
        
        unit_count = (grow_size - sizeof(mem_block))/(sizeof(void*)+unit->unit_size);

        if (unit_count <= 0)
        {
            unit_count = 1;
        }

        if (!_create_memory_block(unit, unit_count))
        {
            return 0;
        }
    }

    alloc_mem = unit->unit_free_head;

    unit->unit_free_head = *(void**)alloc_mem;

    *(void**)alloc_mem = unit;

    return (unsigned char*)alloc_mem + sizeof(void*);
}

void* memory_unit_alloc_ex(HMEMORYUNIT unit, size_t grow_count)
{
    void* alloc_mem;

    if (!unit->unit_free_head)
    {
        if (!grow_count)
        {
            return 0;
        }

        if (!_create_memory_block(unit, grow_count))
        {
            return 0;
        }
    }

    alloc_mem = unit->unit_free_head;

    unit->unit_free_head = *(void**)alloc_mem;

    *(void**)alloc_mem = unit;

    return (unsigned char*)alloc_mem + sizeof(void*);
}

void memory_unit_free(HMEMORYUNIT unit, void* mem)
{
    HMEMORYUNIT check_unit = (HMEMORYUNIT)memory_check_data(mem);

    if (unit)
    {
        if (check_unit != unit)
        {
            CRUSH_CODE;
            return;
        }
    }

    *(void**)((unsigned char*)mem - sizeof(void*)) = check_unit->unit_free_head;
    check_unit->unit_free_head = (unsigned char*)mem - sizeof(void*);
}

void memory_unit_quick_free(HMEMORYUNIT unit, void* mem)
{
    *(void**)((unsigned char*)mem - sizeof(void*)) = unit->unit_free_head;
    unit->unit_free_head = (unsigned char*)mem - sizeof(void*);
}

HMEMORYPOOL create_memory_pool(size_t align, size_t min_mem_size, size_t max_mem_size, size_t grow_size)
{
    size_t mod = 0;
    size_t k = align;
    HMEMORYPOOL pool = (HMEMORYPOOL)malloc(sizeof(mem_pool));

    if (min_mem_size < sizeof(size_t))
    {
        min_mem_size = sizeof(size_t);
    }

    if (max_mem_size < min_mem_size)
    {
        return 0;
    }

    if (align)
    {
        mod = align % sizeof(size_t);
        if (mod)
        {
            pool->align = align + sizeof(size_t) - mod;
        }
        else
            pool->align = align;
    }
    else
        return 0;

    mod = max_mem_size % pool->align;
    if (mod)
    {
        pool->max_mem_size = max_mem_size + pool->align - mod;
    }
    else
        pool->max_mem_size = max_mem_size;

    mod = min_mem_size % pool->align;
    if (mod)
    {
        pool->min_mem_size = min_mem_size + pool->align - mod;
    }
    else
        pool->min_mem_size = min_mem_size;

    pool->shift = 0;
    for (; !(k & 1); k >>= 1, ++pool->shift);             // mod MAX_MEM_ALIGN align

    pool->use_mem_size = 0;
    pool->unit_size = (pool->max_mem_size - pool->min_mem_size) / pool->align + 1;

    pool->units = (struct st_mem_unit**)malloc(pool->unit_size * sizeof(struct st_mem_unit*));
    pool->use_mem_size += pool->unit_size * sizeof(struct st_mem_unit*);

    for (k = 0; k < pool->unit_size; k++)
    {
        pool->units[k] = 0;
    }

    pool->grow = 4 * 1024;
    if (grow_size > pool->grow)
    {
        pool->grow = grow_size;
    }

    return pool;
}

void destroy_memory_pool(HMEMORYPOOL pool)
{
    size_t i;

    for (i = 0; i < pool->unit_size; ++i)
    {
        if (pool->units[i])
        {
            destroy_memory_unit(pool->units[i]);
        }
    }

    free(pool->units);

    free(pool);
}

void* memory_pool_alloc(HMEMORYPOOL pool, size_t mem_size)
{
    size_t i;
    HMEMORYUNIT unit;
    void* alloc_mem;

    if (!mem_size)
    {
        return 0;
    }

    if (mem_size > pool->max_mem_size)
    {
        unsigned char* mem = (unsigned char*)malloc(sizeof(void*) + mem_size);
        *(void**)mem = pool;
        return mem + sizeof(void*);
    }

    if (mem_size <= pool->min_mem_size)
    {
        i = 0;
    }
    else
    {
        mem_size -= pool->min_mem_size;

        i = (mem_size & (pool->align - 1)) ?
            (mem_size >> pool->shift) + 1 : (mem_size >> pool->shift);
    }

    unit = pool->units[i];

    if (!unit)
    {
        pool->units[i] = create_memory_unit(pool->min_mem_size + i * pool->align);
        unit = pool->units[i];

        if (!unit)
        {
            return 0;
        }
        pool->use_mem_size += sizeof(mem_unit);
    }

    if (!unit->unit_free_head)
    {
        size_t last_unit_size = unit->use_mem_size;
        size_t unit_count = (pool->grow - sizeof(mem_block))/(sizeof(void*)+unit->unit_size);
        if (unit_count <= 0)
        {
            unit_count = 1;
        }

        if (!_create_memory_block(unit, unit_count))
        {
            return 0;
        }

        pool->use_mem_size += (unit->use_mem_size - last_unit_size);
    }

    alloc_mem = unit->unit_free_head;

    unit->unit_free_head = *(void**)alloc_mem;

    *(void**)alloc_mem = unit;

    return (unsigned char*)alloc_mem + sizeof(void*);
}

void* memory_pool_realloc(HMEMORYPOOL pool, void* old_mem, size_t mem_size)
{
    void* new_mem = 0;

    if (!old_mem)
    {
        return memory_pool_alloc(pool, mem_size);
    }

    if ((HMEMORYPOOL)memory_check_data(old_mem) == pool)
    {
        unsigned char* new_mem_ptr = (unsigned char*)realloc((unsigned char*)old_mem - sizeof(void*), mem_size + sizeof(void*));

        if (new_mem_ptr)
        {
            return new_mem_ptr + sizeof(void*);
        }

        return 0;
    }
    else
    {
        HMEMORYUNIT unit = (HMEMORYUNIT)memory_check_data(old_mem);

        if (unit == pool->units[(unit->unit_size - pool->min_mem_size) >> pool->shift])
        {
            if (unit->unit_size >= mem_size)
                return old_mem;

            new_mem = memory_pool_alloc(pool, mem_size);

            memcpy(new_mem, old_mem, unit->unit_size);

            memory_unit_quick_free(unit, old_mem);

            return new_mem;
        }

        {
            CRUSH_CODE;
        }

        return 0;
    }
}

void memory_pool_free(HMEMORYPOOL pool, void* mem)
{
    HMEMORYUNIT unit;

    if (!mem)
        return;

    unit = *(HMEMORYUNIT*)((unsigned char*)mem - sizeof(void*));

    if (((HMEMORYPOOL)unit) == pool)
    {
        free((unsigned char*)mem - sizeof(void*));
        return;
    }

    if (unit == pool->units[(unit->unit_size - pool->min_mem_size) >> pool->shift])
    {
        memory_unit_quick_free(unit, mem);
        return;
    }

    {
        CRUSH_CODE;
    }
}

void memory_pool_set_grow(HMEMORYPOOL pool, size_t grow_size)
{
    if (grow_size <= 4*1024)
    {
        pool->grow = 4*1024;
    }
    else
        pool->grow = grow_size;
}

size_t memory_pool_use_memory_size(HMEMORYPOOL pool)
{
    return pool->use_mem_size;
}

size_t memory_unit_use_memory_size(HMEMORYUNIT unit)
{
    return unit->use_mem_size;
}

void* memory_check_data(void* mem)
{
    return *(void**)((unsigned char*)mem - sizeof(void*));
}

size_t memory_unit_size(HMEMORYUNIT unit)
{
    return unit->unit_size;
}

HMEMORYMANAGER create_memory_manager(size_t align, size_t start_size, size_t max_size, size_t grow_size, size_t grow_power)
{
    size_t last_start_size = sizeof(size_t);

    HMEMORYMANAGER mgr = (HMEMORYMANAGER)malloc(sizeof(mem_mgr));
    mgr->mem_pool_map.root = 0;
    mgr->mem_pool_map.size = 0;
    mgr->mem_pool_map.head = 0;
    mgr->mem_pool_map.tail = 0;
    mgr->mem_pool_map.key_cmp = 0;
    mgr->mem_pool_map.node_unit = create_memory_unit(sizeof(avl_node));

    while (start_size <= max_size)
    {
        HMEMORYPOOL pool = create_memory_pool(align, last_start_size, start_size, grow_size);

        avl_tree_insert_int64(&mgr->mem_pool_map, pool->max_mem_size, pool);
        align *= grow_power;
        last_start_size = start_size+1;
        start_size *= grow_power;
    }

    return mgr;
}

void destroy_memory_manager(HMEMORYMANAGER mgr)
{
    HAVLNODE pool_node = avl_first(&mgr->mem_pool_map);
    while (pool_node)
    {
        destroy_memory_pool((HMEMORYPOOL)avl_node_value(pool_node));
        pool_node = avl_next(pool_node);
    }
    destroy_memory_unit(mgr->mem_pool_map.node_unit);
    free(mgr);
}


void* memory_manager_alloc(HMEMORYMANAGER mgr, size_t mem_size)
{
    HAVLNODE pool_node = avl_tree_find_int64_nearby(&mgr->mem_pool_map, mem_size);

    if ((size_t)avl_node_key_int64(pool_node) < mem_size)
    {
        if (avl_next(pool_node))
        {
            pool_node = avl_next(pool_node);
        }
    }

    return memory_pool_alloc((HMEMORYPOOL)avl_node_value(pool_node), mem_size);
}

void* memory_manager_realloc(HMEMORYMANAGER mgr, void * old_mem, size_t mem_size)
{
    HMEMORYUNIT unit = 0;
    void* new_mem = 0;
    HAVLNODE old_node = 0;
    HAVLNODE new_node = 0;

    if (!old_mem)
    {
        return memory_manager_alloc(mgr, mem_size);
    }

    unit = memory_check_data(old_mem);

    if (unit == avl_node_value(avl_last(&mgr->mem_pool_map)))
    {
        return memory_pool_realloc((HMEMORYPOOL)avl_node_value(avl_last(&mgr->mem_pool_map)), old_mem, mem_size);
    }

    if (mem_size <= unit->unit_size)
    {
        return old_mem;
    }

    old_node = avl_tree_find_int64_nearby(&mgr->mem_pool_map, unit->unit_size);
    if ((size_t)avl_node_key_int64(old_node) < unit->unit_size)
    {
        if (avl_next(old_node))
        {
            old_node = avl_next(old_node);
        }
    }

    new_node = avl_tree_find_int64_nearby(&mgr->mem_pool_map, mem_size);
    if ((size_t)avl_node_key_int64(new_node) < mem_size)
    {
        if (avl_next(new_node))
        {
            new_node = avl_next(new_node);
        }
    }

    if (old_node == new_node)
    {
        return memory_pool_realloc((HMEMORYPOOL)avl_node_value(old_node), old_mem, mem_size);
    }

    new_mem = memory_pool_alloc((HMEMORYPOOL)avl_node_value(new_node), mem_size);
    memcpy(new_mem, old_mem, unit->unit_size);
    memory_pool_free((HMEMORYPOOL)avl_node_value(old_node), old_mem);
    return new_mem;
}

void memory_manager_free(HMEMORYMANAGER mgr, void * mem)
{
    HMEMORYUNIT unit = 0;
    HAVLNODE pool_node = 0;
    HMEMORYPOOL pool = 0;

    if (!mem)
        return;

    unit = memory_check_data(mem);

    if (unit == avl_node_value(avl_last(&mgr->mem_pool_map)))
    {
        memory_pool_free((HMEMORYPOOL)avl_node_value(avl_last(&mgr->mem_pool_map)), mem);
        return;
    }

    pool_node = avl_tree_find_int64_nearby(&mgr->mem_pool_map, unit->unit_size);
    if ((size_t)avl_node_key_int64(pool_node) < unit->unit_size)
    {
        if (avl_next(pool_node))
        {
            pool_node = avl_next(pool_node);
        }
    }

    if (!pool_node)
    {
        CRUSH_CODE;
    }

    pool = (HMEMORYPOOL)avl_node_value(pool_node);

    if (pool->units[(unit->unit_size - pool->min_mem_size) >> pool->shift] == unit)
    {
        memory_unit_quick_free(unit, mem);
    }
    else
    {
        CRUSH_CODE;
    }
}



bool memory_unit_check(HMEMORYUNIT unit, void* mem)
{
    if ((HMEMORYUNIT)memory_check_data(mem) == unit)
    {
        return true;
    }

    return false;
}

bool memory_pool_check(HMEMORYPOOL pool, void* mem)
{
    HMEMORYUNIT unit;

    if ((HMEMORYPOOL)memory_check_data(mem) == pool)
    {
        return true;
    }

    unit = (HMEMORYUNIT)memory_check_data(mem);

    if (unit == pool->units[(unit->unit_size - pool->min_mem_size) >> pool->shift])
    {
        return true;
    }

    return false;
}

bool memory_manager_check(HMEMORYMANAGER mgr, void * mem)
{
    HMEMORYUNIT unit = 0;
    HAVLNODE pool_node = 0;
    HMEMORYPOOL pool = 0;

    unit = memory_check_data(mem);

    if (unit == avl_node_value(avl_last(&mgr->mem_pool_map)))
    {
        return true;
    }

    pool_node = avl_tree_find_int64_nearby(&mgr->mem_pool_map, unit->unit_size);
    if ((size_t)avl_node_key_int64(pool_node) < unit->unit_size)
    {
        if (avl_next(pool_node))
        {
            pool_node = avl_next(pool_node);
        }
    }

    if (!pool_node)
    {
        CRUSH_CODE;
    }

    pool = (HMEMORYPOOL)avl_node_value(pool_node);

    if (pool->units[(unit->unit_size - pool->min_mem_size) >> pool->shift] == unit)
    {
        return true;
    }

    return false;
}

void* libsvr_memory_manager_realloc(void* old_mem, size_t mem_size)
{
    return memory_manager_realloc(lib_svr_mem_mgr, old_mem, mem_size);
}

void* libsvr_memory_manager_alloc(size_t mem_size)
{
    return memory_manager_alloc(lib_svr_mem_mgr, mem_size);
}

void libsvr_memory_manager_free(void* mem)
{
    memory_manager_free(lib_svr_mem_mgr, mem);
}

bool libsvr_memory_manager_check(void* mem)
{
    return memory_manager_check(lib_svr_mem_mgr, mem);
}