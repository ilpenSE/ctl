#include <basic.h>
#include <array.h>
#include <either.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// Temp
static char _basic_temp[BASIC_TEMP_BUFFER_SIZE];
static size_t _basic_temp_len = 0;

char* temp_sprintf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* result = temp_vsprintf(fmt, args);
  va_end(args);
  return result;
}

char* temp_vsprintf(const char* fmt, va_list args) {
  int n = vsnprintf(_basic_temp, sizeof(_basic_temp), fmt, args);
  if (n < 0) return NULL;
  _basic_temp_len = clamp((size_t)n, 0, sizeof(_basic_temp) - 1);
  return _basic_temp;
}

size_t temp_len() {
  return _basic_temp_len;
}

void temp_reset() {
  _basic_temp[0] = '\0';
  _basic_temp_len = 0;
}

void temp_clear() {
  memset(_basic_temp, 0, sizeof(_basic_temp));
  _basic_temp_len = 0;
}

// Args Parsing
const char* argv_shift(int* argc, char*** argv) {
  if (*argc < 1) return NULL;
  *argc -= 1;
  return *(*argv)++;
}

// Math
#define X(TName, T) \
  T clamp##TName(T a, T lower, T upper) { \
    return a > upper ? upper : a < lower ? lower : a; \
  }
_BASIC_TYPES_WITH_BITS(X)
#undef X

int powii(int base, int exp) {
  int result = 1;
  while (exp > 0) {
    if (exp & 1)
    result *= base;
    base *= base;
    exp >>= 1;
  }
  return result;
}
