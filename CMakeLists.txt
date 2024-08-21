cmake_minimum_required(VERSION 3.14)

project(PurrfectEngine)

find_package(Vulkan REQUIRED)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/json)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(dependencies/assimp)

set(KTX_FEATURE_TESTS OFF)
set(KTX_FEATURE_TOOLS OFF)
add_subdirectory(dependencies/KTX-Software)

set(ALSOFT_TESTS OFF)
set(ALSOFT_INSTALL OFF)
set(ALSOFT_INSTALL_CONFIG OFF)
set(ALSOFT_INSTALL_HRTF_DATA OFF)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF)
set(ALSOFT_INSTALL_EXAMPLES OFF)
set(ALSOFT_INSTALL_UTILS OFF)
set(ALSOFT_UPDATE_BUILD_VERSION OFF)
set(LIBTYPE STATIC)
add_subdirectory(dependencies/openal-soft)

set(BUILD_TESTING OFF)
set(BUILD_PROGRAMS OFF)
set(BUILD_EXAMPLES OFF)
add_subdirectory(dependencies/libsndfile)

add_subdirectory(dependencies/entt)

include_directories(dependencies/stb/)
include_directories(dependencies/minimp3/)
add_subdirectory(core)
add_subdirectory(test)