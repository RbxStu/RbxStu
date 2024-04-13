//
// Created by Dottik on 3/4/2024.
//
#pragma once

#include "Windows.h"
#include <cstdint>

#define RebaseAddress(x) x - 0x140000000 + reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe"))

namespace RBX::Studio::Offsets {
    const static std::uintptr_t luau_execute = RebaseAddress(0x142f80bf0);          // Search for "C Stack Overflow"
    //const static std::uintptr_t luau_load = RebaseAddress(0x142f697a0);             // Search for "Bytecode Version"

    const static std::uintptr_t _luaO_nilobject = RebaseAddress(
                                                          0x146603268);             // Get to lua_pushvalue and get inside of pseudo2addr, you will find the data xref.
    const static std::uintptr_t _luaH_dummynode = RebaseAddress(
                                                          0x146603128);             // Find " ,"metatable": ", and go to the top of the second xref, a ldebug function. It will contain two references to luaH_dummynode.

    // The way we get our lua state is the classic, hooking.
    const static std::uintptr_t pseudo2addr = RebaseAddress(0x142f74290);

    const static std::uintptr_t rluaE_newthread = RebaseAddress(0x142f75780);
    // Required for not crashing on errors.
    const static std::uintptr_t rLuaD_throw = RebaseAddress(
                                                      0x142f78190);       // search for "memory allocation error: block too big". You will find luaG_runerrorL, last call is luaD_throw.
    const static std::uintptr_t rlua_newthread = RebaseAddress(0x142f72200);
    const static std::uintptr_t rTask_defer = RebaseAddress(0x141cb4d10);

    const static std::uintptr_t rFreeBlock = RebaseAddress(0x142f8ea70);
    const static std::uintptr_t rFromLuaState = RebaseAddress(
                                                        0x141b05490);     // Appears to copy ones' L->userdata into another for new states. Search for "Failed to create Lua State", on the userthread user callback.
    const static std::uintptr_t rLuaD_rawrununprotected = RebaseAddress(
                                                                  0x142f77f40);     // Search for luaD_rununprotected, crawl until you reach into luaD_pcall, from which, search for xrefs into a function referencing luaO_nilobject and pseudo2addr, it will also have an if check at the end checking for a negative value (first bit set).

    const static std::uintptr_t rLuaC_Step = RebaseAddress(0x142f8c370);

    const static std::uintptr_t rRBX__TaskScheduler__getSingleton = RebaseAddress(0x143b461c0);
}

struct lua_State;

namespace FunctionTypes {
    //using luau_load = int32_t (__fastcall *)(struct lua_State *L, const char *chunkName, const char *bytecode,
    //                                         int32_t bytecodeSize,
    //                                         int32_t env);
    using luau_execute = void (__fastcall *)(struct lua_State *L);
    using pseudo2addr = void *(__fastcall *)(lua_State *L, int32_t lua_index);
    using lua_newthread = lua_State *(__fastcall *)(lua_State *L);
    using rTask_spawn = int (__fastcall *)(lua_State *L);
    using rTask_defer = int (__fastcall *)(lua_State *L);
    using rluaE_newthread = lua_State *(__fastcall *)(lua_State *L);
    using rlua_newthread = lua_State *(__fastcall *)(lua_State *L);
    using rbxAllocate = void *(__fastcall *)(std::uintptr_t size);
    using rFromLuaState = void (__fastcall *)(lua_State *LP, lua_State *L);
    using rFreeBlock = void (__fastcall *)(lua_State *L, int32_t sizeClass, void *block);
    using rLuaD_throw = void (__fastcall *)(lua_State *L, int32_t errcode);
    using rLuaD_rawrununprotected = int32_t (__fastcall *)(struct lua_State *L,
                                                           void (*PFunc)(struct lua_State *L, void *ud),
                                                           void *ud);
    using rLuaC_Step = size_t (__fastcall *)(lua_State *L, bool assist);
    using rRBX__TaskScheduler__getSingleton = void *(__fastcall *)(void);
};

namespace RBX::Studio::Functions {
    // const static auto luau_load = reinterpret_cast<FunctionTypes::luau_load>(RBX::Studio::Offsets::luau_load);
    const static auto luau_execute = reinterpret_cast<FunctionTypes::luau_execute>(RBX::Studio::Offsets::luau_execute);
    const static auto rTask_defer = reinterpret_cast<FunctionTypes::rTask_defer>(RBX::Studio::Offsets::rTask_defer);
    const static auto rluaE_newthread = reinterpret_cast<FunctionTypes::rluaE_newthread>(RBX::Studio::Offsets::rluaE_newthread);
    const static auto rlua_newthread = reinterpret_cast<FunctionTypes::rlua_newthread>(RBX::Studio::Offsets::rlua_newthread);
    // const static auto rbxAllocate = reinterpret_cast<FunctionTypes::rbxAllocate>(RBX::Studio::Offsets::rbxAllocate);
    const static auto rFromLuaState = reinterpret_cast<FunctionTypes::rFromLuaState>(RBX::Studio::Offsets::rFromLuaState);
    const static auto rFreeBlock = reinterpret_cast<FunctionTypes::rFreeBlock>(RBX::Studio::Offsets::rFreeBlock);
    const static auto rLuaD_throw = reinterpret_cast<FunctionTypes::rLuaD_throw>(RBX::Studio::Offsets::rLuaD_throw);
    const static auto rLuaD_rawrununprotected = reinterpret_cast<FunctionTypes::rLuaD_rawrununprotected>(RBX::Studio::Offsets::rLuaD_rawrununprotected);
    const static auto rLuaC_Step = reinterpret_cast<FunctionTypes::rLuaC_Step>(RBX::Studio::Offsets::rLuaC_Step);
    const static auto rRBX__TaskScheduler__getSingleton = reinterpret_cast<FunctionTypes::rRBX__TaskScheduler__getSingleton>(RBX::Studio::Offsets::rRBX__TaskScheduler__getSingleton);
    //  We don't require of Robloxs' luaC_step
    //  const static auto rLuaC_step = reinterpret_cast<FunctionTypes::rLuaC_step>(RBX::Studio::Offsets::rLuaC_step);
}

/*
 *  How to get this to compile when updating Luau?
 *      - Modify lobject.cpp and lobject.h to use Studios' luaO_nilobject, same thing with ltable.cpp and ltable.h and luaH_dummynode, as well as modifying lvm.cpp to use luau_execute. You must use luau_load when compiling code, for anyone using this to develop anything.
 */