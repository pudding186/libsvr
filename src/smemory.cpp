#include <string>
#include <typeinfo>
#include <windows.h>
#include "./data_def.h"
#include "../include/rb_tree.h"
#include "../include/smemory.hpp"
#include "../include/utility.hpp"

extern "C"
{
    extern __declspec(thread) HMEMORYMANAGER lib_svr_mem_mgr;

    extern __declspec(thread) HMEMORYUNIT def_rb_tree_unit;
    extern __declspec(thread) HMEMORYUNIT def_rb_node_unit;

    extern __declspec(thread) HMEMORYUNIT def_avl_tree_unit;
    extern __declspec(thread) HMEMORYUNIT def_avl_node_unit;

    extern __declspec(thread) HMEMORYUNIT def_loop_cache_unit;

    extern __declspec(thread) HMEMORYUNIT def_char_segment_unit;
    extern __declspec(thread) HMEMORYUNIT def_char_buffer_unit;

    extern __declspec(thread) HMEMORYUNIT def_json_node_unit;
    extern __declspec(thread) HMEMORYUNIT def_json_struct_unit;

    extern __declspec(thread) HMEMORYUNIT def_shm_info_unit;
    extern __declspec(thread) HMEMORYUNIT def_shm_mgr_unit;

    extern size_t sizeof_share_memory_info(void);
    extern size_t sizeof_share_mamory_mgr(void);
}

extern __declspec(thread) CFuncPerformanceMgr* def_func_perf_mgr;

namespace SMemory
{
    typedef struct st_trace_info
    {
        const char* name;
        const char* file;
        size_t      line;
        size_t      size;
    }TraceInfo;

    ptrdiff_t trace_info_cmp(void* info1, void* info2)
    {
        TraceInfo* t1 = (TraceInfo*)info1;
        TraceInfo* t2 = (TraceInfo*)info2;

        if (t1->line < t2->line)
        {
            return -1;
        }
        else if (t1->line > t2->line)
        {
            return 1;
        }

        if (t1->file < t2->file)
        {
            return -1;
        }
        else if (t1->file > t2->file)
        {
            return 1;
        }

        return (t1->name - t2->name);
    }

    typedef struct st_ptr_info
    {
        TraceInfo*  info;
        size_t      size;
    }PtrInfo;

    static HRBTREE _get_trace_info_map(void)
    {
        __declspec(thread) static HRBTREE info_tree = 0;

        if (info_tree)
        {
            return info_tree;
        }

        info_tree = create_rb_tree(trace_info_cmp);

        return info_tree;
    }

    static HRBTREE _get_trace_ptr_map(void)
    {
        __declspec(thread) static HRBTREE ptr_tree = 0;

        if (ptr_tree)
        {
            return ptr_tree;
        }

        ptr_tree = create_rb_tree(0);

        return ptr_tree;
    }

    static HMEMORYUNIT _get_trace_info_unit(void)
    {
        __declspec(thread) static HMEMORYUNIT info_unit = 0;

        if (info_unit)
        {
            return info_unit;
        }

        info_unit = create_memory_unit(sizeof(TraceInfo));

        return info_unit;
    }

    static HMEMORYUNIT _get_trace_ptr_unit(void)
    {
        __declspec(thread) static HMEMORYUNIT ptr_unit = 0;

        if (ptr_unit)
        {
            return ptr_unit;
        }

        ptr_unit = create_memory_unit(sizeof(PtrInfo));

        return ptr_unit;
    }

    void trace_alloc(const char* name, const char* file, int line, void* ptr, size_t size)
    {
        HRBNODE node;
        TraceInfo info;
        TraceInfo* exist_info;
        PtrInfo* ptr_info;

        info.file = file;
        info.line = line;
        info.name = name;

        node = rb_tree_find_user(_get_trace_info_map(), &info);

        if (node)
        {
            exist_info = (TraceInfo*)rb_node_key_user(node);
            exist_info->size += size;
        }
        else
        {
            exist_info = (TraceInfo*)memory_unit_alloc(_get_trace_info_unit(), 1024);
            exist_info->file = file;
            exist_info->line = line;
            exist_info->name = name;
            exist_info->size = size;

            if (!rb_tree_try_insert_user(_get_trace_info_map(), exist_info, 0, &node))
            {
                char* p = 0;
                *p = 'a';
            }
        }

        ptr_info = (PtrInfo*)memory_unit_alloc(_get_trace_ptr_unit(), 40960);

        ptr_info->info = exist_info;
        ptr_info->size = size;

        if (!rb_tree_try_insert_user(_get_trace_ptr_map(), ptr, ptr_info, &node))
        {
            char* p = 0;
            *p = 'a';
        }
    }

    void trace_free(void* ptr)
    {
        PtrInfo* ptr_info;

        HRBNODE node = rb_tree_find_user(_get_trace_ptr_map(), ptr);

        if (node)
        {
            ptr_info = (PtrInfo*)rb_node_value(node);

            if (ptr_info->info->size < ptr_info->size)
            {
                char* p = 0;
                *p = 'a';
            }

            ptr_info->info->size -= ptr_info->size;

            rb_tree_erase(_get_trace_ptr_map(), node);

            memory_unit_free(_get_trace_ptr_unit(), ptr_info);
        }
        else
        {
            char* p = 0;
            *p = 'a';
        }
    }

    HRBNODE trace_info_first(void)
    {
        return rb_first(_get_trace_info_map());
    }

    HRBNODE trace_info_next(HRBNODE trace_node)
    {
        return rb_next(trace_node);
    }

    TraceInfo* trace_info_from_node(HRBNODE trace_node)
    {
        return (TraceInfo*)rb_node_key_user(trace_node);
    }

    //////////////////////////////////////////////////////////////////////////

    void Delete(void* ptr)
    {
        if (!ptr)
        {
            return;
        }
        IClassMemory** pool = (IClassMemory**)((unsigned char*)ptr - sizeof(IClassMemory**));
        (*pool)->Delete(ptr);
    }

    void TraceDelete(void* ptr)
    {
        trace_free(ptr);
        Delete(ptr);
    }

    __declspec(thread) HMEMORYMANAGER IClassMemory::def_mem_mgr = 0;

    IClassMemory::IClassMemory(void)
    {
        if (!lib_svr_mem_mgr)
        {
            lib_svr_mem_mgr = create_memory_manager(8, 128, 65536, 4 * 1024, 2);
        }

        if (!def_mem_mgr)
        {
            def_mem_mgr = create_memory_manager(8, 128, 65536, 4 * 1024, 2);
        }

        if (!def_rb_tree_unit)
        {
            def_rb_tree_unit = create_memory_unit(sizeof(rb_tree));
        }

        if (!def_rb_node_unit)
        {
            def_rb_node_unit = create_memory_unit(sizeof(rb_node));
        }

        if (!def_avl_tree_unit)
        {
            def_avl_tree_unit = create_memory_unit(sizeof(avl_tree));
        }

        if (!def_avl_node_unit)
        {
            def_avl_node_unit = create_memory_unit(sizeof(avl_node));
        }

        if (!def_loop_cache_unit)
        {
            def_loop_cache_unit = create_memory_unit(sizeof(loop_cache));
        }

        if (!def_char_buffer_unit)
        {
            def_char_buffer_unit = create_memory_unit(sizeof(char_buffer));
        }

        if (!def_char_segment_unit)
        {
            def_char_segment_unit = create_memory_unit(sizeof(char_segment));
        }

        if (!def_json_struct_unit)
        {
            def_json_struct_unit = create_memory_unit(sizeof(json_struct));
        }

        if (!def_json_node_unit)
        {
            def_json_node_unit = create_memory_unit(sizeof(json_node));
        }

        if (!def_shm_mgr_unit)
        {
            def_shm_mgr_unit = create_memory_unit(sizeof_share_memory_info());
        }

        if (!def_shm_info_unit)
        {
            def_shm_info_unit = create_memory_unit(sizeof_share_mamory_mgr());
        }

        if (!def_func_perf_mgr)
        {
            def_func_perf_mgr = CreateFuncPerfMgr(GetCurrentThreadId());
        }
    }

    IClassMemory::~IClassMemory(void)
    {
        if (def_func_perf_mgr)
        {
            DestroyFuncPerfMgr(def_func_perf_mgr);
            def_func_perf_mgr = 0;
        }

        if (def_shm_info_unit)
        {
            destroy_memory_unit(def_shm_info_unit);
            def_shm_info_unit = 0;
        }

        if (def_shm_mgr_unit)
        {
            destroy_memory_unit(def_shm_mgr_unit);
            def_shm_mgr_unit = 0;
        }

        if (def_json_node_unit)
        {
            destroy_memory_unit(def_json_node_unit);
            def_json_node_unit = 0;
        }

        if (def_json_struct_unit)
        {
            destroy_memory_unit(def_json_struct_unit);
            def_json_struct_unit = 0;
        }

        if (def_char_segment_unit)
        {
            destroy_memory_unit(def_char_segment_unit);
            def_char_segment_unit = 0;
        }

        if (def_char_buffer_unit)
        {
            destroy_memory_unit(def_char_buffer_unit);
            def_char_buffer_unit = 0;
        }

        if (def_loop_cache_unit)
        {
            destroy_memory_unit(def_loop_cache_unit);
            def_loop_cache_unit = 0;
        }

        if (def_avl_node_unit)
        {
            destroy_memory_unit(def_avl_node_unit);
            def_avl_node_unit = 0;
        }

        if (def_avl_tree_unit)
        {
            destroy_memory_unit(def_avl_tree_unit);
            def_avl_tree_unit = 0;
        }

        if (def_rb_node_unit)
        {
            destroy_memory_unit(def_rb_node_unit);
            def_rb_node_unit = 0;
        }

        if (def_rb_tree_unit)
        {
            destroy_memory_unit(def_rb_tree_unit);
            def_rb_tree_unit = 0;
        }

        if (def_mem_mgr)
        {
            destroy_memory_manager(def_mem_mgr);
            def_mem_mgr = 0;
        }

        if (lib_svr_mem_mgr)
        {
            destroy_memory_manager(lib_svr_mem_mgr);
            lib_svr_mem_mgr = 0;
        }
    }

    bool IClassMemory::IsValidPtr(void* ptr)
    {
        if (*(IClassMemory**)((unsigned char*)ptr - sizeof(IClassMemory**)) != this)
        {
            return false;
        }

        if (*(HMEMORYMANAGER*)((unsigned char*)ptr - sizeof(IClassMemory**) - sizeof(HMEMORYMANAGER*)) != def_mem_mgr)
        {
            return false;
        }

        if (memory_check_data((unsigned char*)ptr - sizeof(IClassMemory**) - sizeof(HMEMORYMANAGER*)) == unit)
        {
            return true;
        }

        return memory_manager_check(def_mem_mgr, (unsigned char*)ptr - sizeof(IClassMemory**) - sizeof(HMEMORYMANAGER*) - sizeof(size_t));
    }

    __declspec(thread) static CClassMemory<char> g_memory_manager;
}

