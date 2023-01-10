cmake_minimum_required(VERSION 3.11)

project(crossover)

set(SOURCES main.c math.c)
set(HEADERS math.h)

add_executable(crossover ${SOURCES} ${HEADERS})

target_link_libraries(crossover PRIVATE cimgui)
target_include_directories(crossover PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/cimgui)
