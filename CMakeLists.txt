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

add_subdirectory(dependencies/openal-soft)
include_directories(dependencies/openal-soft/include)

add_subdirectory(dependencies/libsndfile)
include_directories(dependencies/libsndfile/include)

include_directories(dependencies/stb/)
include_directories(dependencies/minimp3/)
add_subdirectory(core)
add_subdirectory(test)