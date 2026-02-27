#include <string.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "include/nob.h"

Cmd cmd = {0};

bool add_sources(Walk_Entry entry) {
  if (entry.type == NOB_FILE_REGULAR) {
    int len = strlen(entry.path);
    if (entry.path[len - 1] == 'c') {
      // I ain't gonna free it, OS will free it anyway...
      // OS is THE Arena Allocator fr fr.
      char *src_path = malloc(len);
      memcpy(src_path, entry.path, len);
      cmd_append(&cmd, src_path);
    }
  }
  return true;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  nob_cc(&cmd);
  if (!walk_dir("./src", add_sources))
    return 1;
  nob_cc_output(&cmd, "main");
  nob_cc_flags(&cmd);
  cmd_append(&cmd, "-ggdb", "-I./src", "-I./include", "-L./lib", "-lraylib",
             "-lm");

  if (!cmd_run(&cmd))
    return 1;

  cmd_append(&cmd, "./main");

  if (!cmd_run(&cmd))
    return 1;

  return 0;
}
