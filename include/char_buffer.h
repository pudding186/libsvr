#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_char_buffer* HCHARBUFFER;

extern HCHARBUFFER (create_char_buffer)(void);
extern void (char_buffer_resize)(HCHARBUFFER buffer, size_t size);
extern void (char_buffer_append)(HCHARBUFFER buffer, const char* data, size_t length);
extern const char* (char_buffer_c_str)(HCHARBUFFER buffer);
extern size_t (char_buffer_c_str_length)(HCHARBUFFER buffer);
extern void (destroy_char_buffer)(HCHARBUFFER buffer);

#ifdef  __cplusplus
}
#endif