file(GLOB_RECURSE TEST_SOURCES "src/**.cpp" "include/**.hpp")
add_executable(test ${TEST_SOURCES})
target_link_libraries(test core assimp)
target_include_directories(test PUBLIC "./include/")