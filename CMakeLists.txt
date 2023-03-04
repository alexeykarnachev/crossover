cmake_minimum_required(VERSION 3.11)

project(crossover)

if(WIN32) # to mingw work as all the others
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

set(CMAKE_CXX_STANDARD 11)

# Cimgui settings
set(BAKENDS_FOLDER "${CMAKE_BINARY_DIR}/../dependencies/cimgui/imgui/backends/")
set(TABLES_SOURCE "${CMAKE_BINARY_DIR}/../dependencies/cimgui/imgui/imgui_tables.cpp")

include_directories(${CMAKE_BINARY_DIR}/../dependencies/cimgui/)
include_directories(${CMAKE_BINARY_DIR}/../dependencies/cimgui/imgui)
include_directories(${CMAKE_BINARY_DIR}/../dependencies/cimgui/cimplot)
include_directories(${CMAKE_BINARY_DIR}/../dependencies/cimgui/cimplot/implot)
include_directories(${CMAKE_BINARY_DIR}/../dependencies/cimgui/generator/output/)
add_definitions("-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1")

set(IMGUI_SOURCES 
../dependencies/cimgui/cimgui.cpp 
../dependencies/cimgui/imgui/imgui.cpp 
../dependencies/cimgui/imgui/imgui_draw.cpp 
../dependencies/cimgui/imgui/imgui_demo.cpp 
../dependencies/cimgui/imgui/imgui_widgets.cpp
../dependencies/cimgui/imgui/imgui_tables.cpp
../dependencies/cimgui/cimplot/cimplot.cpp 
../dependencies/cimgui/cimplot/implot/implot.cpp 
../dependencies/cimgui/cimplot/implot/implot_demo.cpp 
../dependencies/cimgui/cimplot/implot/implot_items.cpp 
${TABLES_SOURCE} 
)

set(IMGUI_SOURCES_sdl)
set(IMGUI_LIBRARIES)

if (WIN32)
    add_definitions("-DIMGUI_IMPL_API=extern \"C\" __declspec\(dllexport\)")
else(WIN32)
    add_definitions("-DIMGUI_IMPL_API=extern \"C\" ")
endif(WIN32)

add_compile_definitions("IMGUI_IMPL_OPENGL_LOADER_GL3W")

# optional adding freetype
option(IMGUI_FREETYPE "add Freetype2" OFF)

if(IMGUI_FREETYPE)
	FIND_PACKAGE(freetype REQUIRED PATHS ${FREETYPE_PATH})
	list(APPEND IMGUI_LIBRARIES freetype)
	list(APPEND IMGUI_SOURCES ../dependencies/cimgui/imgui/misc/freetype/imgui_freetype.cpp)
	add_definitions("-DCIMGUI_FREETYPE=1")
endif(IMGUI_FREETYPE)

# opengl3
list(APPEND IMGUI_SOURCES ${BAKENDS_FOLDER}imgui_impl_opengl3.cpp)
include_directories(../dependencies/cimgui/imgui/examples/libs/gl3w)
if(WIN32)
       list(APPEND IMGUI_LIBRARIES opengl32)
else(WIN32) # Unix
       list(APPEND IMGUI_LIBRARIES GL)
endif(WIN32)

# GLFW
list(APPEND IMGUI_SOURCES ${BAKENDS_FOLDER}imgui_impl_glfw.cpp)

set(GLFW_VERSION 3.3.8)
include(FetchContent)
	FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/${GLFW_VERSION}.tar.gz)

FetchContent_GetProperties(glfw)
if (NOT glfw_POPULATED)
	set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(glfw)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

# glfw/imgui gets confused if it is not statically built
IF (WIN32)
  add_library(cimgui STATIC ${IMGUI_SOURCES})
ELSE()
  add_library(cimgui SHARED ${IMGUI_SOURCES})
ENDIF()

# nativefiledialog
add_subdirectory(${CMAKE_BINARY_DIR}/../dependencies/nativefiledialog-extended/)

# Link all
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(GLOB SOURCES "src/**/*.c" "src/*.c")
file(GLOB CORE_SOURCES "src/core/*.c")
file(GLOB TEST_SOURCES "tests/*.c")
set(SOURCES ${SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} ${SOURCES} "../dependencies/glad/glad.c")
add_executable(tests ${CORE_SOURCES} ${TEST_SOURCES} "tests/test_all.c")
target_link_libraries(tests m)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_GLFW)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Create an option for enabling ASAN with debug build
option(ASAN_DEBUG "Enable ASAN with debug build" OFF)

if(ASAN_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_libraries(${PROJECT_NAME} -fsanitize=address)
    target_compile_options(tests PRIVATE -fsanitize=address)
    target_link_libraries(tests -fsanitize=address)
endif()

if (MINGW)
	target_link_options(${PROJECT_NAME} PRIVATE "-mconsole")
endif()

target_link_libraries(${PROJECT_NAME} ${IMGUI_SDL_LIBRARY} cimgui)
target_link_libraries(${PROJECT_NAME} ${IMGUI_LIBRARIES} glfw)
target_link_libraries(${PROJECT_NAME} nfd)
