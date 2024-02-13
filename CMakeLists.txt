# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)
project(JockerParallelProgramming)

include_directories(src)

# Especifica el archivo fuente principal
add_executable(main src/main.cpp)
