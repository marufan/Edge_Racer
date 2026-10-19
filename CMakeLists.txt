cmake_minimum_required(VERSION 3.20)
project(EdgeRacer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wno-missing-field-initializers)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
else()
    add_compile_options(-O3)
endif()

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(EdgeRacer ${SOURCES})

target_include_directories(EdgeRacer PRIVATE src)
target_link_libraries(EdgeRacer PRIVATE raylib)
