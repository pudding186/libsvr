#pragma once
#include <stddef.h>
#include "./type_def.h"
#include "./memory_pool.h"
#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct st_rb_node* HRBNODE;
    typedef struct st_rb_tree* HRBTREE;
    typedef const struct st_rb_node* CONST_HRBNODE;
    typedef const struct st_rb_tree* CONST_HRBTREE;

    extern HRBTREE(create_rb_tree)(key_cmp cmp_func);
    extern void (destroy_rb_tree)(HRBTREE tree);

    extern HRBNODE(rb_tree_insert_int)(HRBTREE tree, int key, void* value);
    extern bool (rb_tree_try_insert_int)(HRBTREE tree, int key, void* value, HRBNODE* insert_or_exist_node);
    extern HRBNODE(rb_tree_find_int)(HRBTREE tree, int key);
    extern HRBNODE(rb_tree_find_int_nearby)(HRBTREE tree, int key);
    extern int (rb_node_key_int)(HRBNODE node);

    extern HRBNODE(rb_tree_insert_int64)(HRBTREE tree, long long key, void* value);
    extern bool (rb_tree_try_insert_int64)(HRBTREE tree, long long key, void* value, HRBNODE* insert_or_exist_node);
    extern HRBNODE(rb_tree_find_int64)(HRBTREE tree, long long key);
    extern HRBNODE(rb_tree_find_int64_nearby)(HRBTREE tree, long long key);
    extern long long (rb_node_key_int64)(HRBNODE node);

    extern HRBNODE(rb_tree_insert_str)(HRBTREE tree, const char* key, void* value);
    extern bool (rb_tree_try_insert_str)(HRBTREE tree, const char* key, void* value, HRBNODE* insert_or_exist_node);
    extern HRBNODE(rb_tree_find_str)(HRBTREE tree, const char* key);
    extern const char* (rb_node_key_str)(HRBNODE node);

    extern HRBNODE(rb_tree_insert_user)(HRBTREE tree, void* key, void* value);
    extern bool (rb_tree_try_insert_user)(HRBTREE tree, void* key, void* value, HRBNODE* insert_or_exist_node);
    extern HRBNODE(rb_tree_find_user)(HRBTREE tree, void* key);
    extern void* (rb_node_key_user)(HRBNODE node);

    extern void* (rb_node_value)(HRBNODE node);
    extern void (rb_node_set_value)(HRBNODE node, void* new_value);

    extern int (rb_node_value_int)(HRBNODE node);
    extern void (rb_node_set_value_int)(HRBNODE node, int int_value);

    extern long long (rb_node_value_int64)(HRBNODE node);
    extern void (rb_node_set_value_int64)(HRBNODE node, long long int64_value);

    extern void (rb_tree_erase)(HRBTREE tree, HRBNODE node);
    extern void (rb_tree_clear)(HRBTREE tree);

    extern key_cmp(rb_tree_cmp_func_ptr)(HRBTREE tree);

    extern HRBNODE(rb_first)(CONST_HRBTREE tree);
    extern HRBNODE(rb_last)(CONST_HRBTREE tree);
    extern HRBNODE(rb_next)(CONST_HRBNODE node);
    extern HRBNODE(rb_prev)(CONST_HRBNODE node);


    extern size_t(rb_tree_size)(HRBTREE tree);

#ifdef  __cplusplus
}
#endif