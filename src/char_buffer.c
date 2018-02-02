#include "../include/type_def.h"
#include "./data_def.h"
#include "../include/memory_pool.h"
#include "../include/char_buffer.h"
#include <memory.h>

#define CRUSH_CODE char* p = 0;*p = 'a';

__declspec(thread) HMEMORYUNIT def_char_segment_unit = 0;
__declspec(thread) HMEMORYUNIT def_char_buffer_unit = 0;

inline HMEMORYUNIT _default_char_segment_unit(void)
{
    return def_char_segment_unit;
}

inline HMEMORYUNIT _default_char_buffer_unit(void)
{
    return def_char_buffer_unit;
}

HCHARBUFFER create_char_buffer( void )
{
    HCHARBUFFER buffer = (HCHARBUFFER)memory_unit_alloc(_default_char_buffer_unit(), 4096);

    buffer->head = 0;
    buffer->tail = 0;
    buffer->buffer_capacity = MAX_CHAR_SEGMENT_SIZE;
    buffer->buffer_ptr = buffer->default_buffer;
    buffer->buffer_use_size = 0;
	buffer->char_segment_unit = _default_char_segment_unit();

    return buffer;
}

void char_buffer_resize(HCHARBUFFER buffer, size_t size)
{
    if (size > buffer->buffer_capacity)
    {
        if (buffer->buffer_ptr == buffer->default_buffer)
        {
            buffer->buffer_ptr = (char*)default_memory_manager_alloc(size);

            memcpy(buffer->buffer_ptr, buffer->default_buffer, buffer->buffer_use_size);
        }
        else
        {
            buffer->buffer_ptr = (char*)default_memory_manager_realloc(buffer->buffer_ptr, size);
        }

        buffer->buffer_capacity = size;
    }
}

void char_buffer_append(HCHARBUFFER buffer, const char* data, size_t length)
{
    size_t left;

    if (!buffer->head)
    {
        if (buffer->buffer_use_size + length <= buffer->buffer_capacity)
        {
            memcpy(buffer->buffer_ptr + buffer->buffer_use_size, data, length);
            buffer->buffer_use_size += length;
            return;
        }
        else
        {
            left = buffer->buffer_capacity - buffer->buffer_use_size;
            memcpy(buffer->buffer_ptr + buffer->buffer_use_size, data, left);
            buffer->buffer_use_size += left;
            data += left;
            length -= left;

			buffer->head = (char_segment*)memory_unit_alloc(buffer->char_segment_unit, 4096);
            buffer->tail = buffer->head;
            buffer->head->next = 0;
            buffer->head->use_size = 0;
        }
    }

    if (length + buffer->tail->use_size <= MAX_CHAR_SEGMENT_SIZE)
    {
        memcpy(buffer->tail->data + buffer->tail->use_size, data, length);
        buffer->tail->use_size += length;
        return;
    }
    else
    {
        left = MAX_CHAR_SEGMENT_SIZE - buffer->tail->use_size;
        memcpy(buffer->tail->data + buffer->tail->use_size, data, left);
        buffer->tail->use_size = MAX_CHAR_SEGMENT_SIZE;
        data += left;
        length -= left;
    }

    for (;;)
    {
        char_segment* seg = (char_segment*)memory_unit_alloc(buffer->char_segment_unit, 4096);
        buffer->tail->next = seg;
        buffer->tail = seg;
        seg->next = 0;

        if (length <= MAX_CHAR_SEGMENT_SIZE)
        {
            memcpy(seg->data, data, length);
            seg->use_size = length;
            return;
        }
        else
        {
            memcpy(seg->data, data, MAX_CHAR_SEGMENT_SIZE);
            seg->use_size = MAX_CHAR_SEGMENT_SIZE;
            data += MAX_CHAR_SEGMENT_SIZE;
            length -= MAX_CHAR_SEGMENT_SIZE;
        }
    }
}

const char* char_buffer_c_str(HCHARBUFFER buffer)
{
    size_t total_data_size = buffer->buffer_use_size;

    char_segment* seg = buffer->head;

    while (seg)
    {
        if (seg->next)
        {
            if (seg->use_size != MAX_CHAR_SEGMENT_SIZE)
            {
                CRUSH_CODE;
            }
        }
        total_data_size += seg->use_size;
        seg = seg->next;
    }

    char_buffer_resize(buffer, total_data_size);

    seg = buffer->head;

    while (seg)
    {
        char_segment* del_seg = seg;
        if (seg->next)
        {
            if (seg->use_size != MAX_CHAR_SEGMENT_SIZE)
            {
                CRUSH_CODE;
            }
        }

        memcpy(buffer->buffer_ptr + buffer->buffer_use_size, seg->data, seg->use_size);
        buffer->buffer_use_size += seg->use_size;
        
        seg = seg->next;

        memory_unit_free(buffer->char_segment_unit, del_seg);
    }

    buffer->tail = 0;
    buffer->head = 0;

    return buffer->buffer_ptr;
}

size_t char_buffer_c_str_length(HCHARBUFFER buffer)
{
    if (buffer->head)
    {
        char_buffer_c_str(buffer);
    }

    return buffer->buffer_use_size;
}


void destroy_char_buffer(HCHARBUFFER buffer)
{
    char_segment* seg = buffer->head;

    while (seg)
    {
        char_segment* del_seg = seg;
        seg = seg->next;
        memory_unit_free(buffer->char_segment_unit, del_seg);
    }

    if (buffer->buffer_ptr != buffer->default_buffer)
    {
        default_memory_manager_free(buffer->buffer_ptr);
    }

    memory_unit_free(_default_char_buffer_unit(), buffer);
}
