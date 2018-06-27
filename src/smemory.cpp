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

