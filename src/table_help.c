#include "../include/table_help.h"
#include "../include/memory_pool.h"
//#include <memory.h>
#include <string.h>

typedef struct st_int_key_group
{
    size_t key_begin;
    size_t key_end;
}int_key_group;

ptrdiff_t int_key_cmp(void* l_key, void* r_key)
{
    int_key_group* key_l = (int_key_group*)l_key;
    int_key_group* key_r = (int_key_group*)r_key;

    if (key_l->key_end < key_r->key_begin)
    {
        return -1;
    }
    else if (key_l->key_begin > key_r->key_end)
    {
        return 1;
    }
    else
        return 0;
}

void* quick_tree_find(HRBTREE tree, size_t idx)
{
    HRBNODE node;
    int_key_group key_group;
    key_group.key_begin = idx;
    key_group.key_end = idx;

    node = rb_tree_find_user(tree, &key_group);

    if (node)
    {
        int_key_group* node_key_group = (int_key_group*)rb_node_key_user(node);
        void** arry = (void**)rb_node_value(node);

        return arry[idx - node_key_group->key_begin];
    }

    return 0;
}

void* tree_find_int64( HRBTREE tree, unsigned long long key )
{
    HRBNODE node = rb_tree_find_int64(tree, key);
    if (node)
    {
        return rb_node_value(node);
    }

    return 0;
}

void* tree_find_int(HRBTREE tree, size_t key)
{
    HRBNODE node = rb_tree_find_int(tree, key);
    if (node)
    {
        return rb_node_value(node);
    }

    return 0;
}

void* tree_find_str( HRBTREE tree, const char* key )
{
    HRBNODE node = rb_tree_find_str(tree, key);
    if (node)
    {
        return rb_node_value(node);
    }

    return 0;
}

size_t quick_tree_node_value(HRBNODE node, void*** value_arry)
{
    int_key_group* key = (int_key_group*)rb_node_key_user(node);
    *value_arry = (void**)rb_node_value(node);

    return (key->key_end - key->key_begin + 1);
}

void destroy_quick_tree(HRBTREE tree)
{
    HRBNODE node = rb_first(tree);

    while (node)
    {
        default_memory_manager_free(rb_node_key_user(node));
        default_memory_manager_free(rb_node_value(node));

        node = rb_next(node);
    }

    destroy_rb_tree(tree);
}

HRBTREE quick_tree(HRBTREE tree, size_t elapse)
{
    HRBTREE new_tree = create_rb_tree(int_key_cmp);

    int_key_group* key_group = 0;

    size_t value_group_count = 64;
    void** value_group = (void**)default_memory_manager_alloc(value_group_count*sizeof(void*));
    size_t group_count = 0;

    HRBNODE node = rb_first(tree);
    while (node)
    {
        if (key_group)
        {
            size_t key = rb_node_key_int(node);

            if (key - key_group->key_end <= elapse)
            {
                key_group->key_end++;
                while (key_group->key_end < key)
                {
                    if (group_count >= value_group_count)
                    {
                        void** tmp;
                        value_group_count += 1024;

                        tmp = (void**)default_memory_manager_alloc(value_group_count*sizeof(void*));
                        memcpy(tmp, value_group, group_count*sizeof(void*));
                        default_memory_manager_free(value_group);
                        value_group = tmp;
                    }
                    value_group[group_count] = 0;
                    group_count++;
                    key_group->key_end++;
                }

                if (group_count >= value_group_count)
                {
                    void** tmp;
                    value_group_count += 1024;

                    tmp = (void**)default_memory_manager_alloc(value_group_count*sizeof(void*));
                    memcpy(tmp, value_group, group_count*sizeof(void*));
                    default_memory_manager_free(value_group);
                    value_group = tmp;
                }
                value_group[group_count] = rb_node_value(node);
                group_count++;
            }
            else
            {
                void** real_value_group = (void**)default_memory_manager_alloc(group_count*sizeof(void*));
                memcpy(real_value_group, value_group, group_count*sizeof(void*));
                rb_tree_insert_user(new_tree, key_group, real_value_group);

                key_group = (int_key_group*)default_memory_manager_alloc(sizeof(int_key_group));
                key_group->key_begin = key;
                key_group->key_end = key_group->key_begin;
                value_group[0] = rb_node_value(node);
                group_count = 1;
            }
        }
        else
        {
            key_group = (int_key_group*)default_memory_manager_alloc(sizeof(int_key_group));
            key_group->key_begin = rb_node_key_int(node);
            key_group->key_end = key_group->key_begin;
            value_group[0] = rb_node_value(node);
            group_count = 1;
        }
        node = rb_next(node);
    }

    if (key_group)
    {
        void** real_value_group = (void**)default_memory_manager_alloc(group_count*sizeof(void*));
        memcpy(real_value_group, value_group, group_count*sizeof(void*));
        rb_tree_insert_user(new_tree, key_group, real_value_group);
    }

    default_memory_manager_free(value_group);

    destroy_rb_tree(tree);

    return new_tree;
}

bool tree_is_quick(HRBTREE tree)
{
    if (rb_tree_cmp_func_ptr(tree) == int_key_cmp)
    {
        return true;
    }

    return false;
}

void add_col_info(HRBTREE tree, const char* col_var_name, size_t col_var_offset, col_data_type col_var_type)
{
    HRBNODE exist_node;
    col_var_info* info;

    info = (col_var_info*)default_memory_manager_alloc(sizeof(col_var_info));
    info->col_var_offset = col_var_offset;
    info->col_var_type = col_var_type;

    if (!rb_tree_try_insert_str(tree, col_var_name, info, &exist_node))
    {
        default_memory_manager_free(rb_node_value(exist_node));
        rb_node_set_value(exist_node, info);
    }
}

col_var_info* get_col_info(HRBTREE tree, const char* col_var_name)
{
    HRBNODE node = rb_tree_find_str(tree, col_var_name);
    if (node)
    {
        return (col_var_info*)rb_node_value(node);
    }

    return 0;
}

void del_col_info(HRBTREE tree, const char* col_var_name)
{
    HRBNODE node = rb_tree_find_str(tree, col_var_name);
    if (node)
    {
        default_memory_manager_free(rb_node_value(node));
    }
}