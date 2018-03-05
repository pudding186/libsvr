#pragma once
#include <crtdefs.h>

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed long long    INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long long  UINT64, *PUINT64;

enum e_json_value_type
{
    json_null = 0,
    json_string,
    json_integer,
    json_float,
    json_object,
    json_array,
    json_true,
    json_false,
};

typedef void * (*libsvr_alloc) (void *ud, void *ptr, size_t osize, size_t nsize);

typedef ptrdiff_t(*key_cmp)(void*, void*);

#ifndef __cplusplus
typedef unsigned char   bool;
#define true 1
#define false 0

extern void* (default_memory_manager_realloc)(void* old_mem, size_t mem_size);

extern void* (default_memory_manager_alloc)(size_t mem_size);

extern void (default_memory_manager_free)(void* mem);

extern bool (is_valid_ptr_in_default_manager)(void* mem);

#else

extern "C" void* (default_memory_manager_realloc)(void* old_mem, size_t mem_size);

extern "C" void* (default_memory_manager_alloc)(size_t mem_size);

extern "C" void (default_memory_manager_free)(void* mem);

extern "C" bool (is_valid_ptr_in_default_manager)(void* mem);

#endif


