//
// Created by Dottik on 14/11/2023.
//
#pragma once

// C
#include <Windows.h>
#include <lualib.h>

// C++
#include <cstdint>
#include <string>
#include "Security.hpp"

// This class dictates the way execution is performed. It contains many utilities and required functions to Execute luau
// bytecode.
class [[maybe_unused]] Execution {
private:
    static Execution *singleton;

public:
    static Execution *get_singleton();

    int register_environment(lua_State *L, bool useInitScript, _In_ _Out_ int *schedulerKey);

    int lua_loadstring(lua_State *L, const std::string &code, std::string chunkName, RBX::Identity identity);

    std::string compile_to_bytecode(const std::string &code);
};
