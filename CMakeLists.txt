cmake_minimum_required(VERSION 3.26)
project(Module)

add_definitions(-DLUAI_GCMETRICS)   # Force GC metrics on Luau.
set(PROJECT_NAME Module)
set(CMAKE_CXX_STANDARD 23)

add_library(Module SHARED
        main.cpp
        Execution.cpp
        Execution.hpp
        Environment/Environment.cpp
        Environment/Environment.hpp
        Execution.cpp
        Utilities.cpp
        Utilities.hpp
        Closures.cpp
        Closures.hpp
        Environment/EnvironmentLibrary.cpp
        Environment/EnvironmentLibrary.hpp
        Environment/EnvironmentLibrary.hpp
        Environment/ClosureLibrary.cpp
        Environment/ClosureLibrary.hpp
        Environment/DebugLibrary.cpp
        Environment/DebugLibrary.hpp
        Scheduler.cpp
        Scheduler.hpp
        Security.cpp
        Hook.cpp
        Hook.hpp
        Hook.hpp
        Security.hpp
)
target_include_directories(Module PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(Module PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")
# Dependencies


include(FetchContent)

# Curl For People
# FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
#         GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8)
# FetchContent_MakeAvailable(cpr)

#rcmp
# FetchContent_Declare(rcmp GIT_REPOSITORY https://github.com/Smertig/rcmp.git
#         GIT_TAG f5f75ae00a57c67fe41f79fd59c4b7f6997b999e)
# FetchContent_MakeAvailable(rcmp)

# cpr
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/cpr")

# Luau
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Luau")

# minhook
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Minhook")

# Imgui
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Imgui")

# oxorany
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/oxorany")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        # minhook
        minhook
        # Curl For People
        cpr::cpr

        # Luau
        Luau.Compiler
        Luau.Ast
        # Luau.Analysis
        Luau.VM
        Luau.VM.Internals

        # Imgui
        Imgui

        # oxorany
        oxorany

        Dbghelp.lib
)

