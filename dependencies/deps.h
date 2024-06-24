//
// IMPORTANT!
// 
// file libglfw3.lib has to be in folder glfw/build/src/ (Required by fr)
// You can copy it from glfw/build/src/Debug/glfw3.lib (on Windows)
//

#ifndef   PURRENGINE_DEPS_H_
#define   PURRENGINE_DEPS_H_

#define deps_path "./dependencies"

#define GLFW_path deps_path"/glfw/"

#define GLM_path deps_path"/glm/"

#define fr_path deps_path"/fr/"
#define fr_include_path fr_path"include/"
#define fr_lib_path fr_path"build/"

#define openal_path deps_path"/openal-soft"
#define openal_include_path openal_path "include/"


bool configure_fr() {
  const char *config = 
    "#ifndef   _CONFIG_H_\n"
    "#define   _CONFIG_H_\n\n"
    "// Path to vulkan sdk on your machine\n"
    "#define VULKAN_SDK_PATH \""VulkanSDK_path"\"\n\n"
    "// Path to glfw\n"
    "#define GLFW_PATH \""GLFW_path"\"\n\n"
    "// Path to glm\n"
    "#define GLM_PATH \""GLM_path"\"\n\n"
    "// Root path for fr\n"
    "#define FR_PATH \""fr_path"\"\n\n"
    "// Path to openal-soft\n"
    "#define OPENAL_PATH \""openal_include_path"\"\n\n"
    "#endif // _CONFIG_H_";

  Nob_String_Builder sb = {0};
  nob_sb_append_cstr(&sb, config);
  nob_write_entire_file(fr_path"config.h", sb.items, sb.count);
  nob_sb_free(sb);

  return true;
}

bool bootstrap() {
  { // Bootstrap fr
    Nob_File_Paths paths = {0};
    bool found = false; // Look for nob.* && skip if ext == c

    if (!nob_read_entire_dir(fr_path, &paths)) return false;
    for (size_t i = 0; i < paths.count; ++i) {
      char *path = paths.items[i];
      if (strcmp(path, nob_filename) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      if (!configure_fr()) return false;

      Nob_Cmd cmd = {0};
      nob_cmd_append(&cmd, "cc", "-o", fr_path""nob_filename, fr_path"nob.c");
      if (!nob_cmd_run_sync(cmd)) return false;
    }
  }

  return true;
}

bool build_deps() {
  if (!bootstrap()) return false;

  Nob_Cmd cmd = {0};

  { cmd.count = 0;
    nob_cmd_append(&cmd, "cmake", "-DCMAKE_BUILD_TYPE=Release", "-B", GLFW_path"build/", GLFW_path);
    if (!nob_cmd_run_sync(cmd)) return false;
  }

  { cmd.count = 0;
    nob_cmd_append(&cmd, "cmake", "--build", GLFW_path"build/");
    if (!nob_cmd_run_sync(cmd)) return false;
  }

  { cmd.count = 0;
    nob_cmd_append(&cmd, fr_path""nob_filename);
    if (!nob_cmd_run_sync(cmd)) return false;
  }

  return true;
}

#endif // PURRENGINE_DEPS_H_