#include <either.h>
#include <assert.h>

const char* err_tostr(ErrorCode err_code) {
  switch(err_code) {
#define X(name, msg) case ERR_##name: return msg;
ERROR_CODES
#undef X
  case ERR_NOERR: return "(no error)";
  default: return NULL;
  }
  assert(false && "unreachable: err_tostr");
}
