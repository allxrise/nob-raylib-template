#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <string.h>

typedef struct {
  const char *path;
  bool recursive;
  bool pedantic;
} Project_Folder;

const char *common_flags[] = {
    "-DPLATFORM_DESKTOP",
    "-D_GLFW_X11",
    "-ggdb",
    "-I./vendor/raylib/src/",
    "-I./vendor/box2d/include/",
    "-I./src",
};

bool is_c_file(const char *path) {
  const char *ext = strrchr(path, '.');
  return (ext != NULL && strcmp(ext, ".c") == 0);
}

bool mkdir_recursive(const char *path) {
  char *temp_path = temp_sprintf("%s", path);
  size_t len = strlen(temp_path);
  for (size_t i = 1; i < len; ++i) {
    if (temp_path[i] == '/') {
      temp_path[i] = '\0';
      if (!mkdir_if_not_exists(temp_path))
        return false;
      temp_path[i] = '/';
    }
  }
  return mkdir_if_not_exists(temp_path);
}

bool collect_and_compile(const char *path, bool recursive, bool pedantic,
                         File_Paths *obj_files) {
  File_Paths children = {0};
  if (!read_entire_dir(path, &children))
    return false;

  for (size_t i = 0; i < children.count; ++i) {
    if (children.items[i][0] == '.')
      continue;

    const char *full_path = temp_sprintf("%s/%s", path, children.items[i]);
    File_Type type = get_file_type(full_path);

    if (type == FILE_DIRECTORY && recursive) {
      if (!collect_and_compile(full_path, true, pedantic, obj_files))
        return false;
    } else if (type == FILE_REGULAR && is_c_file(full_path)) {
      const char *no_dot_path = (full_path[0] == '.' && full_path[1] == '/')
                                    ? full_path + 2
                                    : full_path;
      const char *obj_path = temp_sprintf("./build/%s.o", no_dot_path);
      da_append(obj_files, obj_path);

      char *obj_dir = temp_sprintf("%s", obj_path);
      char *last_slash = strrchr(obj_dir, '/');
      if (last_slash) {
        *last_slash = '\0';
        if (!mkdir_recursive(obj_dir))
          return false;
      }

      if (needs_rebuild(obj_path, &full_path, 1)) {
        Cmd cmd = {0};
        cmd_append(&cmd, "cc", "-c", full_path, "-o", obj_path);
        if (pedantic)
          cmd_append(&cmd, "-Wall", "-Wextra", "-Wpedantic");
        for (size_t j = 0; j < NOB_ARRAY_LEN(common_flags); ++j) {
          cmd_append(&cmd, common_flags[j]);
        }
        if (!cmd_run_sync(cmd))
          return false;
      }
    }
  }
  return true;
}

const char *out_exe = "./build/main";

Project_Folder folders[] = {{"./src", true, true},
                            {"./vendor/raylib/src", false, false},
                            {"./vendor/box2d/src", true, false}};

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);

  if (!mkdir_if_not_exists("./build"))
    return 1;

  File_Paths object_files = {0};

  for (size_t i = 0; i < NOB_ARRAY_LEN(folders); ++i) {
    if (!collect_and_compile(folders[i].path, folders[i].recursive,
                             folders[i].pedantic, &object_files))
      return 1;
  }

  if (needs_rebuild(out_exe, object_files.items, object_files.count)) {
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    for (size_t i = 0; i < object_files.count; ++i)
      cmd_append(&cmd, object_files.items[i]);
    cmd_append(&cmd, "-o", out_exe, "-lm", "-ldl", "-lpthread", "-lX11",
               "-lGL");
    if (!cmd_run_sync(cmd))
      return 1;
  }

  if (argc > 1 && strcmp(argv[1], "run") == 0) {
    Cmd run_cmd = {0};
    cmd_append(&run_cmd, out_exe);
    if (!cmd_run_sync(run_cmd))
      return 1;
  }

  return 0;
}
