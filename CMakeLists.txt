cmake_minimum_required(VERSION 3.10)
project(test01 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp",
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp",
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
