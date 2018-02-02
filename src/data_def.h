#pragma once
#include "../include/type_def.h"
#ifdef  __cplusplus
extern "C" {
#endif

    //////////////////////////////////////////////////////////////////////////
    // memory pool data
    typedef struct st_mem_block
    {
        struct st_mem_block*    next; //指向下一个内存块的
    }mem_block;

    typedef struct st_mem_unit
    {
        size_t                  unit_size;      //内存单元的大小
        struct st_mem_block*    block_head;     //内存块链表头
        void*                   unit_free_head; //可分配内存单元链表头
        size_t                  use_mem_size;   //实际占用内存数量         
    }mem_unit;

    typedef struct st_mem_pool
    {
        struct st_mem_unit**    units;          //内存池数组
        size_t                  unit_size;      //内存池数组长度
        size_t                  shift;          //位移偏移量
        size_t                  align;          //内存池对齐字节数,必须是4的倍数
        size_t                  grow;           //每次扩展内存大小
        size_t                  min_mem_size;   //内存池管理的最小内存大小，小于此大小按最小分配
        size_t                  max_mem_size;   //内存池管理的最大内存大小，大于此大小的内存由系统托管
        size_t                  use_mem_size;   //实际占用内存数量
    }mem_pool;


    //////////////////////////////////////////////////////////////////////////
    // rb tree data

    union un_key
    {
        int         key_int;
        long long   key_int64;
        const char* key_str;
        void*       key_user;
    };

    union un_value
    {
        void*       value_user;
        int         value_int;
        long long   value_int64;
    };

    typedef struct st_rb_node
    {
        struct st_rb_node*  rb_right;
        struct st_rb_node*  rb_left;
        struct st_rb_node*  rb_parent;
        int                 rb_color;

        struct st_rb_node*  list_prev;
        struct st_rb_node*  list_next;

        union un_key        key;
        union un_value      value;


    }rb_node;

    typedef struct st_rb_tree
    {
        struct st_rb_node*  root;
        struct st_rb_node*  head;
        struct st_rb_node*  tail;

        struct st_mem_unit* node_unit;
        size_t              size;

        key_cmp             key_cmp;
    }rb_tree;

    //////////////////////////////////////////////////////////////////////////
    // avl tree data
    typedef struct st_avl_node
    {
        struct st_avl_node* avl_child[2];
        struct st_avl_node* avl_parent;
        ptrdiff_t           avl_height;

        struct st_avl_node* list_prev;
        struct st_avl_node* list_next;

        union un_key        key;
        union un_value      value;
    }avl_node;

    typedef struct st_avl_tree
    {
        struct st_avl_node* root;
        struct st_avl_node* head;
        struct st_avl_node* tail;

        struct st_mem_unit* node_unit;
        size_t              size;

        key_cmp             key_cmp;
    }avl_tree;

    //////////////////////////////////////////////////////////////////////////
    // memory pool manager data
    typedef struct st_mem_mgr
    {
        struct st_avl_tree  mem_pool_map;
    }mem_mgr;

    //////////////////////////////////////////////////////////////////////////
    // loop cache data
    typedef struct st_loop_cache
    {
        char*   cache_begin;
        char*   cache_end;
        char*   head;
        char*   tail;
        char*   alloc_cache;
        size_t  size;
    }loop_cache;

    //////////////////////////////////////////////////////////////////////////
    // char buffer
#define MAX_CHAR_SEGMENT_SIZE   1024

    typedef struct st_char_segment
    {
        struct st_char_segment* next;
        size_t                  use_size;
        char                    data[MAX_CHAR_SEGMENT_SIZE];
    }char_segment;

    typedef struct st_char_buffer
    {
        struct st_char_segment* head;     //数据段头
        struct st_char_segment* tail;     //数据段尾
        size_t                  buffer_capacity; //缓存实际大小
        char*                   buffer_ptr;      //缓存指针
        size_t                  buffer_use_size; //缓冲使用大小
        struct st_mem_unit*     char_segment_unit;
        char                    default_buffer[MAX_CHAR_SEGMENT_SIZE];
    }char_buffer;

    //////////////////////////////////////////////////////////////////////////
    // json data
    typedef struct st_json_node
    {
        struct st_json_node*       next;
        struct st_json_node*       stack;

        enum e_json_value_type  type;
        char*                   key;
        size_t                  key_length;
        size_t                  str_length;

        union
        {
            char*               str;
            long long           number;
            double              number_ex;
            struct st_json_struct* struct_ptr;
        } value;
    }json_node;

    typedef struct st_json_struct
    {
        enum e_json_value_type  type;
        struct st_json_node*    head;
        struct st_json_node*    tail;
        struct st_json_struct*  stack;
    }json_struct;

#ifdef  __cplusplus
}
#endif