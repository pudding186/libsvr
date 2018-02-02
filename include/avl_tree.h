#pragma once
#include "./type_def.h"
#include "./memory_pool.h"
#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_avl_node* HAVLNODE;
typedef struct st_avl_tree* HAVLTREE;
typedef const struct st_avl_node* CONST_HAVLNODE;
typedef const struct st_avl_tree* CONST_HAVLTREE;

extern HAVLTREE (create_avl_tree)(key_cmp cmp_func);
extern void (destroy_avl_tree)(HAVLTREE tree);

extern HAVLNODE (avl_tree_insert_int)(HAVLTREE tree, int key, void* value);
extern bool (avl_tree_try_insert_int)(HAVLTREE tree, int key, void* value, HAVLNODE* insert_or_exist_node);
extern HAVLNODE (avl_tree_find_int)(HAVLTREE tree, int key);
extern HAVLNODE (avl_tree_find_int_nearby)(HAVLTREE tree, int key);
extern int (avl_node_key_int)(HAVLNODE node);

extern HAVLNODE (avl_tree_insert_int64)(HAVLTREE tree, long long key, void* value);
extern bool (avl_tree_try_insert_int64)(HAVLTREE tree, long long key, void* value, HAVLNODE* insert_or_exist_node);
extern HAVLNODE (avl_tree_find_int64)(HAVLTREE tree, long long key);
extern HAVLNODE (avl_tree_find_int64_nearby)(HAVLTREE tree, long long key);
extern long long (avl_node_key_int64)(HAVLNODE node);

extern HAVLNODE (avl_tree_insert_str)(HAVLTREE tree, const char* key, void* value);
extern bool (avl_tree_try_insert_str)(HAVLTREE tree, const char* key, void* value, HAVLNODE* insert_or_exist_node);
extern HAVLNODE (avl_tree_find_str)(HAVLTREE tree, const char* key);
extern const char* (avl_node_key_str)(HAVLNODE node);

extern HAVLNODE (avl_tree_insert_user)(HAVLTREE tree, void* key, void* value);
extern bool (avl_tree_try_insert_user)(HAVLTREE tree, void* key, void* value, HAVLNODE* insert_or_exist_node);
extern HAVLNODE (avl_tree_find_user)(HAVLTREE tree, void* key);
extern void* (avl_node_key_user)(HAVLNODE node);

extern void* (avl_node_value)(HAVLNODE node);
extern void (avl_node_set_value)(HAVLNODE node, void* new_value);

extern int (avl_node_value_int)(HAVLNODE node);
extern void (avl_node_set_value_int)(HAVLNODE node, int int_value);

extern long long (avl_node_value_int64)(HAVLNODE node);
extern void (avl_node_set_value_int64)(HAVLNODE node, long long int64_value);

extern void (avl_tree_erase)(HAVLTREE tree, HAVLNODE node);
extern void (avl_tree_clear)(HAVLTREE tree);

extern HAVLNODE (avl_first)(CONST_HAVLTREE tree);
extern HAVLNODE (avl_last)(CONST_HAVLTREE tree);
extern HAVLNODE (avl_next)(CONST_HAVLNODE node);
extern HAVLNODE (avl_prev)(CONST_HAVLNODE node);

extern key_cmp (avl_tree_cmp_func_ptr)(HAVLTREE tree);

extern size_t (avl_tree_size)(HAVLTREE tree);

#ifdef  __cplusplus
}
#endif