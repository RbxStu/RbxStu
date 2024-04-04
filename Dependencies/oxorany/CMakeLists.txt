cmake_minimum_required(VERSION 3.26)
project(oxorany)

set(CMAKE_CXX_STANDARD 23)

add_library(oxorany STATIC oxorany.cpp oxorany.h)
target_include_directories(oxorany PUBLIC "include/")
