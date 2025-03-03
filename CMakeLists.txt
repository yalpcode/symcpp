cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(symcpp)


file(GLOB SRC src/*.cpp)
add_library(src STATIC ${SRC})

include_directories(include)

add_executable(main main.cpp)

target_link_libraries(main 
    src
)

