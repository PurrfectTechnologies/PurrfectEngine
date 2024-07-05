file(GLOB_RECURSE CORE_SOURCES "src/**.cpp" "include/PurrfectEngine/**.hpp")
add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC "./include/")
target_link_libraries(core assimp Vulkan::Vulkan glfw glm nlohmann_json ktx OpenAL sndfile)