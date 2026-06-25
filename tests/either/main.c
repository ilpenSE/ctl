#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <either.h>

typedef const char* cc_ptr;
typedef char* c_ptr;

DECL_EITHER(cc_ptr, int);
DECL_RESULT(c_ptr);
DECL_OPTION(size_t);

Either(cc_ptr, int) doEither(int x) {
  if (x == 0) return EITHER_L(cc_ptr, int, "Hello");
  return EITHER_R(cc_ptr, int, 0);
}

Result(c_ptr) doResult(int x) {
  if (x == 0) return RES_ERR_MSG(c_ptr, ERR_INVALID_ARG, "x is NEVER meant to be 0");
  char* ok = (char*)malloc(32);
  memcpy(ok, "Hello!", 7);
  return RES_OK(c_ptr, ok);
}

Option(size_t) returnSizeT(int x) {
  if (x==0) return OPT_NONE(size_t);
  return OPT_SOME(size_t, (size_t)123);
}

ErrorOrNot doErrorOrNot(int x) {
  if (x == 0) return EON_ERROR(ERR_INVALID_ARG, "x is not 0 beyb");
  printf("Hello, World!\n");
  return EON_SUCCESS;
}

int main() {
  Error test = RES_ERR_MSG(c_ptr, ERR_INTERNAL, "Some error message").as.right;
  printf("%s\n", test.message);

  // EITHERS
  printf("\n=== EITHERS ===\n");
  Either(cc_ptr, int) a = doEither(1); // 0 in int
  printf("Violent left check in eitha: %s\n",  EITHER_GETL(a));
  printf("Violent right check in eitha: %d\n", EITHER_GETR(a));
  if (a.is_left) printf("Either is left!!!: %s\n", a.as.left);
  else printf("Either is RIGHT!!!::: %d\n", a.as.right);

  // RESULTS
  printf("\n=== RESULTS ===\n");
  Result(c_ptr) b = doResult(1); // "Hello!" in char*
  char* str = RES_UNWRAP(b);
  // if b == doResult(0) => unwrap is dangerous and this will give SEGFAULT:
  printf("Unwrapped forcefully and violently: %s\n", str);
  if (RES_ISERR(b)) printf("Result has errur: %s\n", RES_GETE(b).message);
  else printf("Result does NOT have errur: %s\n", RES_UNWRAP(b));

  // OPTIONS
  printf("\n=== OPTIONS ===\n");
  Option(size_t) c = returnSizeT(0); // 123 in size_t
  printf("Unwrapped option: %ld\n", OPT_UNWRAP(c));
  printf("Checking the option\n");
  if (c.is_some) printf("With check: %ld\n", c.value);
  else printf("crazy!! no size_t fo u m8\n");

  // ERROR OR NUTS
  printf("\n=== ERROR OR NUTS ===\n");
  ErrorOrNot d = doErrorOrNot(0);
  if (d.is_error) printf("error or nuts gave error: %s\n", d.error.message);

  return 0;
}
