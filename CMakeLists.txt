cmake_minimum_required(VERSION 3.24)
project(SmallFastRayTracer)

set(CMAKE_CXX_STANDARD 14)

include_directories(dependencies)
include_directories(dependencies/glad)
include_directories(dependencies/windowing)

add_executable(SmallFastRayTracer
        main.c)

add_custom_command(TARGET SmallFastRayTracer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/shaders/final.glsl" "${CMAKE_SOURCE_DIR}/shaders/rayTracer.glsl" "${CMAKE_SOURCE_DIR}/shaders/screenQuad.glsl" $<TARGET_FILE_DIR:SmallFastRayTracer>)

find_package(OpenGL REQUIRED)
target_link_libraries(SmallFastRayTracer OpenGL::GL)
