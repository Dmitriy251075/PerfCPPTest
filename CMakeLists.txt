cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(PerfCPPTest CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB_RECURSE Srcs src/*.cpp src/*.c)
file(GLOB_RECURSE Headers src/*.hpp src/*.h)

include_directories(src)

add_executable(PerfCPPTest ${Srcs} ${Headers})
