cmake_minimum_required(VERSION 3.11)

project(crossover)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(dependencies/cimgui)

