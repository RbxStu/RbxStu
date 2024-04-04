cmake_minimum_required(VERSION 3.0)

project(Imgui LANGUAGES CXX)

set(PROJECT Imgui)
set(CMAKE_CXX_STANDARD 23)

file(GLOB IMGUI_SOURCES *.cpp *.h *.hpp)

add_library(${PROJECT} STATIC
        ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")