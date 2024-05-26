file(GLOB_RECURSE CORE_SOURCES "src/**.cpp" "include/PurrfectEngine/**.hpp")
add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC "./include/")
target_link_libraries(core fr glm nlohmann_json assimp)