#pragma once
#include "./type_def.h"
#include "./memory_pool.h"

#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct st_json_struct* HJSONSTRUCT;
    typedef struct st_json_node* HJSONNODE;

    typedef void(*json_string_append) (void *ud, const char* data, size_t data_size);

    extern HJSONSTRUCT(string_to_json)(const char* str, char* err_info, size_t err_info_len);
    extern size_t(json_to_string)(HJSONSTRUCT json_struct_ptr, char* json_str_ptr, size_t json_str_size);
    extern void (json_to_string_ex)(HJSONSTRUCT json_struct_ptr, json_string_append f, void* ud);


    extern HJSONSTRUCT(create_json)(enum e_json_value_type type);
    extern void (destroy_json)(HJSONSTRUCT json_struct);

    extern HJSONNODE(json_add_integer)(HJSONSTRUCT json_struct, long long value, const char* key, size_t key_length);
    extern HJSONNODE(json_add_float)(HJSONSTRUCT json_struct, double value, const char* key, size_t key_length);
    extern HJSONNODE(json_add_string)(HJSONSTRUCT json_struct, const char* value, size_t value_length, const char* key, size_t key_length);
    extern HJSONNODE(json_add_true)(HJSONSTRUCT json_struct, const char* key, size_t key_length);
    extern HJSONNODE(json_add_false)(HJSONSTRUCT json_struct, const char* key, size_t key_length);
    extern HJSONNODE(json_add_null)(HJSONSTRUCT json_struct, const char* key, size_t key_length);
    extern HJSONSTRUCT(json_add_struct)(HJSONSTRUCT json_struct, enum e_json_value_type type, const char* key, size_t key_length);


    extern HJSONNODE(json_struct_first_node)(HJSONSTRUCT json_struct);
    extern HJSONNODE(json_struct_next_node)(HJSONNODE json_node);

    extern const char* (json_key)(HJSONNODE json_node);
    extern bool (json_value_integer)(HJSONNODE json_node, long long* value);
    extern bool (json_value_float)(HJSONNODE json_node, double* value);
    extern bool (json_value_string)(HJSONNODE json_node, const char** value, size_t* length);
    extern bool (json_value_true)(HJSONNODE json_node);
    extern bool (json_value_false)(HJSONNODE json_node);
    extern bool (json_value_null)(HJSONNODE json_node);
    extern HJSONSTRUCT(json_value_struct)(HJSONNODE json_node);
    extern enum e_json_value_type(json_value_type)(HJSONNODE json_node);
    extern enum e_json_value_type(json_struct_type)(HJSONSTRUCT json_struct);


#ifdef  __cplusplus
}
#endif