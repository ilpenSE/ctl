#include <stdio.h>
#include <string.h>
#define CTL_IMPLEMENTATION
#include "ctl.h"

const char* const modules[] = {
  "either", // No dependecy
  "array", // No dependecy
  "buffer", // No dependecy
  "basic", // Dependencies: either and array
  "sv", // Dependencies: basic
  "str", // Dependencies: basic and array
  "futil", // Dependencies: basic, str, either
  "buic", // Dependencies: basic, futil, str, array
  "json", // Dependencies: basic, either, str, sv, array, futil
};

#define countof(arr) sizeof(arr)/sizeof(*arr)

#define BUILD_FOLDER ".build/"
#define BUILD_LIB_FOLDER BUILD_FOLDER"lib/"
#define BUILD_INC_FOLDER BUILD_FOLDER"include/"
#define LIB_OUTPUT_NAME BUILD_LIB_FOLDER"libctl"

#define ARTIFACTS_FOLDER "artifacts/"

bool create_folder(const char* folder);
bool generate_single_header(const char* file_name, bool has_impl);
bool build(CommandBuilder* cmd, bool is_debug);
bool pack(CommandBuilder* cmd);

int main(int argc, char** argv) {
  BUIC_REBUILD_URSELF(argc, argv);
  const char* program_name = argv_shift(&argc, &argv);

  // Create all necessary folders.
  if (!create_folder(BUILD_FOLDER)) return 1;
  if (!create_folder(BUILD_LIB_FOLDER)) return 1;
  if (!create_folder(BUILD_INC_FOLDER)) return 1;

  // Parse flags
  bool is_debug = false;
  bool is_pack = false;
  for (const char* flag = argv_shift(&argc, &argv); flag; flag = argv_shift(&argc, &argv)) {
    if (strcmp(flag, "-d") == 0) is_debug = true;
    else if (strcmp(flag, "-D") == 0) is_debug = true;
    else if (strcmp(flag, "-p") == 0) is_pack = true;
    else if (strcmp(flag, "-pack") == 0) is_pack = true;
    else {
      fprintf(stderr, "ERROR: Unknown option '%s'.\n", flag);
      fprintf(stderr, "  Available flags:\n");
      fprintf(stderr, "  '-d' or '-D': Enable debugging information and don't optimize.\n");
      fprintf(stderr, "  '-p' or '-pack': Combine the build artifacts "
                      "into release package (tarball, zip and folder).\n");
    }
  }

  CommandBuilder cmd = {0};
  if (!build(&cmd, is_debug)) return 1;
  if (is_pack) if (!pack(&cmd)) return 1;
  cmd_free(&cmd);

  // Copy new ctl.h into this folder.
  cmd_insta_run("cp", BUILD_INC_FOLDER"ctl.h", ".");
}

bool build(CommandBuilder* cmd, bool is_debug)
{
  // Build object files
  for (size_t i = 0; i < countof(modules); i++) {
    cmd_push(cmd, "clang");
    cmd_push(cmd, "-I./include");
    cmd_push(cmd, "-c");
    cmd_push(cmd, "-fPIC");
    if (is_debug) {
      cmd_push(cmd, "-ggdb", "-O0", "-fsanitize=memory");
    } else {
      cmd_push(cmd, "-O2", "-ffast-math");
    }
    cmd_push(cmd, "-Wall", "-Wextra",
                   "-Wno-override-init",
                   "-Wno-initializer-overrides");

    String path = str_new(temp_sprintf("src/%s.c", modules[i]));
    cmd_push(cmd, path.data);
    cmd_push(cmd, "-o", temp_sprintf(BUILD_FOLDER"%s.o", modules[i]));
    if (!cmd_run(cmd)) return false;
  }

  // Build dynamic library
  cmd_push(cmd, "clang");
  cmd_push(cmd, "-lm");
  cmd_push(cmd, "-shared");
  for (size_t i = 0; i < countof(modules); i++) {
    String path = str_new(temp_sprintf(BUILD_FOLDER"%s.o", modules[i]));
    cmd_push(cmd, path.data);
  }
  cmd_push(cmd, "-o", LIB_OUTPUT_NAME".so");
  if (!cmd_run(cmd)) return false;

  // Build static library
  cmd_push(cmd, "ar", "-rcs");
  cmd_push(cmd, LIB_OUTPUT_NAME".a");
  for (size_t i = 0; i < countof(modules); i++) {
    String path = str_new(temp_sprintf(BUILD_FOLDER"%s.o", modules[i]));
    cmd_push(cmd, path.data);
  }
  if (!cmd_run(cmd)) return false;

  // Strip only dynamic library on release build
  if (!is_debug) {
    if (!cmd_insta_run("strip", LIB_OUTPUT_NAME".so")) return false;
  }

  // Generate single header with implementation
  return generate_single_header(BUILD_INC_FOLDER"ctl.h", true);
}

bool pack(CommandBuilder* cmd) {
  if (mkdir_if_not_exists(ARTIFACTS_FOLDER).is_error) return false;

  // Create tarball for x86_64 linux
  cmd_push(cmd, "tar", "czf", ARTIFACTS_FOLDER"x86_64-linux-gnu.tar.gz");
  cmd_push(cmd, "--transform=s,^,ctl/,");
  cmd_push(cmd, BUILD_FOLDER"include/", BUILD_FOLDER"lib/");
  if (!cmd_run(cmd)) return false;

  return true;
}

bool generate_single_header(const char* file_name, bool has_impl)
{
  bool ok = false;
  FILE* f = fopen(file_name, "w");

  fprintf(f, "/* SPDX-License-Identifier: GPL-3.0-only */\n");
  fprintf(f, "/* This is an amalgamation of libctl headers. */\n");
  fprintf(f, "#ifndef CTL_H\n");
  fprintf(f, "#define CTL_H\n\n");
  fprintf(f, "#ifdef _MSC_VER\n");
  fprintf(f, "  #error \"This header does not support MSVC, please don't use garbage slop compilers.\"\n");
  fprintf(f, "#endif\n\n");
  fprintf(f, "#ifdef __cplusplus\n");
  fprintf(f, "  #define CTLDEF extern \"C\"\n");
  fprintf(f, "#else\n");
  fprintf(f, "  #define CTLDEF extern\n");
  fprintf(f, "#endif\n");
  for (size_t i = 0; i < countof(modules); i++) {
    // Read header files
    char file[1024];
    snprintf(file, sizeof file, "./include/%s.h", modules[i]);
    Result(String) content_r = read_entire_file(file);
    if (RES_ISERR(content_r)) {
      fprintf(stderr, "Failed to read header file: %s: %s",
                      file, err_tostr(content_r.as.right.code));
      goto defer;
    }
    String content = RES_UNWRAP(content_r);

    StringView view = svs(&content);
    while (view.len > 0) {
      StringView line = sv_chop_by_delim(&view, '\n');
      // Remove SPDX license identifier
      if (sv_starts_with(line, "/* SPDX-License-Identifier")) continue;
      String upper_module = str_new(modules[i]);
      str_toupper(&upper_module);
      StringView line_tr = sv_trim(line);

      // Remove header guards
      if (sv_starts_with(line_tr, temp_sprintf("#ifndef %s_H", upper_module.data))) {
        StringView line_ahead = sv_chop_by_delim(&view, '\n');
        if (sv_starts_with(sv_trim(line_ahead), temp_sprintf("#define %s_H", upper_module.data))) {
          sv_chop_by_delim(&view, '\n'); // '\n'
          continue;
        } else {
          fprintf(f, SV_FMT"\n", SV_ARG(line));
          line = line_ahead;
        }
      }
      if (sv_starts_with(line_tr, temp_sprintf("#endif /* %s_H */", upper_module.data))) continue;

      // Remove MODULEDEF macros (extern)
      if (sv_starts_with(line_tr, temp_sprintf("#ifdef __cplusplus"))) {
        StringView line_ahead = sv_chop_by_delim(&view, '\n');
        if (sv_starts_with(sv_trim(line_ahead), temp_sprintf("#define %sDEF extern", upper_module.data))) {
          sv_chop_by_delim(&view, '\n'); // '#else'
          sv_chop_by_delim(&view, '\n'); // '#define XDEF extern'
          sv_chop_by_delim(&view, '\n'); // '#endif'
          sv_chop_by_delim(&view, '\n'); // '\n'
          continue;
        } else {
          fprintf(f, SV_FMT"\n", SV_ARG(line));
          line = line_ahead;
        }
      }

      // Replace old module extern macros to CTLDEF
      if (sv_starts_with(line_tr, temp_sprintf("%sDEF", upper_module.data))) {
        sv_chop_left(&line, temp_len()); // 'MODULEDEF'
        fprintf(f, "CTLDEF"SV_FMT"\n", SV_ARG(line));
        continue;
      }

      // Remove MSC_VER error
      if (sv_starts_with(line_tr, "#ifdef _MSC_VER")) {
        StringView line_ahead = sv_chop_by_delim(&view, '\n');
        if (sv_starts_with(sv_trim(line_ahead), "#error")) {
          sv_chop_by_delim(&view, '\n'); // '#endif'
          continue;
        } else {
          fprintf(f, SV_FMT"\n", SV_ARG(line));
          line = line_ahead;
        }
      }

      // Remove local header includes (just like in .c files)
      if (sv_starts_with(line_tr, "#include \"")) continue;

      fprintf(f, SV_FMT"\n", SV_ARG(line));
    }
    str_free(&content);
  }

  // Add implementation if needed
  if (!has_impl) goto end;
  fprintf(f, "#ifdef CTL_IMPLEMENTATION\n");
  for (size_t i = 0; i < countof(modules); i++) {
    fprintf(f, "\n");
    // Read C source files
    char file[1024];
    snprintf(file, sizeof file, "./src/%s.c", modules[i]);
    Result(String) content_r = read_entire_file(file);
    if (RES_ISERR(content_r)) {
      fprintf(stderr, "Failed to read source file: %s: %s",
                       file, err_tostr(content_r.as.right.code));
      goto defer;
    }
    fprintf(f, "// %s.c start\n", modules[i]);

    String content = RES_UNWRAP(content_r);
    StringView view = svs(&content);
    while (view.len > 0) {
      StringView line = sv_chop_by_delim(&view, '\n');
      StringView line_tr = sv_trim(line);
      // Don't write ctl header includes (dont make any sense in amalgamated header)
      if (sv_starts_with(line_tr, "#include <")) {
        sv_chop_left(&line_tr, strlen("#include <"));
        for (size_t i = 0; i < countof(modules); i++) {
          if (sv_starts_with(line_tr, temp_sprintf("%s.h>", modules[i])))
            goto cont_str;
        }
      }
      fprintf(f, SV_FMT"\n", SV_ARG(line));
    cont_str:
      continue;
    }
    fprintf(f, "// %s.c end\n", modules[i]);
    str_free(&content);
  }
  fprintf(f, "\n#endif /* CTL_IMPLEMENTATION */\n");

end:
  ok = true;
  fprintf(f, "#endif /* CTL_H */\n");
  printf("%s:1: Single header file has been created\n", file_name);
defer:
  fclose(f);
  return ok;
}

bool create_folder(const char* folder) {
  ErrorOrNot res = mkdir_if_not_exists(folder);
  if (res.is_error) {
    fprintf(stderr, "ERROR: Cannot create folder: %s: %s: %s\n",
                    folder, err_tostr(res.error.code), res.error.message);
    return false;
  }
  return true;
}
