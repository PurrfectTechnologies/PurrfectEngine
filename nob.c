#define NOB_IMPLEMENTATION
#include "dependencies/nob.h"

void help(const char *const program) {
  nob_log(NOB_INFO, "Usage: %s [init/...FLAGS]");
  nob_log(NOB_INFO, "FLAGS:");
  nob_log(NOB_INFO, "  - build           Builds project.");
  nob_log(NOB_INFO, "  - shaders         Compiles shaders.");
}

bool shader_build(const char *const input_file_path, const char *const output_file_path);
bool shaders_build();
bool cmake_build();
bool cmake_init();

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  const char *const program = nob_shift_args(&argc, &argv);

  if (argc > 0) {
    const char *flag = nob_shift_args(&argc, &argv);
    if (strcmp(flag, "init") == 0) {
      if (!shaders_build() || !cmake_init() || !cmake_build()) return false;
      if (argc > 0) {
        Nob_String_Builder sb = {0};
        while (argc > 0) {
          if (sb.count > 0) {
            nob_sb_append_cstr(&sb, ", ");
          }
          nob_sb_append_cstr(&sb, nob_shift_args(&argc, &argv));
        }
        nob_sb_append_null(&sb);
        nob_log(NOB_WARNING, "Skipping arguments: %s", sb.items);
        nob_sb_free(sb);
      }
    } else {
      do {
        if (strcmp(flag, "build") == 0) {
          if (!cmake_build()) return 1;
        } else if (strcmp(flag, "shaders") == 0) {
          if (!shaders_build()) return 1;
        } else {
          nob_log(NOB_WARNING, "Unknown flag \"%s\", skipping!", flag);
        }
        flag = (argc>0?nob_shift_args(&argc, &argv):NULL);
      } while (argc > 0);
    }
  } else {
    if (!cmake_build()) return 1;
  }

  nob_log(NOB_INFO, "Success!");

  return 0;
}

bool shader_build(const char *const input_file_path, const char *const output_file_path) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "glslc", input_file_path, "-o", output_file_path);
  return nob_cmd_run_sync(cmd);
}

const char *const shaders[] = {
  "./assets/shaders/shader.vert", "./assets/shaders/shader_v.spv",
  "./assets/shaders/shader.frag", "./assets/shaders/shader_f.spv",
  "./assets/shaders/scene.vert",  "./assets/shaders/scene_v.spv",
  "./assets/shaders/scene.frag",  "./assets/shaders/scene_f.spv",
};

static_assert((NOB_ARRAY_LEN(shaders)%2)==0, "shaders' length must be even!");

bool shaders_build() {
  for (size_t i = 0; i < NOB_ARRAY_LEN(shaders); i += 2)
    if (!shader_build(shaders[i], shaders[i+1])) return false;
  return true;
}

bool cmake_build() {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "cmake", "--build", "./build/");
  return nob_cmd_run_sync(cmd);
}

bool cmake_init() {
  nob_mkdir_if_not_exists("./build/");
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "cmake", "-B", "./build/", ".");
  return nob_cmd_run_sync(cmd);
}