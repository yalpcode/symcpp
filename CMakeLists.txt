cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(symcpp)

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG        v3.1.0
)
FetchContent_MakeAvailable(cxxopts)

file(GLOB SRC src/*.cpp)
add_library(src STATIC ${SRC})

include_directories(include)

add_executable(differentiator main.cpp)

target_link_libraries(differentiator src cxxopts::cxxopts)

add_executable(tests test/test.cpp)
target_link_libraries(tests gtest gtest_main)