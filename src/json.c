#include "../include/type_def.h"
#include "./data_def.h"
#include "../include/json.h"
#include "../include/memory_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CRUSH_CODE char* p = 0;*p = 'a';
#define TRIM_SPACE(char_ptr) while (isspace(*char_ptr)) ++char_ptr;

__declspec(thread) HMEMORYUNIT def_json_node_unit = 0;
__declspec(thread) HMEMORYUNIT def_json_struct_unit = 0;

inline HMEMORYUNIT _default_json_node_unit(void)
{
    return def_json_node_unit;
}

inline HMEMORYUNIT _default_json_struct_unit(void)
{
    return def_json_struct_unit;
}

void escape_to_json_append(const char* src, size_t n, json_string_append f, void* ud)
{
    char *s = (char*)src, *e = (char*)src + n;
    while (s < e)
    {
        switch (*s)
        {
        case '\\':
        {
            (*f)(ud, "\\\\", 2);
            break;
        }
        case '/':
        {
            (*f)(ud, "\\/", 2);
            break;
        }
        case '"':
        {
            (*f)(ud, "\\\"", 2);
            break;
        }
        case '\t':
        {
            (*f)(ud, "\\t", 2);
            break;
        }
        case '\f':
        {
            (*f)(ud, "\\f", 2);
            break;
        }
        case '\b':
        {
            (*f)(ud, "\\b", 2);
            break;
        }
        case '\n':
        {
            (*f)(ud, "\\n", 2);
            break;
        }
        case '\r':
        {
            (*f)(ud, "\\r", 2);
            break;
        }
        default:
        {
            (*f)(ud, s, 1);
            break;
        }
        }
        ++s;
    }

}

size_t escape_to(const char* src, size_t n, char* dst)
{
    char* p = dst;
    char *s = (char*)src, *e = (char*)src + n;
    while (s < e)
    {
        switch (*s)
        {
        case '\\':
        {
            memcpy(p, "\\\\", 2);
            p += 2;
            break;
        }
        case '/':
        {
            memcpy(p, "\\/", 2);
            p += 2;
            break;
        }
        case '"':
        {
            memcpy(p, "\\\"", 2);
            p += 2;
            break;
        }
        case '\t':
        {
            memcpy(p, "\\t", 2);
            p += 2;
            break;
        }
        case '\f':
        {
            memcpy(p, "\\f", 2);
            p += 2;
            break;
        }
        case '\b':
        {
            memcpy(p, "\\b", 2);
            p += 2;
            break;
        }
        case '\n':
        {
            memcpy(p, "\\n", 2);
            p += 2;
            break;
        }
        case '\r':
        {
            memcpy(p, "\\r", 2);
            p += 2;
            break;
        }
        default:
        {
            memcpy(p, s, 1);
            p += 1;
            break;
        }
        }
        ++s;
    }

    return p - dst;
}

size_t escape_from(const char* src, size_t n, char* dst)
{
    char* p = dst;
    char *s = (char*)src, *e = (char*)src + n;
    while (s < e)
    {
        if (*s == '\\')
        {
            if (*(s + 1) == '\\')
            {
                memcpy(p, "\\", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == '/')
            {
                memcpy(p, "/", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == '"')
            {
                memcpy(p, "\"", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == 't')
            {
                memcpy(p, "\t", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == 'f')
            {
                memcpy(p, "\f", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == 'b')
            {
                memcpy(p, "\b", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == 'n')
            {
                memcpy(p, "\n", 1);
                s += 2;
                p += 1;
            }
            else if (*(s + 1) == 'r')
            {
                memcpy(p, "\r", 1);
                s += 2;
                p += 1;
            }
            else
            {
                memcpy(p, s, 1);
                ++s;
                p += 1;
            }
        }
        else
        {
            memcpy(p, s, 1);
            ++s;
            p += 1;
        }
    }
    return (p - dst);
}

struct st_json_struct* _json_create_object(void)
{
    struct st_json_struct* json_struct_ptr = (struct st_json_struct*)memory_unit_alloc(_default_json_struct_unit(), 4 * 1024);
    json_struct_ptr->head = 0;
    json_struct_ptr->tail = 0;
    json_struct_ptr->stack = 0;
    json_struct_ptr->type = json_object;


    return json_struct_ptr;
}

struct st_json_struct* _json_create_array(void)
{
    struct st_json_struct* json_struct_ptr = (struct st_json_struct*)memory_unit_alloc(_default_json_struct_unit(), 4 * 1024);
    json_struct_ptr->head = 0;
    json_struct_ptr->tail = 0;
    json_struct_ptr->stack = 0;
    json_struct_ptr->type = json_array;


    return json_struct_ptr;
}

HJSONSTRUCT create_json(enum e_json_value_type type)
{
    switch (type)
    {
    case json_object:
        return _json_create_object();
        break;
    case json_array:
        return _json_create_array();
        break;
    default:
        return 0;
    }
};


void destroy_json(HJSONSTRUCT json_struct_ptr)
{
    struct st_json_node* node_ptr = 0;
    struct st_json_node* node_stack_head = 0;
    struct st_json_node* free_node = 0;

    if (json_struct_ptr)
    {
        node_ptr = json_struct_ptr->head;
    }
    else
    {
        return;
    }

    while (node_ptr)
    {
        if (node_ptr->key)
        {
            default_memory_manager_free(node_ptr->key);
        }

        switch (node_ptr->type)
        {
        case json_null:
        {
            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_string:
        {
            default_memory_manager_free(node_ptr->value.str);

            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_integer:
        {
            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_float:
        {
            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_true:
        {
            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_false:
        {
            free_node = node_ptr;

            node_ptr = node_ptr->next;

            memory_unit_free(_default_json_node_unit(), free_node);
        }
        break;
        case json_object:
        {
            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        case json_array:
        {
            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        }

        if (node_ptr)
        {
        }
        else
        {
            for (;;)
            {
                node_ptr = node_stack_head;

                if (node_ptr)
                {
                    free_node = node_ptr;

                    node_stack_head = node_stack_head->stack;

                    if (node_ptr->next)
                    {
                        node_ptr = node_ptr->next;
                        memory_unit_free(_default_json_struct_unit(), free_node->value.struct_ptr);
                        memory_unit_free(_default_json_node_unit(), free_node);
                        break;
                    }
                    else
                    {
                        memory_unit_free(_default_json_struct_unit(), free_node->value.struct_ptr);
                        memory_unit_free(_default_json_node_unit(), free_node);
                    }
                }
                else
                    break;
            }
        }
    }

    memory_unit_free(_default_json_struct_unit(), json_struct_ptr);
}

void _json_struct_add_node(struct st_json_struct* json_struct_ptr, struct st_json_node* json_node_ptr)
{
    if (json_struct_ptr->tail)
    {
        json_struct_ptr->tail->next = json_node_ptr;
        json_struct_ptr->tail = json_node_ptr;
    }
    else
    {
        json_struct_ptr->head = json_node_ptr;
        json_struct_ptr->tail = json_node_ptr;
    }
}

HJSONNODE json_add_float(HJSONSTRUCT json_struct_ptr, double number, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_float;
    node->value.number_ex = number;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }
        node->key_length = key_length;
        node->key = (char*)default_memory_manager_alloc(node->key_length + 1);
        memcpy(node->key, key, node->key_length);
        node->key[node->key_length] = '\0';

    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONNODE json_add_integer(HJSONSTRUCT json_struct_ptr, long long number, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_integer;
    node->value.number = number;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }
        node->key = (char*)default_memory_manager_alloc(key_length + 1);
        node->key_length = escape_from(key, key_length, node->key);
        node->key[node->key_length] = '\0';

    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONNODE json_add_string(HJSONSTRUCT json_struct_ptr, const char* string, size_t string_length, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_string;

    node->value.str = (char*)default_memory_manager_alloc(string_length + 1);
    node->str_length = escape_from(string, string_length, node->value.str);
    node->value.str[node->str_length] = '\0';

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }
        node->key = (char*)default_memory_manager_alloc(key_length + 1);
        node->key_length = escape_from(key, key_length, node->key);
        node->key[node->key_length] = '\0';
    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONNODE json_add_true(HJSONSTRUCT json_struct_ptr, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_true;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }
        node->key = (char*)default_memory_manager_alloc(key_length + 1);
        node->key_length = escape_from(key, key_length, node->key);
        node->key[node->key_length] = '\0';
    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONNODE json_add_false(HJSONSTRUCT json_struct_ptr, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_false;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }

        node->key = (char*)default_memory_manager_alloc(key_length + 1);
        node->key_length = escape_from(key, key_length, node->key);
        node->key[node->key_length] = '\0';
    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONNODE json_add_null(HJSONSTRUCT json_struct_ptr, const char* key, size_t key_length)
{
    struct st_json_node* node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    node->next = 0;
    node->stack = 0;
    node->type = json_null;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_node_unit(), node);
            return 0;
        }
        node->key = (char*)default_memory_manager_alloc(key_length + 1);
        node->key_length = escape_from(key, key_length, node->key);
        node->key[node->key_length] = '\0';
    }
    break;
    case json_array:
    {
        node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_node_unit(), node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, node);

    return node;
}

HJSONSTRUCT json_add_struct(HJSONSTRUCT json_struct_ptr,
    enum e_json_value_type type, const char* key, size_t key_length)
{
    struct st_json_node* new_json_node = 0;

    new_json_node = (struct st_json_node*)memory_unit_alloc(_default_json_node_unit(), 4 * 1024);

    new_json_node->next = 0;
    new_json_node->stack = 0;
    switch (type)
    {
    case json_object:
    {
        new_json_node->type = json_object;
        new_json_node->value.struct_ptr = _json_create_object();
    }
    break;
    case json_array:
    {
        new_json_node->type = json_array;
        new_json_node->value.struct_ptr = _json_create_array();
    }
    break;
    }


    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (!key)
        {
            memory_unit_free(_default_json_struct_unit(), new_json_node->value.struct_ptr);
            memory_unit_free(_default_json_node_unit(), new_json_node);
            return 0;
        }
        new_json_node->key = (char*)default_memory_manager_alloc(key_length + 1);
        new_json_node->key_length = escape_from(key, key_length, new_json_node->key);
        new_json_node->key[new_json_node->key_length] = '\0';
    }
    break;
    case json_array:
    {
        new_json_node->key = 0;
    }
    break;
    default:
    {
        memory_unit_free(_default_json_struct_unit(), new_json_node->value.struct_ptr);
        memory_unit_free(_default_json_node_unit(), new_json_node);
        return 0;
    }
    }

    _json_struct_add_node(json_struct_ptr, new_json_node);

    return new_json_node->value.struct_ptr;
}

void json_to_string_ex(HJSONSTRUCT json_struct_ptr, json_string_append f, void* ud)
{
    char tmp_buf[64];
    size_t tmp_length = 0;
    struct st_json_node* node_ptr = json_struct_ptr->head;
    struct st_json_node* node_stack_head = 0;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        (*f)(ud, "{", 1);
    }
    break;
    case json_array:
    {
        (*f)(ud, "[", 1);
    }
    break;
    default:
    {
        CRUSH_CODE;
    }
    }

    node_ptr = json_struct_ptr->head;

    while (node_ptr)
    {
        if (node_ptr->key)
        {
            (*f)(ud, "\"", 1);
            escape_to_json_append(node_ptr->key, node_ptr->key_length, f, ud);
            (*f)(ud, "\":", 2);
        }

        switch (node_ptr->type)
        {
        case json_null:
        {
            (*f)(ud, "null", 4);

            node_ptr = node_ptr->next;
        }
        break;
        case json_string:
        {
            (*f)(ud, "\"", 1);
            escape_to_json_append(node_ptr->value.str, node_ptr->str_length, f, ud);
            (*f)(ud, "\"", 1);

            node_ptr = node_ptr->next;
        }
        break;
        case json_integer:
        {
            _i64toa_s(node_ptr->value.number, tmp_buf, sizeof(tmp_buf), 10);
            tmp_length = strlen(tmp_buf);

            (*f)(ud, tmp_buf, tmp_length);

            node_ptr = node_ptr->next;
        }
        break;
        case json_float:
        {
            sprintf_s(tmp_buf, sizeof(tmp_buf), "%f", node_ptr->value.number_ex);
            tmp_length = strlen(tmp_buf);

            (*f)(ud, tmp_buf, tmp_length);

            node_ptr = node_ptr->next;
        }
        break;
        case json_true:
        {
            (*f)(ud, "true", 4);

            node_ptr = node_ptr->next;
        }
        break;
        case json_false:
        {
            (*f)(ud, "false", 5);

            node_ptr = node_ptr->next;
        }
        break;
        case json_object:
        {
            (*f)(ud, "{", 1);

            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        case json_array:
        {
            (*f)(ud, "[", 1);

            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        }

        if (node_ptr)
        {
            (*f)(ud, ",", 1);
        }
        else
        {
            for (;;)
            {

                node_ptr = node_stack_head;

                if (node_ptr)
                {
                    node_stack_head = node_stack_head->stack;

                    switch (node_ptr->type)
                    {
                    case json_object:
                    {
                        (*f)(ud, "}", 1);
                    }
                    break;
                    case json_array:
                    {
                        (*f)(ud, "]", 1);
                    }
                    break;
                    default:
                    {
                        CRUSH_CODE;
                    }
                    }

                    if (node_ptr->next)
                    {
                        (*f)(ud, ",", 1);

                        node_ptr = node_ptr->next;
                        break;
                    }
                }
                else
                    break;
            }
        }
    }

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        (*f)(ud, "}", 1);
    }
    break;
    case json_array:
    {
        (*f)(ud, "]", 1);
    }
    break;
    default:
    {
        CRUSH_CODE;
    }
    }
}

size_t json_to_string(HJSONSTRUCT json_struct_ptr, char* json_str_ptr, size_t json_str_size)
{
    char tmp_buf[64];
    size_t tmp_length = 0;
    struct st_json_node* node_ptr = json_struct_ptr->head;
    struct st_json_node* node_stack_head = 0;
    char* current_pos = 0;
    char* end_pos = json_str_ptr + json_str_size;

    current_pos = json_str_ptr;

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (end_pos > current_pos)
        {
            memcpy(current_pos, "{", 1);
            current_pos++;
        }
        else
            return 0;
    }
    break;
    case json_array:
    {
        if (end_pos > current_pos)
        {
            memcpy(current_pos, "[", 1);
            current_pos++;
        }
        else
            return 0;
    }
    break;
    default:
    {
        CRUSH_CODE;
    }
    }

    node_ptr = json_struct_ptr->head;

    while (node_ptr)
    {
        if (node_ptr->key)
        {
            if (end_pos - current_pos > 3 + (int)node_ptr->key_length * 2)
            {
                *current_pos = '\"';
                current_pos++;
                current_pos += escape_to(node_ptr->key, node_ptr->key_length, current_pos);

                memcpy(current_pos, "\":", 2);
                current_pos += 2;
            }
            else
                return 0;
        }

        switch (node_ptr->type)
        {
        case json_null:
        {
            if (end_pos - current_pos > 4)
            {
                memcpy(current_pos, "null", 4);
                current_pos += 4;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_string:
        {
            if (end_pos - current_pos > 2 + (int)node_ptr->str_length * 2)
            {
                *current_pos = '\"';
                ++current_pos;
                current_pos += escape_to(node_ptr->value.str, node_ptr->str_length, current_pos);
                *current_pos = '\"';
                current_pos += 1;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_integer:
        {
            _i64toa_s(node_ptr->value.number, tmp_buf, sizeof(tmp_buf), 10);
            tmp_length = strlen(tmp_buf);

            if (end_pos - current_pos > (int)tmp_length)
            {
                memcpy(current_pos, tmp_buf, tmp_length);
                current_pos += tmp_length;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_float:
        {
            sprintf_s(tmp_buf, sizeof(tmp_buf), "%f", node_ptr->value.number_ex);
            tmp_length = strlen(tmp_buf);

            if (end_pos - current_pos > (int)tmp_length)
            {
                memcpy(current_pos, tmp_buf, tmp_length);
                current_pos += tmp_length;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_true:
        {
            if (end_pos - current_pos > 4)
            {
                memcpy(current_pos, "true", 4);
                current_pos += 4;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_false:
        {
            if (end_pos - current_pos > 5)
            {
                memcpy(current_pos, "false", 5);
                current_pos += 5;
            }
            else
                return 0;

            node_ptr = node_ptr->next;
        }
        break;
        case json_object:
        {
            if (end_pos - current_pos > 1)
            {
                memcpy(current_pos, "{", 1);
                current_pos++;
            }
            else
                return 0;

            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        case json_array:
        {
            if (end_pos - current_pos > 1)
            {
                memcpy(current_pos, "[", 1);
                current_pos++;
            }
            else
                return 0;

            node_ptr->stack = node_stack_head;
            node_stack_head = node_ptr;
            node_ptr = node_ptr->value.struct_ptr->head;

            if (node_ptr)
            {
                continue;
            }
        }
        break;
        }

        if (node_ptr)
        {
            if (end_pos - current_pos > 1)
            {
                memcpy(current_pos, ",", 1);
                current_pos++;
            }
            else
                return 0;
        }
        else
        {
            for (;;)
            {

                node_ptr = node_stack_head;

                if (node_ptr)
                {
                    node_stack_head = node_stack_head->stack;

                    switch (node_ptr->type)
                    {
                    case json_object:
                    {
                        if (end_pos - current_pos > 1)
                        {
                            memcpy(current_pos, "}", 1);
                            current_pos += 1;
                        }
                        else
                            return 0;
                    }
                    break;
                    case json_array:
                    {
                        if (end_pos - current_pos > 1)
                        {
                            memcpy(current_pos, "]", 1);
                            current_pos += 1;
                        }
                        else
                            return 0;
                    }
                    break;
                    default:
                    {
                        CRUSH_CODE;
                    }
                    }

                    if (node_ptr->next)
                    {
                        if (end_pos - current_pos > 1)
                        {
                            memcpy(current_pos, ",", 1);
                            current_pos += 1;
                        }
                        else
                            return 0;

                        node_ptr = node_ptr->next;
                        break;
                    }
                }
                else
                    break;
            }
        }
    }

    switch (json_struct_ptr->type)
    {
    case json_object:
    {
        if (end_pos - current_pos > 1)
        {
            memcpy(current_pos, "}", 1);
            current_pos++;
        }
        else
            return 0;
    }
    break;
    case json_array:
    {
        if (end_pos - current_pos > 1)
        {
            memcpy(current_pos, "]", 1);
            current_pos++;
        }
        else
            return 0;
    }
    break;
    default:
    {
        CRUSH_CODE;
    }
    }

    if (end_pos - current_pos > 1)
    {
        *current_pos = '\0';
    }

    return (current_pos - json_str_ptr);
}

bool step_number(const char** ptr)
{
    bool is_double = false;

    if (**ptr == '-') ++(*ptr);
    for (; ;)
    {
        switch (**ptr)
        {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
        {
            ++(*ptr);
            break;
        }
        case '.':
        {
            is_double = true;
            ++(*ptr);
            if (**ptr < '0' || **ptr > '9') return is_double;
            break;
        }
        default: return is_double;
        }
    }
}

void step_string(const char** ptr)
{
    for (; ;)
    {
        switch (**ptr)
        {
        case '\\':
        {
            ++(*ptr);
            ++(*ptr);
            break;
        }
        case '"':
        case '\0': return;
        default:
        {
            ++(*ptr);
            break;
        }
        }
    }
}

bool isdouble(const char* pDigit, size_t nSize)
{
    char* s = (char*)pDigit;
    char* e = (char*)pDigit + nSize;

    while (s < e)
    {
        if (*s++ == '.') return true;
    }

    return false;
}

bool step_key_value(const char** ptr, struct st_json_struct* json_struct_ptr, const char** key_ptr, size_t* key_length)
{
    bool is_double;
    const char* value_ptr = 0;
    size_t value_length = 0;

    for (;;)
    {
        *key_ptr = 0;
        *key_length = 0;
        value_ptr = 0;
        value_length = 0;

        TRIM_SPACE((*ptr));

        if (**ptr != '"')
        {
            return false;
        }
        else
            ++(*ptr);

        (*key_ptr) = (*ptr);
        step_string(ptr);
        *key_length = *ptr - *key_ptr;

        ++(*ptr);

        TRIM_SPACE((*ptr));

        if (**ptr != ':')
        {
            return false;
        }
        else
            ++(*ptr);

        TRIM_SPACE((*ptr));

        switch (**ptr)
        {
        case '\n': case '\r': case ' ': case '\f': case '\t': case '\v': case '\b':
        {
            ++(*ptr);
        }
        break;
        case '{':
        case '[':
        case '}':
        {
            return true;
        }
        break;
        case '"':
        {
            ++(*ptr);
            value_ptr = *ptr;
            step_string(ptr);
            value_length = *ptr - value_ptr;

            json_add_string(json_struct_ptr, value_ptr, value_length, *key_ptr, *key_length);

            ++(*ptr);
            TRIM_SPACE((*ptr));

            switch (**ptr)
            {
            case ',':
            {
                ++(*ptr);
            }
            break;
            case ']':
            case '}':
            {
                return true;
            }
            break;
            default:
                return false;
            }
        }
        break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': case '-':
        {
            value_ptr = *ptr;
            is_double = step_number(ptr);
            value_length = *ptr - value_ptr;

            if (is_double)
            {
                json_add_float(json_struct_ptr, atof(value_ptr), *key_ptr, *key_length);
            }
            else
            {
                json_add_integer(json_struct_ptr, _strtoi64(value_ptr, 0, 10), *key_ptr, *key_length);
            }

            TRIM_SPACE((*ptr));

            switch (**ptr)
            {
            case ',':
            {
                ++(*ptr);
            }
            break;
            case ']':
            case '}':
            {
                return true;
            }
            break;
            default:
                return false;
            }
        }
        break;
        case 't':
        {
            if (!strncmp(*ptr, "true", 4))
            {
                (*ptr) += 4;
                json_add_true(json_struct_ptr, *key_ptr, *key_length);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        case 'f':
        {
            if (!strncmp(*ptr, "false", 5))
            {
                (*ptr) += 5;
                json_add_false(json_struct_ptr, *key_ptr, *key_length);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        case 'n':
        {
            if (!strncmp(*ptr, "null", 4))
            {
                (*ptr) += 4;
                json_add_null(json_struct_ptr, *key_ptr, *key_length);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        default:
        {
            return false;
        }
        }
    }
}

bool step_value(const char** ptr, struct st_json_struct* json_struct_ptr)
{
    bool is_double;
    const char* value_ptr = 0;
    size_t value_length = 0;

    for (;;)
    {
        value_ptr = 0;
        value_length = 0;

        switch (**ptr)
        {
        case '\n': case '\r': case ' ': case '\f': case '\t': case '\v': case '\b':
        {
            ++(*ptr);
        }
        break;
        case '{':
        case '[':
        case ']':
        {
            return true;
        }
        break;
        case '"':
        {
            ++(*ptr);
            value_ptr = *ptr;
            step_string(ptr);
            value_length = *ptr - value_ptr;

            json_add_string(json_struct_ptr, value_ptr, value_length, 0, 0);

            ++(*ptr);
            TRIM_SPACE((*ptr));

            switch (**ptr)
            {
            case ',':
            {
                ++(*ptr);
            }
            break;
            case ']':
            case '}':
            {
                return true;
            }
            break;
            default:
                return false;
            }
        }
        break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': case '-':
        {
            value_ptr = *ptr;
            is_double = step_number(ptr);
            value_length = *ptr - value_ptr;

            if (is_double)
            {
                json_add_float(json_struct_ptr, atof(value_ptr), 0, 0);
            }
            else
            {
                json_add_integer(json_struct_ptr, _strtoi64(value_ptr, 0, 10), 0, 0);
            }

            TRIM_SPACE((*ptr));

            switch (**ptr)
            {
            case ',':
            {
                ++(*ptr);
            }
            break;
            case ']':
            case '}':
            {
                return true;
            }
            break;
            default:
                return false;
            }
        }
        break;
        case 't':
        {
            if (!strncmp(*ptr, "true", 4))
            {
                (*ptr) += 4;
                json_add_true(json_struct_ptr, 0, 0);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        case 'f':
        {
            if (!strncmp(*ptr, "false", 5))
            {
                (*ptr) += 5;
                json_add_false(json_struct_ptr, 0, 0);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        case 'n':
        {
            if (!strncmp(*ptr, "null", 4))
            {
                (*ptr) += 4;
                json_add_null(json_struct_ptr, 0, 0);

                TRIM_SPACE((*ptr));

                switch (**ptr)
                {
                case ',':
                {
                    ++(*ptr);
                }
                break;
                case ']':
                case '}':
                {
                    return true;
                }
                break;
                default:
                    return false;
                }
            }
            else
                return false;
        }
        break;
        default:
        {
            return false;
        }
        }
    }
}

void json_log_error(const char* json_str_begin, const char* current_ptr, char* err_msg, size_t err_msg_size)
{
    const char* err_begin;
    const char* err_end;

    if (err_msg && err_msg_size >= 64)
    {
        err_begin = current_ptr;
        err_end = current_ptr;

        while (current_ptr - err_begin < 10)
        {
            if (err_begin == json_str_begin)
            {
                break;
            }
            else
            {
                err_begin--;
            }
        }

        while (err_end - current_ptr < 10)
        {
            if (*err_end == '\0')
            {
                break;
            }
            else
            {
                err_end++;
            }
        }

        //*err_msg = (char*)(*json_alloc)(json_ud, 0, 64);

        memcpy(err_msg, "json error near: ", strlen("json error near: "));
        memcpy(err_msg + strlen("json error near: "), err_begin, err_end - err_begin);
        (err_msg)[strlen("json error near: ") + (err_end - err_begin)] = '\'';
        (err_msg)[strlen("json error near: ") + (err_end - err_begin) + 1] = '\0';
    }
}

HJSONSTRUCT string_to_json(const char* str, char* err_info, size_t err_info_len)
{
    struct st_json_struct* current_json_struct = 0;
    const char* current_ptr = str;
    struct st_json_struct* json_struct_stack_head = 0;

    struct st_json_struct* json_struct_root = 0;

    const char* key_ptr = 0;
    size_t key_length = 0;

    for (;;)
    {
        switch (*current_ptr)
        {
        case '\n': case '\r': case ' ': case '\f': case '\t': case '\v':
        {
            ++current_ptr;
        }
        break;
        case '[':
        {
            if (current_json_struct)
            {
                current_json_struct->stack = json_struct_stack_head;
                json_struct_stack_head = current_json_struct;

                if (current_json_struct->type == json_object)
                {
                    current_json_struct = json_add_struct(current_json_struct, json_array, key_ptr, key_length);
                }
                else
                {
                    current_json_struct = json_add_struct(current_json_struct, json_array, 0, 0);
                }
            }
            else
            {
                current_json_struct = _json_create_array();
                json_struct_root = current_json_struct;
            }

            ++current_ptr;

            TRIM_SPACE(current_ptr);

            if (*current_ptr != ']')
            {
                if (!step_value(&current_ptr, current_json_struct))
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
            }
        }
        break;
        case '{':
        {
            if (current_json_struct)
            {
                current_json_struct->stack = json_struct_stack_head;
                json_struct_stack_head = current_json_struct;

                if (current_json_struct->type == json_object)
                {
                    current_json_struct = json_add_struct(current_json_struct, json_object, key_ptr, key_length);
                }
                else
                {
                    current_json_struct = json_add_struct(current_json_struct, json_object, 0, 0);
                }
            }
            else
            {
                current_json_struct = _json_create_object();
                json_struct_root = current_json_struct;
            }

            ++current_ptr;

            TRIM_SPACE(current_ptr);

            if (*current_ptr != '}')
            {
                if (!step_key_value(&current_ptr, current_json_struct, &key_ptr, &key_length))
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
            }
        }
        break;
        case ']':
        {
            if (current_json_struct->type != json_array)
            {
                if (err_info && err_info_len)
                {
                    json_log_error(str, current_ptr, err_info, err_info_len);
                }
                destroy_json(json_struct_root);
                return 0;
            }

            ++current_ptr;

            if (json_struct_stack_head)
            {
                current_json_struct = json_struct_stack_head;
                json_struct_stack_head = json_struct_stack_head->stack;

                TRIM_SPACE(current_ptr);
                switch (*current_ptr)
                {
                case ',':
                {
                    ++current_ptr;

                    switch (current_json_struct->type)
                    {
                    case json_object:
                    {
                        if (!step_key_value(&current_ptr, current_json_struct, &key_ptr, &key_length))
                        {
                            if (err_info && err_info_len)
                            {
                                json_log_error(str, current_ptr, err_info, err_info_len);
                            }
                            destroy_json(json_struct_root);
                            return 0;
                        }
                    }
                    break;
                    case json_array:
                    {
                        if (!step_value(&current_ptr, current_json_struct))
                        {
                            if (err_info && err_info_len)
                            {
                                json_log_error(str, current_ptr, err_info, err_info_len);
                            }
                            destroy_json(json_struct_root);
                            return 0;
                        }
                    }
                    break;
                    default:
                    {
                        CRUSH_CODE;
                    }
                    }
                }
                break;
                case ']':
                case '}':
                {
                }
                break;
                default:
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
                }
            }
            else
            {
                TRIM_SPACE(current_ptr);
                if (*current_ptr != '\0')
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
                return json_struct_root;
            }
        }
        break;
        case '}':
        {
            if (current_json_struct->type != json_object)
            {
                if (err_info && err_info_len)
                {
                    json_log_error(str, current_ptr, err_info, err_info_len);
                }
                destroy_json(json_struct_root);
                return 0;
            }

            ++current_ptr;

            if (json_struct_stack_head)
            {
                current_json_struct = json_struct_stack_head;
                json_struct_stack_head = json_struct_stack_head->stack;
                TRIM_SPACE(current_ptr);

                switch (*current_ptr)
                {
                case ',':
                {
                    ++current_ptr;

                    switch (current_json_struct->type)
                    {
                    case json_object:
                    {
                        if (!step_key_value(&current_ptr, current_json_struct, &key_ptr, &key_length))
                        {
                            if (err_info && err_info_len)
                            {
                                json_log_error(str, current_ptr, err_info, err_info_len);
                            }
                            destroy_json(json_struct_root);
                            return 0;
                        }
                    }
                    break;
                    case json_array:
                    {
                        if (!step_value(&current_ptr, current_json_struct))
                        {
                            if (err_info && err_info_len)
                            {
                                json_log_error(str, current_ptr, err_info, err_info_len);
                            }
                            destroy_json(json_struct_root);
                            return 0;
                        }
                    }
                    break;
                    default:
                    {
                        CRUSH_CODE;
                    }
                    }
                }
                break;
                case ']':
                case '}':
                {
                }
                break;
                default:
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
                }
            }
            else
            {
                TRIM_SPACE(current_ptr);
                if (*current_ptr != '\0')
                {
                    if (err_info && err_info_len)
                    {
                        json_log_error(str, current_ptr, err_info, err_info_len);
                    }
                    destroy_json(json_struct_root);
                    return 0;
                }
                return json_struct_root;
            }

        }
        break;
        default:
        {
            if (err_info && err_info_len)
            {
                json_log_error(str, current_ptr, err_info, err_info_len);
            }
            destroy_json(json_struct_root);
            return 0;
        }
        }
    }
}

HJSONNODE json_struct_first_node(HJSONSTRUCT json_struct)
{
    return json_struct->head;
}

HJSONNODE json_struct_next_node(HJSONNODE json_node)
{
    return json_node->next;
}

const char* json_key(HJSONNODE json_node)
{
    return json_node->key;
}

bool json_value_integer(HJSONNODE json_node, long long* value)
{
    if (json_node->type == json_integer)
    {
        *value = json_node->value.number;
        return true;
    }

    return false;
}

bool json_value_float(HJSONNODE json_node, double* value)
{
    if (json_node->type == json_float)
    {
        *value = json_node->value.number_ex;
        return true;
    }

    return false;
}

bool json_value_string(HJSONNODE json_node, const char** value, size_t* length)
{
    if (json_node->type == json_string)
    {
        *value = json_node->value.str;
        *length = json_node->str_length;
        return true;
    }

    return false;
}

bool json_value_true(HJSONNODE json_node)
{
    if (json_node->type == json_true)
    {
        return true;
    }

    return false;
}

bool json_value_false(HJSONNODE json_node)
{
    if (json_node->type == json_false)
    {
        return true;
    }

    return false;
}

bool json_value_null(HJSONNODE json_node)
{
    if (json_node->type == json_null)
    {
        return true;
    }

    return false;
}

HJSONSTRUCT json_value_struct(HJSONNODE json_node)
{
    switch (json_node->type)
    {
    case json_object:
    case json_array:
    {
        return json_node->value.struct_ptr;
    }
    break;
    default:
        return 0;
    }
}

enum e_json_value_type json_value_type(HJSONNODE json_node)
{
    return json_node->type;
}

enum e_json_value_type json_struct_type(HJSONSTRUCT json_struct)
{
    return json_struct->type;
}
