#ifndef EITHER_H
#define EITHER_H

#ifdef _MSC_VER
  #error "This header does not support MSVC, please don't use garbage slop compilers."
#endif

/*
  Rust's Result<T, E> implementation in C.
  This library DOES NOT DEPEND ANYTHING EXCEPT LIBC!
  This header supposed to be used ONLY FOR C!
  DO NOT USE RAW POINTERS IN EITHER/OPTION/RESULT NAME PARAMS!
  Instead, use (for int*) int_ptr or if you have a special pointer type
  define typedef of it. Because something like this: Result(int*)
  gives some errors. But this is safe: Result(int_ptr)
  And it comes with some tools for development like TODO or UNREACHABLE macros
*/

#ifdef __cplusplus
  #define EITHERDEF extern "C"
#else
  #define EITHERDEF extern
#endif

#include <stddef.h>
#include <stdbool.h>

#define ERROR_CODES \
  X(INVALID_ARG, "Invalid argument") \
  X(NOT_FOUND, "Not found") \
  X(VALIDATION_FAILED, "Validation failed") \
  X(INTERNAL, "Internal error") \
  X(NULL_PTR, "Null pointer") \
  X(NO_SUCH_FILE, "No such file or directory") \
  X(MEM_CORRUPT, "Memory is corrupted")

/* Error Code enum, you can add or change here */
typedef enum {
  ERR_NOERR = 0,
#define X(name, _) ERR_##name,
  ERROR_CODES
#undef X
  _ErrorCode_count,
} ErrorCode;

typedef struct {
  ErrorCode code;
  const char* message;
} Error;

EITHERDEF const char* err_tostr(ErrorCode err_code);

/* TODO macro, use this for not-implemented features */
#define TODO(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: TODO: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS); \
    abort(); \
  } while (0)

/* UNREACHABLE macro, use it for regions that are not meant to be reached */
#define UNREACHABLE(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: UNREACHABLE: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    __builtin_unreachable(); \
  } while(0)

#define STRINGIFY(macro) #macro

#define MAKE_ERR(ecode, emsg) \
  ((Error){.code=(ecode), .message=(emsg)})

/* Either(L, R) holds 2 different typed value */
#define Either(LName, RName) Either_##LName##_##RName

#define DECL_EITHER(LType, LName, RType, RName) \
  typedef struct {                              \
    bool is_left;                               \
    union {                                     \
      LType left;                               \
      RType right;                              \
    } as;                                       \
  } Either(LName, RName);

/* Producing Either structs for returns at functions */
#define EITHER_L(LName, RName, lval)                          \
  (Either(LName, RName)){ .is_left = 1, .as.left = (lval) }
#define EITHER_R(LName, RName, rval)                            \
  (Either(LName, RName)){ .is_left = 0, .as.right = (rval) }

/* Unwrap mechanism to get R (right) and L (left) */
#define EITHER_GETL(either)                     \
  ((either).as.left)
#define EITHER_GETR(either)                     \
  ((either).as.right)

/* we have already is_left field to query value is left or right */

/* Result(T) aka Either(T, Error) (Wrapper of Either) */
#define Result(TName) Either(TName, Error)
#define DECL_RESULT(T, TName) DECL_EITHER(T, TName, Error, Error)

/* Producing Result objects for returns */
#define RES_OK(TName, val)                      \
  EITHER_L(TName, Error, (val))
#define RES_ERR_MSG(TName, errcode, errmsg)         \
  EITHER_R(TName, Error, MAKE_ERR(errcode, errmsg))
#define RES_ERR(TName, errcode)         \
  EITHER_R(TName, Error, MAKE_ERR(errcode, NULL))

/* Unwraps blindly, just unwrap */
#define RES_UNWRAP(res)                         \
  ((res).as.left)

/* Left: Value, Right: Error */
#define RES_ISERR(res)                          \
  (!(res).is_left)

/* Get error without asking */
#define RES_GETE(res) \
  (EITHER_GETR((res)))

/* Option(T) */
#define Option(TName) Option_##TName

#define DECL_OPTION(TType, TName)               \
  typedef struct {                              \
    bool is_some;                               \
    TType value;                                \
  } Option(TName);

/* Producing Option structs for returns */
#define OPT_SOME(TName, val)                     \
  ((Option(TName)){ .is_some = true, .value = (val) })
#define OPT_NONE(TName)                         \
  ((Option(TName)){ .is_some = false })

/* Unwrap blindly */
#define OPT_UNWRAP(opt)                         \
  ((opt).value)

/* we have already is_some to query the value is either Some or None */

/*
  like Option<void>,
  instead of using Option<bool>, use this
  (if bool is just an error state)
*/
typedef struct {
  bool is_error;
  Error error;
} ErrorOrNot;

#define EON_SUCCESS ((ErrorOrNot){ false, { ERR_NOERR, NULL } })
#define EON_ERROR(EEnum, EMessage) ((ErrorOrNot){ true, { .code=EEnum, .message=EMessage } })

// IMPLEMENTATION BEGIN
#ifdef EITHER_IMPLEMENTATION
const char* err_tostr(ErrorCode err_code) {
  switch(err_code) {
#define X(name, msg) case ERR_##name: return msg;
ERROR_CODES
#undef X
  default: UNREACHABLE("err_tostr");
  }
}
#endif // EITHER_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* EITHER_H */

/*
  The MIT License
  Copyright (c) 2026 ilpeN

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
