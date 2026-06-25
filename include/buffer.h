#ifndef BUFFER_H
#define BUFFER_H

#ifdef __cplusplus
  #define BUFFERDEF extern "C"
#else
  #define BUFFERDEF extern
#endif

#include <stddef.h>
#include <stdbool.h>

#define DECL_BUFFER(T, TName, SIZE) \
  typedef struct {                  \
    T items[SIZE];                  \
    size_t len;                     \
  } Buffer_##TName##_##SIZE;

#define Buffer(TName, S) Buffer_##TName##_##S

typedef struct {
  void** items;
  const size_t elem_size;
  const size_t nmemb;
  size_t* len;
} BufferGeneric;

#define BUF_TO_GENERIC(buffer) \
  (BufferGeneric){.items = (void**)(&(buffer).items), \
                  .elem_size = sizeof(*(buffer).items), \
                  .nmemb = sizeof((buffer).items) / sizeof(*(buffer).items), \
                  .len = &(buffer).len,}

#define buf_push(buffer, ...) \
  __extension__({ \
    __typeof__(*(buffer).items) _arr[] = {__VA_ARGS__}; \
    _buf_push(BUF_TO_GENERIC((buffer)), _arr, sizeof(_arr)/sizeof(*_arr)); \
  })

BUFFERDEF bool _buf_push(BufferGeneric buf, const void* values, size_t count);

#endif /* BUFFER_H */
