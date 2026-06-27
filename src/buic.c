#include <buic.h>
#include <basic.h>
#include <futil.h>
#include <str.h>
#include <sv.h>
#include <array.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef PLATFORM_POSIX
  #include <unistd.h>
  #include <sys/wait.h>
  #include <sys/stat.h>
  #include <time.h>
#elif PLATFORM_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #error "Your platform is not supported."
#endif

bool cmd_run_impl(CommandBuilder* cmd, CommandRunOptions opts) {
  if (arr_len(*cmd) == 0) return false;

#ifdef PLATFORM_POSIX
  if (opts.log) {
    printf("[BUIC/INFO] Running: ");
    size_t len = arr_len(*cmd);
    for (size_t i = 0; i < len; i++) {
      printf("%s", cmd->items[i]);
      if (i != len - 1) printf(" ");
    }
    printf("\n");
  }

  pid_t pid = fork();
  if (pid == 0) {
    // child
    arr_push(cmd, NULL);
    execvp(cmd->items[0], (char* const*)cmd->items);
    fprintf(stderr, "[BUIC/ERROR] ");
    perror("execvp");
    exit(1);
  } else if (pid > 0) {
    // parent
    int wstatus = 0;
    waitpid(pid, &wstatus, 0);
    if (WIFEXITED(wstatus)) {
      int exit_code = WEXITSTATUS(wstatus);
      if (exit_code != 0) {
        fprintf(stderr, "[BUIC/ERROR] Command failed with code %d\n", exit_code);
        return false;
      }
    }
  } else {
    fprintf(stderr, "[BUIC/ERROR] ");
    perror("fork");
    return false;
  }

#elif defined(PLATFORM_WINDOWS)
  // Since this is a microslop product, you have to suffer
  // Concat the command line string and let the kernel parse again to have performance issues
  String cmdline_str = {0};
  size_t len = arr_len(*cmd);
  for (size_t i = 0; i < len; i++) {
    str_cat(&cmdline_str, cmd->items[i]);
    if (i != len - 1) str_append(&cmdline_str, ' ');
  }

  if (opts.log) {
    printf("[BUIC/INFO] Running: %.*s\n", (int)cmdline_str.len, cmdline_str.data);
  }

  STARTUPINFOA si = {0};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {0};
  BOOL ok = CreateProcessA(NULL, cmdline_str.data, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
  str_free(&cmdline_str);
  if (!ok) {
    fprintf(stderr, "[BUIC/ERROR] CreateProcess failed: %lu\n", GetLastError());
    return false;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD exit_code = 0;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  if (exit_code != 0) {
    fprintf(stderr, "[BUIC/ERROR] Command failed with code %lu\n", exit_code);
    return false;
  }
#endif

  // Reset if needed
  if (opts.reset) {
    arr_len(*cmd) = 0;
  }

  return true;
}

bool cmd_insta_run_impl(const char* first, ...) {
  bool ok = false;
  if (!first) goto defer;
  CommandBuilder cmd = {0};
  cmd_push(&cmd, first);

  va_list args;
  va_start(args, first);
  const char* arg;
  while ((arg = va_arg(args, const char*)) != NULL) {
    cmd_push(&cmd, arg);
  }
  va_end(args);
  if (!cmd_run(&cmd)) goto defer_cmd;

  ok = true;
defer_cmd:
  cmd_free(&cmd);
defer:
  return ok;
}

#define _rebuild_urself_error(fmt, ...) \
  do { \
    fprintf(stderr, "ERROR: Cannot rebuild urself: "fmt": %s\n", ##__VA_ARGS__, strerror(errno)); \
  } while (0)

ctl_time_t buic_compare_mtimes(const char* f1, const char* f2) {
  futil_struct_stat st_f1, st_f2 = {0};
  if (!futil_stat(f1, &st_f1)) {
    _rebuild_urself_error("cannot stat '%s'", f1);
    return false;
  }

  if (!futil_stat(f2, &st_f2)) {
    _rebuild_urself_error("cannot stat '%s'", f2);
    return false;
  }
  return (ctl_time_t)(st_f1.st_mtime - st_f2.st_mtime);
}

bool buic_rebuild_urself(int argc, char** argv, const char* file_name, const char* first, ...) {
  char** orig_argv = argv;
  const char* bin_name = argv_shift(&argc, &argv);
  char old_bin[1024] = {0};
  snprintf(old_bin, sizeof(old_bin), "%s.old", bin_name);
  bool needs_rebuild = false;

  // If old binary exists, take its last modification time and compare it to source file
  if (!futil_access(old_bin, F_OK)) {
    needs_rebuild = true; // no .old binary
  } else {
    // fail-safe over fail-silent policy
    // Even if both script and binary has same last mtime, rebuild anyway.
    // Because in other way, if you for example extract these files from
    // a zip or you are so fast that you edited and instantly run build
    // binary and so their mtimes can be equal. At this point, we'll try
    // to rebuild anyway because one additional build doesn't kill
    needs_rebuild = buic_compare_mtimes(file_name, bin_name) >= 0;
  }

  // Early return if no rebuild needed
  if (!needs_rebuild) return true;
  printf("INFO: Change detected in build script, rebuilding itself.\n");

  // Rename the binary to old one
  printf("INFO: Renaming: '%s' -> '%s'\n", bin_name, old_bin);
  if (rename(bin_name, old_bin) != 0) {
    _rebuild_urself_error("cannot rename '%s'", bin_name);
    return false;
  }

  // Construct and run rebuild command
  CommandBuilder cmd = {0};
  if (NATIVE_COMPILER) cmd_push(&cmd, NATIVE_COMPILER);
  else cmd_push(&cmd, buic_get_native_compiler());
  cmd_push(&cmd, file_name);
  // custom flags if you need
  if (first) {
    cmd_push(&cmd, first);
    va_list args;
    va_start(args, first);
    const char* arg;
    while ((arg = va_arg(args, const char*)) != NULL)
      cmd_push(&cmd, arg);
    va_end(args);
  }
  if (strcmp(cmd.items[0], "cl.exe") == 0 ||
      strcmp(cmd.items[0], "cl") == 0)
    cmd_push(&cmd, temp_sprintf("/Fe:%s", bin_name));
  else cmd_push(&cmd, "-o", bin_name);

  // Run rebuild command
  if (!cmd_run(&cmd)) {
    _rebuild_urself_error("cannot rebuild itself.");
    return false;
  }

  // Run the new binary and exit this old one
  ctl_execvp(bin_name, orig_argv);
  _rebuild_urself_error("cannot run new binary: %s", strerror(errno));
  return false;
}

const char* buic_get_native_compiler(void) {
  const char *env = getenv("CC");
  if (env && *env) return env; // use user's definition if exists

#ifdef PLATFORM_UNIX
  // Search for "cc", "gcc", "clang"
  const char* candidates[] = {"cc","gcc","clang",NULL};
#elif defined(PLATFORM_APPLE)
  const char* candidates[] = {"clang",NULL};
#else
  const char* candidates[] = {"cl.exe","gcc.exe","cc.exe","clang.exe",NULL};
#endif

  for (int i = 0; candidates[i]; i++) {
    String s = buic_search_path(candidates[i]);
    bool found = s.data != NULL;
    str_free(&s);
    if (found) return candidates[i];
  }
  return NULL;
}

String buic_search_path(const char* bin) {
#ifdef PLATFORM_WINDOWS
  char full[PATH_MAX]; // NOTE: bin should ends with .exe
  size_t fullsz = (size_t)SearchPathA(NULL, bin, NULL, PATH_MAX, full, NULL);
  if (fullsz == 0) goto fail;
  return str_newn(full, fullsz);
#else
  const char* path_env = getenv("PATH");
  if (!path_env) goto fail;
  StringView env_sv = sv(path_env);
  StringView dir = sv_chop_by_delim(&env_sv, ':');
  while (dir.len > 0) {
    char full[PATH_MAX];
    int n = snprintf(full, sizeof(full), SV_FMT PATH_SEP_DQ"%s", SV_ARG(dir), bin);
    assert(n >= 0);
    if (futil_access(full, F_OK)) return str_newn(full, (size_t)n);
    dir = sv_chop_by_delim(&env_sv, ':');
  }
#endif
fail:
  return (String){0};
}
