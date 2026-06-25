/* SPDX-License-Identifier: GPL-3.0-only */
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

#define MAKE_ERR(ecode, emsg) \
  ((Error){.code=(ecode), .message=(emsg)})

/*
  DISCLAIMER: LT, RT or T is a type name so this means
  if one of them is pointer, you have to use typedef of it.
  For example: DECL_EITHER(int*, bool) won't work
  But, DECL_EITHER(int_ptr, bool) will work
  This pattern is everywhere I put generic type like "T"
*/

/*
  Either(L, R) holds 2 different typed value
*/
#define Either(LT, RT) Either_##LT##_##RT

#define DECL_EITHER(LT, RT) \
  typedef struct {                              \
    bool is_left;                               \
    union {                                     \
      LT left;                               \
      RT right;                              \
    } as;                                       \
  } Either(LT, RT);

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
#define Result(T) Either(T, Error)
#define DECL_RESULT(T) DECL_EITHER(T, Error)

/* Producing Result objects for returns */
#define RES_OK(T, val)                      \
  EITHER_L(T, Error, (val))
#define RES_ERR_MSG(T, errcode, errmsg)         \
  EITHER_R(T, Error, MAKE_ERR(errcode, errmsg))
#define RES_ERR(T, errcode)         \
  EITHER_R(T, Error, MAKE_ERR(errcode, NULL))

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
#define Option(T) Option_##T

#define DECL_OPTION(T)               \
  typedef struct {                              \
    bool is_some;                               \
    T value;                                \
  } Option(T);

/* Producing Option structs for returns */
#define OPT_SOME(T, val)                     \
  ((Option(T)){ .is_some = true, .value = (val) })
#define OPT_NONE(T)                         \
  ((Option(T)){ .is_some = false })

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

#endif /* EITHER_H */
