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

// This class dictates the way execution is performed. It contains many utilities and required functions to Execute luau bytecode.
class [[maybe_unused]] Execution {
private:
    static Execution *singleton;
public:
    static Execution *GetSingleton();

    int RegisterEnvironment(lua_State *L, bool useInitScript);

    int lua_loadstring(lua_State *L, const std::string &code, std::string chunkName);

    std::string Compile(const std::string &code);
};

