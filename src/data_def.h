#pragma once
#include "../include/type_def.h"
#ifdef  __cplusplus
extern "C" {
#endif

    //////////////////////////////////////////////////////////////////////////
    // memory pool data
    typedef struct st_mem_block
    {
        struct st_mem_block*    next; //ָ����һ���ڴ���
    }mem_block;

    typedef struct st_mem_unit
    {
        size_t                  unit_size;      //�ڴ浥Ԫ�Ĵ�С
        struct st_mem_block*    block_head;     //�ڴ������ͷ
        void*                   unit_free_head; //�ɷ����ڴ浥Ԫ����ͷ
        size_t                  use_mem_size;   //ʵ��ռ���ڴ�����         
    }mem_unit;

    typedef struct st_mem_pool
    {
        struct st_mem_unit**    units;          //�ڴ������
        size_t                  unit_size;      //�ڴ�����鳤��
        size_t                  shift;          //λ��ƫ����
        size_t                  align;          //�ڴ�ض����ֽ���,������4�ı���
        size_t                  grow;           //ÿ����չ�ڴ��С
        size_t                  min_mem_size;   //�ڴ�ع������С�ڴ��С��С�ڴ˴�С����С����
        size_t                  max_mem_size;   //�ڴ�ع��������ڴ��С�����ڴ˴�С���ڴ���ϵͳ�й�
        size_t                  use_mem_size;   //ʵ��ռ���ڴ�����
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
        struct st_char_segment* head;     //���ݶ�ͷ
        struct st_char_segment* tail;     //���ݶ�β
        size_t                  buffer_capacity; //����ʵ�ʴ�С
        char*                   buffer_ptr;      //����ָ��
        size_t                  buffer_use_size; //����ʹ�ô�С
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


    extern void* (libsvr_memory_manager_realloc)(void* old_mem, size_t mem_size);

    extern void* (libsvr_memory_manager_alloc)(size_t mem_size);

    extern void (libsvr_memory_manager_free)(void* mem);

    extern bool (libsvr_memory_manager_check)(void* mem);

#ifdef  __cplusplus
}
#endif