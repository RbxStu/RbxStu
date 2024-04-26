//
// Created by Dottik on 3/4/2024.
//
#pragma once

#include "Windows.h"
#include <cstdint>

#define RebaseAddress(x) (x - 0x140000000 + reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))

namespace RBX::Studio::Offsets {
    // Search for "C Stack Overflow"
    // Signature: 80 79 06 00 0F 85 ? ? ? ? E9 ? ? ? ?
    const static std::uintptr_t luau_execute = RebaseAddress(0x14308c150);

    // Get to lua_pushvalue and get inside of pseudo2addr, you will find the data xref.
    const static std::uintptr_t _luaO_nilobject = RebaseAddress(0x146758208);

    // Find " ,"metatable": ", and go to the top of the second xref, a ldebug
    // function. It will contain two references to luaH_dummynode.
    const static std::uintptr_t _luaH_dummynode = RebaseAddress(0x1467580c8);

    // The way we get our lua state is the classic, hooking.
    // Signature: 0F 84 88 00 00 00 81 FA EF D8 FF FF 74 45 81 FA F0 D8 FF FF 74 32 48 8B 41 ? 44 2B CA 48 8B 48 ? 4C 8B 01 41 0F B6 40 ? 44 3B C8 7F 12 41 8D 41 ? 48 98 48 83 C0 ? 48 C1 E0 ? 49 03 C0 C3
    const static std::uintptr_t pseudo2addr = RebaseAddress(0x14307f7f0);

    // search for coroutine.wrap. Inside there will be a lua_pushthread and lua_newthread call. Inside lua_newthread luaE_newthread is called.
    const static std::uintptr_t rluaE_newthread = RebaseAddress(0x143080ce0);
    // search for "memory allocation error: block too big". You will find luaG_runerrorL, last call is luaD_throw.
    // Signature for luaG_runerrorL (Calls luaD_throw): 48 89 50 ? 4C 89 40 ? 4C 89 48 ? 53 48 81 EC ? ? ? ? 48 8B D9 4C 8D 48 ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 48 8D 54 24 ? 48 8B CB E8 ? ? ? ? BA ? ? ? ? 48 8B CB E8 ? ? ? ?
    const static std::uintptr_t rLuaD_throw = RebaseAddress(0x1430836f0);
    // search for coroutine.wrap. Inside there will be a lua_pushthread and lua_newthread call.
    const static std::uintptr_t rlua_newthread = RebaseAddress(0x14307d760);
    // search for "defer"
    const static std::uintptr_t rTask_defer = RebaseAddress(0x141d77ef0);

    // Get into luaC_step, then into gcstep and then into luaH_free, which inside has the call you need to freestack (or luaM_freearray I don't remember)
    // Which inside calls freeblock.
    // Signature: 4C 8B 51 ? 49 83 E8 ? 44 8B CA 4C 8B D9 49 8B 10 48 83 7A 28 00 75 22 83 7A 30 00 7D 1C 49 63 C1 49 8D 0C C2 49 8B 44 C2 ? 48 89 42 ? 48 85 C0 74 03 48 89 10 48 89 51 ? 48 8B 42 ? 49 89 00 83 6A ? ? 4C 89 42 ? 75 4B 48 8B 4A ? 48 85 C9 74 06 48 8B 02 48 89 01 48 8B 0A 48 85 C9 74 0A 48 8B 42 ? 48 89 41 ? EB 17 41 0F B6 C1 49 39 54 C2 60 49 8D 0C C2 75 08 48 8B 42 ? 48 89 41 ? 49 8B 43 ? 45 33 C9 4C 63 42 ? 48 8B 48 ? 48 FF 60 ?
    const static std::uintptr_t rFreeBlock = RebaseAddress(0x14309a130);

    // Appears to copy ones' L->userdata into another for new states. Search for
    // "Failed to create Lua State", on the userthread user callback.
    const static std::uintptr_t rFromLuaState = RebaseAddress(0x141bbe4a0);

    // Search for luaD_rununprotected, crawl until you reach into luaD_pcall, from which, search for xrefs into a function referencing
    // luaO_nilobject and pseudo2addr, it will also have an if check at the end checking for a negative value (first bit set).
    const static std::uintptr_t rLuaD_rawrununprotected = RebaseAddress(0x1430834a0);

    const static std::uintptr_t rLuaC_Step = RebaseAddress(0x143097a30);

    // search for "ProximityPrompt_Triggered".
    const static std::uintptr_t fireproximityprompt = RebaseAddress(0x14252bfe0);

    // search for "InvalidInstance". Caller with two arguments (Modifies lua stack)
    const static std::uintptr_t pushinstance = RebaseAddress(0x141c83de0);
} // namespace RBX::Studio::Offsets

struct lua_State;

namespace FunctionTypes {
    using luau_execute = void(__fastcall*)(lua_State* L);
    using pseudo2addr = void*(__fastcall*)(lua_State* L, int32_t lua_index);
    using rTask_defer = int(__fastcall*)(lua_State* L);
    using rluaE_newthread = lua_State*(__fastcall*)(lua_State* L);
    using rlua_newthread = lua_State*(__fastcall*)(lua_State* L);
    using rFromLuaState = void(__fastcall*)(lua_State* LP, lua_State* L);
    using rFreeBlock = void(__fastcall*)(lua_State* L, int32_t sizeClass, void* block);
    using rLuaD_throw = void(__fastcall*)(lua_State* L, int32_t errcode);
    using rLuaD_rawrununprotected = int32_t(__fastcall*)(lua_State* L, void (*PFunc)(lua_State* L, void* ud), void* ud);
    using rLuaC_Step = size_t(__fastcall*)(lua_State* L, bool assist);
    using fireproximityprompt = void(__fastcall*)(void *proximityPrompt);
    using pushinstance = std::uintptr_t (__fastcall *)(lua_State *L, void *instance);
}; // namespace FunctionTypes

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
    const static auto fireproximityprompt = reinterpret_cast<FunctionTypes::fireproximityprompt>(RBX::Studio::Offsets::fireproximityprompt);
    const static auto pushinstance = reinterpret_cast<FunctionTypes::pushinstance>(RBX::Studio::Offsets::pushinstance);
} // namespace RBX::Studio::Functions

/*
 *  How to get this to compile when updating Luau?
 *      - Modify lobject.cpp and lobject.h to use Studios' luaO_nilobject, same thing with ltable.cpp and ltable.h and luaH_dummynode, as well as
 * modifying lvm.cpp to use luau_execute. You must use luau_load when compiling code, for anyone using this to develop anything.
 */