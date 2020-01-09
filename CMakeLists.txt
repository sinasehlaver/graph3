cmake_minimum_required(VERSION 3.15)
project(opengl)

set(CMAKE_CXX_STANDARD 14)

include_directories( /usr/local/Cellar/glfw/ )
include_directories( /usr/local/Cellar/glew/ )
add_executable(opengl main.cpp)