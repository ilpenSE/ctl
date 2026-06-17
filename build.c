#ifdef _MSC_VER
  #error "This library does not support MSVC, please don't use garbage slop compilers."
#endif

#define FUTIL_IMPLEMENTATION
#define SV_IMPLEMENTATION
#define STR_IMPLEMENTATION
#define EITHER_IMPLEMENTATION
#define ARRAY_IMPLEMENTATION
#define BUIC_IMPLEMENTATION

#include "futil.h"
#include "sv.h"
#include "str.h"
#include "either.h"
#include "array.h"
#include "buic.h"
