#ifndef SLIBS_H
#define SLIBS_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma region Miscellaneous

#define sl_auto(name, x) typeof(x) name = x

#define sl_new(type, ...)                                                      \
  ({                                                                           \
    type *ptr = malloc(sizeof(type));                                          \
    *ptr = (type){__VA_ARGS__};                                                \
    ptr;                                                                       \
  })

#define sl_init(type, ...)                                                     \
  (type) { __VA_ARGS__ }

#define sl_array_len(arr) sizeof(arr) / sizeof(arr[0])

#define sl_fmt_spec(arg)                                                       \
  _Generic((arg),                                                              \
      int8_t: "%d",                                                            \
      int16_t: "%d",                                                           \
      int32_t: "%d",                                                           \
      int64_t: "%lld",                                                         \
      uint8_t: "%u",                                                           \
      uint16_t: "%u",                                                          \
      uint32_t: "%lu",                                                         \
      uint64_t: "%llu",                                                        \
      double: "%lf",                                                           \
      float: "%f",                                                             \
      char: "%c",                                                              \
      char *: "%s",                                                            \
      void *: "%p",                                                            \
      default: "Unknown")

#define sl_stringify(x) #x

#pragma endregion
#pragma region Vector

#define sl_vec(type)                                                           \
  struct {                                                                     \
    type *data;                                                                \
    size_t size;                                                               \
    size_t capacity;                                                           \
  }

#define sl_vec_grow(vec)                                                       \
  {                                                                            \
    (vec).capacity = (vec).capacity * 2 + 1;                                   \
    void *ptr = realloc((vec).data, (vec).capacity * sizeof(*(vec).data));     \
    if (ptr)                                                                   \
      (vec).data = ptr;                                                        \
  }

#define sl_vec_push(vec, element)                                              \
  {                                                                            \
    if ((vec).size >= (vec).capacity)                                          \
      sl_vec_grow(vec);                                                        \
    (vec).data[(vec).size++] = (element);                                      \
  }

#define sl_vec_shift(vec, element)                                             \
  {                                                                            \
    if ((vec).size >= (vec).capacity)                                          \
      sl_vec_grow(vec);                                                        \
    memmove((vec).data + 1, (vec).data, (vec).size * sizeof(*(vec).data));     \
    (vec).data[0] = (element);                                                 \
    (vec).size++;                                                              \
  }

#define sl_vec_pop(vec)                                                        \
  {                                                                            \
    if ((vec).size > 0) {                                                      \
      (vec).size--;                                                            \
    }                                                                          \
  }

#define sl_vec_at(vec, index) ((vec).data[index])

#define sl_vec_size(vec) ((vec).size)

#define sl_vec_capacity(vec) ((vec).capacity)

#define sl_vec_free(vec) free((vec).data)

#define sl_vec_begin(vec) (vec).data

#define sl_vec_end(vec) ((vec).data + (vec).size)

#define sl_vec_it(name, vec)                                                   \
  sl_auto((name), sl_vec_begin(vec));                                          \
  (name) != sl_vec_end(vec);                                                   \
  ++(name)

#pragma endregion
#pragma region String

typedef sl_vec(char) sl_string;

#define sl_string(c_str)                                                       \
  ({                                                                           \
    sl_string str = {0};                                                       \
    for (size_t i = 0; i < strlen((c_str)); i++)                               \
      sl_vec_push(str, (c_str)[i]);                                            \
    str;                                                                       \
  })

#define sl_tostring(val)                                                       \
  ({                                                                           \
    sl_auto(len, snprintf(NULL, 0, sl_fmt_spec(val), val) + 1);                \
    sl_auto(buf, (char *)malloc(len));                                         \
    snprintf(buf, len, sl_fmt_spec(val), val);                                 \
    sl_auto(str, sl_string(buf));                                              \
    free(buf);                                                                 \
    str;                                                                       \
  })

#define sl_str_free(str) sl_vec_free(str)

#define sl_c_str(str)                                                          \
  ({                                                                           \
    sl_vec_push((str), '\0');                                                  \
    (str).size--;                                                              \
    (str).data;                                                                \
  })

#pragma endregion
#pragma region Pointers

#define sl_ptr(type)                                                           \
  struct {                                                                     \
    type *ptr;                                                                 \
    int ref_count;                                                             \
  }

#define sl_ptr_make(raw_ptr)                                                   \
  { raw_ptr, 1 }

#define sl_ptr_release(smart_ptr)                                              \
  ({                                                                           \
    smart_ptr.ref_count--;                                                     \
    if (smart_ptr.ref_count <= 0) {                                            \
      free(smart_ptr.ptr);                                                     \
    }                                                                          \
  })

#define sl_ptr_get(smart_ptr, raw_ptr_name, scope)                             \
  ({                                                                           \
    assert(smart_ptr.ref_count > 0 && "Smart pointer already released!");      \
    sl_auto(raw_ptr_name, smart_ptr.ptr);                                      \
    smart_ptr.ref_count++;                                                     \
    scope;                                                                     \
    sl_ptr_release(smart_ptr);                                                 \
  });

#define sl_ptr_scope(smart_ptr, scope)                                         \
  ({                                                                           \
    scope;                                                                     \
    sl_ptr_release(smart_ptr);                                                 \
  });

void sl_read_file(const char *filename, sl_string *buffer) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: could not open file %s\n", filename);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  for (size_t i = 0; i < file_size; i++) {
    sl_vec_push(*buffer, fgetc(file));
  }

  fclose(file);
}

#pragma endregion

#endif // SLIBS_H
