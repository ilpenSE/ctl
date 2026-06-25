/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef STR_H
#define STR_H

#ifdef __cplusplus
  #define STRDEF extern "C"
#else
  #define STRDEF extern
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "array.h"
#include "basic.h"

#define str_foreach(s, name) for(char* name = (s).data; name < (s).data + (s).len; name++)

/*
  Functions that take String context
  as first argument and pointer DO NOT CHECK NULL CONTEXT!
  Like: bool str_reserve(NULL, 10); RESULTS WITH UNDEFINED BEHAVIOR!
  It's your responsibility to check them
*/
/*
  Converts StringView into heap-allocated String
  Allocates memory for String, copies StringView's data and length
*/
#define str_from_sv(sv, ...) \
  str_from_sv_impl(sv, (StringMemory){__VA_ARGS__})
STRDEF String str_from_sv_impl(const StringView* sv, StringMemory memory);

/*
  Makes and returns string struct from C-strings with length
  If you have len value, use this
  But if you dont have and if you're using strlen, you can use
  str_from_cstr, it does strlen
 */
#define str_newn(buf, len, ...) \
  str_newn_impl(buf, len, (StringMemory){__VA_ARGS__})
STRDEF String str_newn_impl(const char* buf, size_t len, StringMemory memory);

/*
  str_newn with no len value, it calculates len via strlen
  If you dont have your String's length value or just dont want
  to use strlen by yourself, you can use this
 */
#define str_new(buf, ...) \
  str_new_impl(buf, (StringMemory){__VA_ARGS__})
STRDEF String str_new_impl(const char* buf, StringMemory memory);

/*
  Reservers needed memory for the String
  It usually multiplies it by 2 if String is too long
  Extra is additional bytes to append
 */
STRDEF bool str_reserve(String* s, size_t extra);

/*
  Shrinks the memory (cap field) to length + 1
*/
STRDEF bool str_shrink_to_fit(String* s);

/*
  Returns char in that String by a pos value
  pos stands for position (the index, starts from zero)
 */
STRDEF char str_idx(const String* s, size_t pos);

/*
  It appends char into that String
  Automatically puts \0 at the end
 */
STRDEF bool str_append(String* s, char c);

/*
  Combine all c-strings into single heap-allocated string with delimeter.
  You can have const char* or char* arrays use str_join to dispatch
  between them in compile-time.
*/
#define str_join(str, arr, delim) _Generic((arr), \
                                   Array(cchar_ptr)*: str_join_cc, \
                                   Array(char_ptr)*: str_join_c)(str, arr, delim)
STRDEF bool str_join_cc(String* str, Array(cchar_ptr)* arr, char delim);
STRDEF bool str_join_c(String* str, Array(char_ptr)* arr, char delim);

/*
  Str_cat but with C-type Strings (char*)
  "bufsz" is length of buf.
*/
STRDEF bool str_catn(String* s, const char* buf, size_t bufsz);

/*
  Wrapper of str_append_manyn, calls strlen on buf
 */
STRDEF bool str_cat(String* s, const char* buf);

/*
  It concatenates dest String and src String
  It puts src String's chars into dest String and
  puts \0 at the end and updates count and capacity (if needed)
*/
STRDEF bool str_cat_str(String* dest, const String* src);

/*
  It gets "data" field in that String
  You can always use s->data or s.data
  But this checks NULL or empty conditions
*/
STRDEF char* str_to_cstr(const String* s);

/*
  Compares 2 strings their lengths and buffers
  zero     if they're equal (length and buffer)
  positive if lhs > rhs (length or buffer)
  negative if rhs > lhs
*/
STRDEF int str_cmp(const String* lhs, const String* rhs);

/*
  Calls str_cmp with check if return value is 0 or not
*/
STRDEF bool str_equals(const String* lhs, const String* rhs);

/*
  Checks if string starts with the buf while bufsz is strlen(buf)
  If bufsz is equal to 0 or greater than view's length, returns false
*/
STRDEF bool str_starts_withn(const String* str, const char* buf, size_t bufsz);
STRDEF bool str_starts_with(const String* str, const char* buf);

/*
  Free the String
*/
STRDEF void str_free(String* s);

/*
  It clears the String, sets len to zero and make data \0
  Capacity still there and this function DOES NOT FREE THE MEMORY!
*/
STRDEF void str_clear(String* s);

/*
  Trims the whitespaces given string from left or right or both sides
*/
STRDEF void str_trim(String* s);
STRDEF void str_trim_left(String* s);
STRDEF void str_trim_right(String* s);

/*
  Closes the String if not closed or corrupted
  Inserts \0 at the end (depends on count field)
*/
STRDEF void str_close(String* s);

/*
  Checks if String is ended with \0
*/
STRDEF bool str_is_closed(const String* s);

/*
  Repeats String with provided count, modifies original String
  It appends that String at the end count - 1 times
  that means if you provide "xx" as String and 4 as count,
  you get: "xxxxxxxx" (=4*"xx")
*/
STRDEF void str_repeat(String* s, size_t count);

/*
  Formats the String with given format String and variadics
  Acts like running a temporary snprintf and applying it to String
*/
STRDEF void str_format_into(String* s, const char* fmt, ...);

/*
  Makes all alpha characters in that String lowercased - ASCII only
 */
STRDEF void str_tolower(String* s);

/*
  Makes all alpha characters in that String uppercased - ASCII only
*/
STRDEF void str_toupper(String* s);

/*
  Checks if all of chars in that String is alpha - ASCII only
 */
STRDEF bool str_isalpha(const String* s);

/*
  Checks if all chars in String is alphanumeric - ASCII only
 */
STRDEF bool str_isalphanum(const String* s);

/*
  Capitalizes String like this:
  hello -> Hello
  heLLo -> Hello
  hell1o -> Hello1o
  Doesn't touch non-alpha chars
  Uppers first char if it is alpha and lowers the rest (alpha ones)
  1hello -> 1hello (because 1 is not alpha)
 */
STRDEF void str_capitalize(String* s);

#endif /* STR_H */
