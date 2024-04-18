//
// Created by Dottik on 3/4/2024.
//
#pragma once

#include "Windows.h"
#include <cstdint>

#define RebaseAddress(x) x - 0x140000000 + reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe"))

namespace RBX::Studio::Offsets {
    const static std::uintptr_t luau_execute = RebaseAddress(0x142ffc270);          // Search for "C Stack Overflow"

    const static std::uintptr_t _luaO_nilobject = RebaseAddress(
                                                          0x1466c5748);             // Get to lua_pushvalue and get inside of pseudo2addr, you will find the data xref.
    const static std::uintptr_t _luaH_dummynode = RebaseAddress(
                                                          0x1466c5608);             // Find " ,"metatable": ", and go to the top of the second xref, a ldebug function. It will contain two references to luaH_dummynode.

    // The way we get our lua state is the classic, hooking.
    const static std::uintptr_t pseudo2addr = RebaseAddress(0x142fef910);

    const static std::uintptr_t rluaE_newthread = RebaseAddress(0x142ff0e00);
    // Required for not crashing on errors.
    const static std::uintptr_t rLuaD_throw = RebaseAddress(
                                                      0x142ff3810);       // search for "memory allocation error: block too big". You will find luaG_runerrorL, last call is luaD_throw.
    const static std::uintptr_t rlua_newthread = RebaseAddress(0x142fed880);
    const static std::uintptr_t rTask_defer = RebaseAddress(0x141d0d280);

    const static std::uintptr_t rFreeBlock = RebaseAddress(0x14300a0d0);
    const static std::uintptr_t rFromLuaState = RebaseAddress(
                                                        0x141b57900);     // Appears to copy ones' L->userdata into another for new states. Search for "Failed to create Lua State", on the userthread user callback.
    const static std::uintptr_t rLuaD_rawrununprotected = RebaseAddress(
                                                                  0x142ff35c0);     // Search for luaD_rununprotected, crawl until you reach into luaD_pcall, from which, search for xrefs into a function referencing luaO_nilobject and pseudo2addr, it will also have an if check at the end checking for a negative value (first bit set).

    const static std::uintptr_t rLuaC_Step = RebaseAddress(0x1430079d0);
}

struct lua_State;

namespace FunctionTypes {
    using luau_execute = void (__fastcall *)(lua_State *L);
    using pseudo2addr = void *(__fastcall *)(lua_State *L, int32_t lua_index);
    using rTask_defer = int (__fastcall *)(lua_State *L);
    using rluaE_newthread = lua_State *(__fastcall *)(lua_State *L);
    using rlua_newthread = lua_State *(__fastcall *)(lua_State *L);
    using rFromLuaState = void (__fastcall *)(lua_State *LP, lua_State *L);
    using rFreeBlock = void (__fastcall *)(lua_State *L, int32_t sizeClass, void *block);
    using rLuaD_throw = void (__fastcall *)(lua_State *L, int32_t errcode);
    using rLuaD_rawrununprotected = int32_t (__fastcall *)(lua_State *L,
                                                           void (*PFunc)(lua_State *L, void *ud),
                                                           void *ud);
    using rLuaC_Step = size_t (__fastcall *)(lua_State *L, bool assist);
};

namespace RBX::Studio::Functions {
    const static auto luau_execute = reinterpret_cast<FunctionTypes::luau_execute>(RBX::Studio::Offsets::luau_execute);
    const static auto rTask_defer = reinterpret_cast<FunctionTypes::rTask_defer>(RBX::Studio::Offsets::rTask_defer);
    const static auto rluaE_newthread = reinterpret_cast<FunctionTypes::rluaE_newthread>(RBX::Studio::Offsets::rluaE_newthread);
    const static auto rlua_newthread = reinterpret_cast<FunctionTypes::rlua_newthread>(RBX::Studio::Offsets::rlua_newthread);
    const static auto rFromLuaState = reinterpret_cast<FunctionTypes::rFromLuaState>(RBX::Studio::Offsets::rFromLuaState);
    const static auto rFreeBlock = reinterpret_cast<FunctionTypes::rFreeBlock>(RBX::Studio::Offsets::rFreeBlock);
    const static auto rLuaD_throw = reinterpret_cast<FunctionTypes::rLuaD_throw>(RBX::Studio::Offsets::rLuaD_throw);
    const static auto rLuaD_rawrununprotected = reinterpret_cast<FunctionTypes::rLuaD_rawrununprotected>(RBX::Studio::Offsets::rLuaD_rawrununprotected);
    const static auto rLuaC_Step = reinterpret_cast<FunctionTypes::rLuaC_Step>(RBX::Studio::Offsets::rLuaC_Step);
}

/*
 *  How to get this to compile when updating Luau?
 *      - Modify lobject.cpp and lobject.h to use Studios' luaO_nilobject, same thing with ltable.cpp and ltable.h and luaH_dummynode, as well as modifying lvm.cpp to use luau_execute. You must use luau_load when compiling code, for anyone using this to develop anything.
 */