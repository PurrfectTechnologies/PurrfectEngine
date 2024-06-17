cmake_minimum_required(VERSION 3.14)

project(PurrfectEngine)

find_package(Vulkan REQUIRED)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/json)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(dependencies/assimp)

set(KTX_FEATURE_TESTS OFF)
add_subdirectory(dependencies/KTX-Software)

include_directories(dependencies/stb/)
add_subdirectory(core)
add_subdirectory(test)