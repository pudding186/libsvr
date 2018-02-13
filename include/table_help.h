#pragma once
#include "rb_tree.h"
#ifdef  __cplusplus
extern "C" {
#endif

typedef enum e_col_data_type
{
    col_null = 0,
    col_int8,
    col_uint8,
    col_int16,
    col_uint16,
    col_int32,
    col_uint32,
    col_int64,
    col_uint64,
    col_string
}col_data_type;

typedef struct st_col_var_info
{
    size_t          col_var_offset;
    col_data_type   col_var_type;
}col_var_info;

typedef struct st_col_var
{
    void*           col_var_ptr;
    col_data_type   col_var_type;
}col_var;

extern HRBTREE (quick_tree)(HRBTREE tree, size_t elapse);
extern void* (quick_tree_find)(HRBTREE tree, size_t key);
extern void (destroy_quick_tree)(HRBTREE tree);
extern size_t (quick_tree_node_value)(HRBNODE node, void*** value_arry);
extern bool (tree_is_quick)(HRBTREE tree);

extern void* (tree_find_int64)(HRBTREE tree, unsigned long long key);
extern void* (tree_find_int)(HRBTREE tree, unsigned long key);
extern void* (tree_find_str)(HRBTREE tree, const char* key);

extern void (add_col_info)(HRBTREE tree, const char* col_var_name, size_t col_var_offset, col_data_type col_var_type);
extern col_var_info* (get_col_info)(HRBTREE tree, const char* col_var_name);
extern void (del_col_info)(HRBTREE tree, const char* col_var_name);

#ifdef  __cplusplus
}
#endif