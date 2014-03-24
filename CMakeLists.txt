#Copyright (c) 2014 Julius Ikkala

#DisplayRaw is licensed under the 3-clause BSD License.
#Please see the accompanying LICENSE file for information about the License.


cmake_minimum_required(VERSION 2.6)
project(DRAW C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB DRAW_C displayraw.c)

add_definitions(-g)
add_executable(displayraw ${DRAW_C})

target_link_libraries(displayraw SDL2)

