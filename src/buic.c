#include <buic.h>
#include <basic.h>
#include <futil.h>
#include <str.h>
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

time_t buic_compare_mtimes(const char* f1, const char* f2) {
  struct stat st_f1 = {0};
  if (stat(f1, &st_f1) != 0) {
    _rebuild_urself_error("cannot stat '%s'", f1);
    return false;
  }

  struct stat st_f2 = {0};
  if (stat(f2, &st_f2) != 0) {
    _rebuild_urself_error("cannot stat '%s'", f2);
    return false;
  }

  return st_f1.st_mtime - st_f2.st_mtime;
}

bool buic_rebuild_urself(int argc, char** argv, const char* file_name, const char* first, ...) {
  char** orig_argv = argv;
  const char* bin_name = argv_shift(&argc, &argv);

#ifdef PLATFORM_POSIX
  char old_bin[1024] = {0};
  snprintf(old_bin, sizeof(old_bin), "%s.old", bin_name);

  // If old binary exists, take its last modification time and compare it to source file
  bool needs_rebuild = false;
  if (access(old_bin, F_OK) != 0) {
    needs_rebuild = true; // no .old binary
  } else {
    needs_rebuild = buic_compare_mtimes(file_name, bin_name) > 0;
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
  cmd_push(&cmd, "cc");
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
  cmd_push(&cmd, "-o", bin_name);

  // Run rebuild command
  if (!cmd_run(&cmd)) {
    _rebuild_urself_error("cannot rebuild itself.");
    return false;
  }

  // Run the new binary and exit this old one
  execvp(bin_name, orig_argv);
  _rebuild_urself_error("cannot run new binary: %s", strerror(errno));
  return false;

#elif defined(PLATFORM_WINDOWS)
  TODO("buic_rebuild_urself not implemented for winslop.");
#endif
  return true;
}
