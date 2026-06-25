#include <sv.h>
#include <basic.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

StringView sv_from_cstrn_impl(const char* buf, size_t len, StringSlice slice) {
  StringView sv = {0};
  if (slice.start > slice.end || slice.start > len || slice.end > len) return sv;
  sv.data = buf + slice.start;
  sv.len = slice.end - slice.start;
  return sv;
}

StringView sv_from_cstr_impl(const char* buf, StringSlice slice) {
  if (!buf) return (StringView){0};
  size_t buflen = strlen(buf);
  return sv_from_cstrn_impl(buf, buflen, slice);
}

StringView sv_from_str_impl(const String* s, StringSlice slice) {
  return sv_from_cstrn_impl(s->data, s->len, slice);
}

void sv_tostr(StringView sv, char* out) {
  if (!out) return;
  memcpy(out, sv.data, sv.len);
  out[sv.len] = '\0';
}

void sv_clear(StringView *sv) {
  if (sv == NULL) return;
  sv->len = 0;
  sv->data = "";
}

bool sv_isempty(StringView sv) {
  return sv.len == 0 && (!sv.data ? true : *sv.data == 0);
}

int sv_cmp(StringView lhs, StringView rhs) {
  int sdiff = (int)lhs.len - (int)rhs.len;
  if (sdiff != 0) return sdiff; // size diff
  return memcmp(lhs.data, rhs.data, lhs.len);
}

bool sv_starts_withn(StringView view, const char* buf, size_t bufsz) {
  if (view.len == 0) return false;
  if (0 == bufsz || bufsz > view.len) return false;
  return memcmp(view.data, buf, bufsz) == 0;
}

bool sv_starts_with(StringView view, const char* buf) {
  return sv_starts_withn(view, buf, strlen(buf));
}

bool sv_ends_withn(StringView view, const char* buf, size_t bufsz) {
  if (view.len == 0) return false;
  if (0 == bufsz || bufsz > view.len) return false;
  return memcmp(view.data + view.len - bufsz, buf, bufsz) == 0;
}

bool sv_ends_with(StringView view, const char* buf) {
  return sv_ends_withn(view, buf, strlen(buf));
}

bool sv_equals(StringView lhs, StringView rhs) {
  return sv_cmp(lhs, rhs) == 0;
}

bool sv_equals_cstr(StringView lhs, const char* rhs) {
  StringView view = sv(rhs);
  return sv_equals(lhs, view);
}

StringView sv_chop_by_delim(StringView* sv, char delim) {
  StringView chopped = {0};
  const char* found = memchr(sv->data, (uchar_t)delim, sv->len);
  if (!found) return chopped;
  size_t i = (size_t)(found - sv->data);

  chopped.data = sv->data;
  chopped.len = i;

  size_t remaining = sv->len - i - 1;
  sv->len = remaining;
  if (remaining == 0) {
    sv->data = NULL;
  } else {
    sv->data += i + 1;
  }

  return chopped;
}

StringView sv_chop_by_func(StringView* sv, int (*func)(int)) {
  StringView chopped = {0};
  size_t i = 0;
  for (; i < sv->len; i++)
    if (func((uchar_t)sv->data[i])) break;
  if (i == sv->len) return chopped;

  chopped.data = sv->data;
  chopped.len = i;

  size_t remaining = sv->len - i - 1;
  sv->len = remaining;
  if (remaining == 0) {
    sv->data = NULL;
  } else {
    sv->data += i + 1;
  }

  return chopped;
}

StringView sv_chop_left(StringView* sv, size_t n) {
  if (n >= sv->len) return (StringView) {0};
  StringView res = {.data=sv->data, .len=n};
  sv->data += n;
  sv->len -= n;
  return res;
}

StringView sv_chop_right(StringView* sv, size_t n) {
  StringView res = {.data=sv->data + sv->len - n, .len=n};
  sv->len -= n;
  return res;
}

StringView sv_trim_left(StringView sv) {
  while (sv.len > 0 && isspace((uchar_t)sv.data[0])) {
    sv.data++;
    sv.len--;
  }
  return sv;
}

StringView sv_trim_right(StringView sv) {
  while (sv.len > 0 && isspace((uchar_t)sv.data[sv.len - 1])){
    sv.len--;
  }
  return sv;
}

StringView sv_trim(StringView sv) {
  return sv_trim_right(sv_trim_left(sv));
}
