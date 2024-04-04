//
// Created by Dottik on 3/4/2024.
//
#pragma once

#include "Windows.h"
#include <cstdint>

#define RebaseAddress(x) x - 0x140000000 + reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe"))

namespace RBX::Studio::Offsets {
    const static std::uintptr_t luau_execute = RebaseAddress(0x142f5fb90);      // Search for "C Stack Overflow"
    const static std::uintptr_t luau_load = RebaseAddress(0x142f697a0);         // Search for "Bytecode Version"

    const static std::uintptr_t _luaO_nilobject = RebaseAddress(
                                                          0x1465dd4b0);   // Get to lua_pushvalue and get inside of pseudo2addr, you will find the data xref.
    const static std::uintptr_t _luaH_dummynode = RebaseAddress(
                                                          0x1465dd358);   // Find " ,"metatable": ", and go to the top of the second xref, a ldebug function. It will contain two references to luaH_dummynode.
}

struct lua_State;

namespace FunctionTypes {
    using luau_load = int32_t (__fastcall *)(struct lua_State *L, const char *chunkName, const char *bytecode,
                                             int32_t bytecodeSize,
                                             int32_t env);
    using luau_execute = void (__fastcall *)(struct lua_State *L);
};

namespace RBX::Studio::Functions {
    const static auto luau_load = reinterpret_cast<FunctionTypes::luau_load>(RBX::Studio::Offsets::luau_load);
    const static auto luau_execute = reinterpret_cast<FunctionTypes::luau_execute>(RBX::Studio::Offsets::luau_execute);
}

/*
 *  How to get this to compile when updating Luau?
 *      - Modify lobject.cpp and lobject.h to use Studios' luaO_nilobject, same thing with ltable.cpp and ltable.h and luaH_dummynode, as well as modifying lvm.cpp to use luau_execute. You must use luau_load when compiling code, for anyone using this to develop anything.
 */